/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "syscalls.h"
#include "filesystem/FD.h"
#include "filesystem/RawFD.h"
#include "filesystem/DirFD.h"
#include "filesystem/VFS.h"
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <dirent.h>
#include <poll.h>
#include <map>

#define LINUX_POLLIN          0x001           /* There is data to read.  */
#define LINUX_POLLPRI         0x002           /* There is urgent data to read.  */
#define LINUX_POLLOUT         0x004           /* Writing now will not block.  */
#define LINUX_POLLRDNORM      0x040           /* Normal data may be read.  */
#define LINUX_POLLRDBAND      0x080           /* Priority data may be read.  */
#define LINUX_POLLWRNORM      0x100           /* Writing now will not block.  */
#define LINUX_POLLWRBAND      0x200           /* Priority data may be written.  */
#define LINUX_POLLMSG         0x400
#define LINUX_POLLREMOVE      0x1000
#define LINUX_POLLRDHUP       0x2000
#define LINUX_POLLERR         0x008           /* Error condition.  */
#define LINUX_POLLHUP         0x010           /* Hung up.  */
#define LINUX_POLLNVAL        0x020           /* Invalid polling request.  */

/* Interix supports poll... but only on /proc/%/ctl files. Bah.
 * Luckily the struct pollfd structures are compatible.
 */
SYSCALL(sys_poll)
{
	struct pollfd* lp = (struct pollfd*) arg.a0.p;
	unsigned int nfds = arg.a1.u;
	long timeout = arg.a2.s;

	fd_set reads;
	fd_set writes;
	fd_set excepts;

	FD_ZERO(&reads);
	FD_ZERO(&writes);
	FD_ZERO(&excepts);

	//log("poll (nfds=%d timeout=%ld)", nfds, timeout);
	int maxfd = 0;
	for (unsigned i = 0; i < nfds; i++)
	{
		int fd = lp[i].fd;
		int realfd = FD::Get(fd)->GetRealFD();

		//log("realfd %d", realfd);
		if (realfd > maxfd)
			maxfd = realfd;

		short flags = lp[i].events;
		if (flags & LINUX_POLLIN)
		{
			//log("...interested in being readable");
			FD_SET(realfd, &reads);
		}
		if (flags & LINUX_POLLOUT)
		{
			//log("...interested in being writeable");
			FD_SET(realfd, &writes);
		}
		if (flags & LINUX_POLLERR)
		{
			//log("...interested in errors");
			FD_SET(realfd, &excepts);
		}

		lp[i].revents = 0;
	}

	struct timeval tv;
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;

	int result = select(maxfd+1, &reads, &writes, &excepts,
			(timeout == -1) ? NULL : &tv);
	if (result == -1)
		throw errno;
	if (result == 0)
	{
		//log("...timed out with no change");
		return 0;
	}

	for (unsigned i = 0; i < nfds; i++)
	{
		int fd = lp[i].fd;
		int realfd = FD::Get(fd)->GetRealFD();

		//log("realfd %d", realfd);
		lp[i].revents = 0;
		if (FD_ISSET(realfd, &reads))
		{
			//log("...is readable");
			lp[i].revents |= LINUX_POLLIN;
		}
		if (FD_ISSET(realfd, &writes))
		{
			//log("...is writeable");
			lp[i].revents |= LINUX_POLLOUT;
		}
		if (FD_ISSET(realfd, &excepts))
		{
			//log("...is error");
			lp[i].revents |= LINUX_POLLERR;
		}
	}

	return result;
}

/* compat_timeval is compatible with Interix */
SYSCALL(compat_sys_select)
{
	int size = arg.a0.s;
	fd_set* lreads = (fd_set*) arg.a1.p;
	fd_set* lwrites = (fd_set*) arg.a2.p;
	fd_set* lexcepts = (fd_set*) arg.a3.p;
	struct timeval* tv = (struct timeval*) arg.a4.p;

	fd_set interested;
	FD_ZERO(&interested);

	fd_set reads;
	fd_set writes;
	fd_set excepts;

	FD_ZERO(&reads);
	FD_ZERO(&writes);
	FD_ZERO(&excepts);

	for (int i = 0; i < size; i++)
	{
		bool r = lreads ? FD_ISSET(i, lreads) : false;
		bool w = lwrites ? FD_ISSET(i, lwrites) : false;
		bool x = lexcepts ? FD_ISSET(i, lexcepts) : false;

		if (r || w || x)
		{
			FD_SET(i, &interested);

			int realfd = FD::Get(i)->GetRealFD();
			if (r)
				FD_SET(realfd, &reads);
			if (w)
				FD_SET(realfd, &writes);
			if (x)
				FD_SET(realfd, &excepts);
		}
	}

	int result = select(size, &reads, &writes, &excepts, tv);
	if (result == -1)
		throw errno;

	for (int i = 0; i < size; i++)
	{
		if (FD_ISSET(i, &interested))
		{
			int realfd = FD::Get(i)->GetRealFD();

			if (lreads)
			{
				if (FD_ISSET(realfd, &reads))
					FD_SET(i, lreads);
				else
					FD_CLR(i, lreads);
			}

			if (lwrites)
			{
				if (FD_ISSET(realfd, &writes))
					FD_SET(i, lwrites);
				else
					FD_CLR(i, lwrites);
			}

			if (lexcepts)
			{
				if (FD_ISSET(realfd, &excepts))
					FD_SET(i, lexcepts);
				else
					FD_CLR(i, lexcepts);
			}
		}
	}

	return result;
}
