#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <semaphore.h>

class Thread
{
public:
	typedef void* Routine(void* user);

public:
	Thread();
	~Thread();

	int Create(const pthread_attr_t* attr,
			Routine* routine, void* arg);

	pthread_t SlaveThread() const { return _slave; }

private:
	static void* master_cb(void* arg);
	static void* slave_cb(void* arg);

	void* master_main();
	void* slave_main();

	void cmd(int opcode);
	void cmd(int opcode, u_int32_t param);

private:
	pthread_mutex_t _mutex;
	pthread_t _master;
	pthread_t _slave;
	Routine* _routine;
	void* _user;
	int _ctl_fd;
	int _status_fd;

	sem_t _master_startup_semaphore;
	sem_t _slave_startup_semaphore;
	int _startup_state;
};

extern int wrapped_pthread_create(pthread_t* thread,
		const pthread_attr_t* attr,
		Thread::Routine* routine, void* arg);

#endif
