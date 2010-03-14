/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "syscalls.h"
#include <sys/time.h>
#include <time.h>

#define LINUX_CLOCK_REALTIME  0
#define LINUX_CLOCK_MONOTONIC 1

/* compat_timeval is compatible with Interix */
SYSCALL(compat_sys_gettimeofday)
{
	struct timeval& lt = *(struct timeval*) arg.a0.p;

	int result = gettimeofday(&lt, NULL);
	return SysError(result);
}

SYSCALL(compat_sys_time)
{
	u_int32_t* tp = (u_int32_t*) arg.a0.p;

	time_t t = time(NULL);
	if (tp)
		*tp = t;
	return t;
}

/* compat_timespec is compatible with Interix */
SYSCALL(compat_sys_clock_gettime)
{
	int which_clock = arg.a0.s;
	struct timespec& lt = *(struct timespec*) arg.a1.p;

	switch (which_clock)
	{
		case LINUX_CLOCK_REALTIME:
		case LINUX_CLOCK_MONOTONIC:
		{
			struct timeval it;
			int result = gettimeofday(&it, NULL);
			if (result == -1)
				throw EINVAL;

			lt.tv_sec = it.tv_sec;
			lt.tv_nsec = it.tv_usec * 1000;
			break;
		}

		default:
			throw EINVAL;
	}

	return 0;
}

SYSCALL(compat_sys_clock_getres)
{
	int which_clock = arg.a0.s;
	struct timespec& lt = *(struct timespec*) arg.a1.p;

	switch (which_clock)
	{
		case LINUX_CLOCK_REALTIME:
		case LINUX_CLOCK_MONOTONIC:
		{
			lt.tv_sec = 0;
			lt.tv_nsec = 1000000000LL / CLOCKS_PER_SEC;
			break;
		}

		default:
			throw EINVAL;
	}

	return 0;
}

/* struct itermval is compatible; timer names are compatible */
SYSCALL(compat_sys_setitimer)
{
	int which = arg.a0.s;
	const struct itimerval* value = (const struct itimerval*) arg.a1.p;
	struct itimerval* ovalue = (struct itimerval*) arg.a2.p;

	int i = setitimer(which, value, ovalue);
	if (i == -1)
		throw errno;
	return 0;
}
