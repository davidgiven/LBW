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