#include "globals.h"
#include "syscalls.h"
#include <sys/utsname.h>

struct linux_old_utsname {
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
};

SYSCALL(sys_personality)
{
	unsigned long persona = arg.a0.u;
	if (persona == 0)
		return 0;

	log("attempt to set invalid persona %d\n", persona);
	return -EINVAL;
}

SYSCALL(sys_uname)
{
	struct linux_old_utsname& lu = *(struct linux_old_utsname*) arg.a0.p;

	struct utsname iu;
	int result = uname(&iu);
	if (result == -1)
		return -ErrnoI2L(errno);

	strncpy(lu.sysname, "Linux", sizeof(lu.sysname));
	strncpy(lu.nodename, iu.nodename, sizeof(lu.nodename));
	strncpy(lu.release, "2.6.23", sizeof(lu.release));
	strncpy(lu.version, "LWB emulation 0.1", sizeof(lu.version));
	strncpy(lu.machine, iu.machine, sizeof(lu.machine));
	return 0;
}

