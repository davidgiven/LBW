/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "syscalls.h"
#include <sys/utsname.h>

#pragma pack(push, 1)
struct linux_old_utsname {
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
};

struct linux_compat_sysinfo {
	s32 uptime;
	u32 loads[3];
	u32 totalram;
	u32 freeram;
	u32 sharedram;
	u32 bufferram;
	u32 totalswap;
	u32 freeswap;
	u16 procs;
	u16 pad;
	u32 totalhigh;
	u32 freehigh;
	u32 mem_unit;
	char _f[20-2*sizeof(u32)-sizeof(int)];
};
#pragma pack(pop)

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

SYSCALL(compat_sys_sysinfo)
{
	struct linux_compat_sysinfo* sysinfo =
			(struct linux_compat_sysinfo*) arg.a0.p;

	Warning("sysinfo() not implemented yet");
	throw ENOSYS;
}
