#include "globals.h"
#include "Thread.h"
#include <sys/procfs.h>
#include <sys/fault.h>

int wrapped_pthread_create(pthread_t* thread,
		const pthread_attr_t* attr,
		Thread::Routine* routine, void* arg)
{
	Thread* t = new Thread();
	if (!t)
		return EAGAIN;

	int i = t->Create(attr, routine, arg);
	if (i)
	{
		delete t;
		return i;
	}

	*thread = t->SlaveThread();
	return 0;
}

Thread::Thread()
{
	pthread_mutex_init(&_mutex, NULL);
	sem_init(&_master_startup_semaphore, false, 0);
	sem_init(&_slave_startup_semaphore, false, 0);
	_ctl_fd = _status_fd = -1;
}

Thread::~Thread()
{
	pthread_mutex_destroy(&_mutex);
	sem_destroy(&_master_startup_semaphore);
	sem_destroy(&_slave_startup_semaphore);

	if (_ctl_fd != -1)
		close(_ctl_fd);
	if (_status_fd != -1)
		close(_status_fd);
}

int Thread::Create(const pthread_attr_t* attr,
		Routine* routine, void* arg)
{
	_routine = routine;
	_user = arg;

	_startup_state = 0;
	int i = pthread_create(&_slave, attr, slave_cb, this);
	if (i != 0)
		return i;

	/* Block waiting for the startup to complete. */

	sem_wait(&_slave_startup_semaphore);
	return _startup_state;
}

void* Thread::slave_cb(void* user)
{
	Thread* thread = (Thread*) user;
	return thread->slave_main();
}

void* Thread::master_cb(void* user)
{
	Thread* thread = (Thread*) user;
	return thread->master_main();
}

void* Thread::slave_main()
{
	log("slave thread %x started", _slave);

	/* Before we can run any user code, we must ensure that
	 * the master thread has startup up and is watching this
	 * thread.
	 *
	 * First, create the master thread.
	 */

	_startup_state = pthread_create(&_master, NULL, master_cb, this);
	if (_startup_state != 0)
	{
		log("master thread creation failed");
		goto error;
	}

	/* Now wait for the master thread to start up. */

	sem_wait(&_master_startup_semaphore);
	if (_startup_state != 0)
	{
		log("master thread init failed");
		goto error;
	}

	/* Now we can run user code. */

	sem_post(&_slave_startup_semaphore);
	return _routine(_user);

error:
	sem_post(&_slave_startup_semaphore);
	return NULL;
}

void Thread::cmd(int opcode)
{
	log("master: cmd %x", opcode);
	write(_ctl_fd, &opcode, sizeof(opcode));
}

void Thread::cmd(int opcode, u_int32_t param)
{
	log("master: cmd %x %lx", opcode, param);
	u_int32_t buf[] = {opcode, param};
	write(_ctl_fd, &buf, sizeof(buf));
}

void* Thread::master_main()
{
	int fd;
	log("master thread started");

	/* Open the LWP ctl and status pseudofiles. */

	char buffer[64];
	snprintf(buffer, sizeof(buffer), "/proc/%ld/lwp/%ld/lwpctl",
			getpid(), _slave);
	_ctl_fd = open(buffer, O_WRONLY);
	log("%s -> %d", buffer, _ctl_fd);
	if (_ctl_fd == -1)
		goto error;

	snprintf(buffer, sizeof(buffer), "/proc/%ld/lwp/%ld/lwpstatus",
			getpid(), _slave);
	_status_fd = open(buffer, O_RDONLY);
	log("%s -> %d", buffer, _status_fd);
	if (_status_fd == -1)
		goto error;

	/* Open the root process ctl pseudofile. */

	snprintf(buffer, sizeof(buffer), "/proc/%ld/ctl",
			getpid(), _slave);
	fd = open(buffer, O_WRONLY);
	log("%s -> %d", buffer, fd);

	{
		int buf[] = {
				PCSFAULT, 0xffffffff,
				PCSTRACE, 0xffffffff
		};
		write(fd, buf, sizeof(buf));
	}

	/* We initialise the LWP interface here, so that we only get
	 * notified of events we're interested in.
	 */

	//cmd(PCSFAULT, 0xffffffff);
	//cmd(PCSTRACE, 0xffffffff);

	/* Initialisation complete! Tell the child to stop
	 * as soon as possible, and then clear the semaphore
	 * so it'll run once we resume it.. */

	cmd(PCSTOP);
	sem_post(&_master_startup_semaphore);

	/* ...and enter the event loop. */

	for (;;)
	{
		struct lwpstatus status;

		{
			int i = read(_status_fd, &status, sizeof(status));
			log("master: read %d bytes", i);
			if (i == 0)
				break;
		}

		log("master: status is %d", status.pr_why);
	}

	delete this;
	return NULL;

error:
	_startup_state = EAGAIN;
	sem_post(&_master_startup_semaphore);
	return NULL;
}
