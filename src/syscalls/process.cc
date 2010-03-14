/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "syscalls.h"
#include <sys/resource.h>
#include <sys/wait.h>

const uid_t overflowuid = 65534;
const uid_t overflowgid = 65534;

#define LINUX_RLIMIT_CPU		0	/* CPU time in sec */
#define LINUX_RLIMIT_FSIZE		1	/* Maximum filesize */
#define LINUX_RLIMIT_DATA		2	/* max data size */
#define LINUX_RLIMIT_STACK		3	/* max stack size */
#define LINUX_RLIMIT_CORE		4	/* max core file size */
#define LINUX_RLIMIT_RSS		5	/* max resident set size */
#define LINUX_RLIMIT_NPROC		6	/* max number of processes */
#define LINUX_RLIMIT_NOFILE		7	/* max number of open files */
#define LINUX_RLIMIT_MEMLOCK		8	/* max locked-in-memory address space */
#define LINUX_RLIMIT_AS		9	/* address space limit */
#define LINUX_RLIMIT_LOCKS		10	/* maximum file locks held */
#define LINUX_RLIMIT_SIGPENDING	11	/* max number of pending signals */
#define LINUX_RLIMIT_MSGQUEUE		12	/* maximum bytes in POSIX mqueues */
#define LINUX_RLIMIT_NICE		13	/* max nice prio allowed to raise to
					   0-39 for nice level 19 .. -20 */
#define LINUX_RLIMIT_RTPRIO		14	/* maximum realtime priority */
#define LINUX_RLIMIT_RTTIME		15	/* timeout for RT tasks in us */
#define LINUX_RLIM_NLIMITS		16

#define LINUX_WNOHANG	0x00000001
#define LINUX_WUNTRACED	0x00000002
#define LINUX_WEXITED	0x00000004
#define LINUX_WCONTINUED	0x00000008
#define LINUX_WNOWAIT	0x01000000	/* Don't reap, just poll status.  */

#define LINUX___WNOTHREAD	0x20000000	/* Don't wait on children of other threads in this group */
#define LINUX___WALL		0x40000000	/* Wait on all children, regardless of type */
#define LINUX___WCLONE	0x80000000	/* Wait only on non-SIGCHLD children */



static uid_t high2lowuid(uid_t uid)
{
	if (uid > 0x10000)
		return overflowuid;
	return uid;
}

static gid_t high2lowgid(gid_t gid)
{
	if (gid > 0x10000)
		return overflowgid;
	return gid;
}

SYSCALL(sys_exit)
{
	exit(arg.a0.s);
}

SYSCALL(sys_exit_group)
{
	exit(arg.a0.s);
}

SYSCALL(sys_getpid)
{
	return getpid();
}

SYSCALL(sys_getppid)
{
	return getppid();
}

SYSCALL(sys_getuid)
{
	if (Options.FakeRoot)
		return 0;
	return getuid();
}

SYSCALL(sys_setuid)
{
	uid_t uid = arg.a0.u;

	if (Options.FakeRoot)
		return 0;

	int result = setuid(uid);
	return SysError(result);
}

SYSCALL(sys_getuid16)
{
	if (Options.FakeRoot)
		return 0;

	return high2lowuid(getuid());
}

SYSCALL(sys_geteuid)
{
	if (Options.FakeRoot)
		return 0;

	return geteuid();
}

SYSCALL(sys_geteuid16)
{
	if (Options.FakeRoot)
		return 0;

	return high2lowuid(geteuid());
}

SYSCALL(sys_getgid)
{
	return getgid();
}

SYSCALL(sys_setgid)
{
	gid_t gid = arg.a0.u;

	int result = setgid(gid);
	return SysError(result);
}

SYSCALL(sys_getgid16)
{
	return high2lowgid(getgid());
}

SYSCALL(sys_getegid)
{
	return getegid();
}

SYSCALL(sys_getegid16)
{
	return high2lowgid(getegid());
}

SYSCALL(sys_getgroups)
{
	int gidsetsize = arg.a0.s;
	gid_t* grouplist = (gid_t*) arg.a1.p;

	int i = getgroups(gidsetsize, grouplist);
	if (i == -1)
		throw errno;
	return i;
}

static int convert_resource_l2i(int resource)
{
	switch (resource)
	{
		case LINUX_RLIMIT_CPU:    return RLIMIT_CPU;
		case LINUX_RLIMIT_FSIZE:  return RLIMIT_FSIZE;
		case LINUX_RLIMIT_DATA:   return RLIMIT_DATA;
		case LINUX_RLIMIT_STACK:  return RLIMIT_STACK;
		case LINUX_RLIMIT_CORE:   return RLIMIT_CORE;
		case LINUX_RLIMIT_NOFILE: return RLIMIT_NOFILE;
		case LINUX_RLIMIT_AS:     return RLIMIT_AS;
	}

	return -1;
}

/* struct rlimit and struct linux_compat_rlimit are compatible */
SYSCALL(compat_sys_getrlimit)
{
	int resource = arg.a0.s;
	struct rlimit* limit = (struct rlimit*) arg.a1.p;

	int iresource = convert_resource_l2i(resource);
	if (iresource == -1)
	{
		Warning("getrlimit(%d) unsupported on Interix", resource);
		return -LINUX_EINVAL;
	}

	int result = getrlimit(iresource, limit);
	if (result == -1)
		throw errno;
	return 0;
}

SYSCALL(sys_getpgrp)
{
	return getpgrp();
}

SYSCALL(sys_setpgid)
{
	pid_t pid = arg.a0.u;
	pid_t pgrp = arg.a1.u;

	int result = setpgid(pid, pgrp);
	if (result == -1)
		throw errno;
	return 0;
}

static int convert_waitoptions_l2i(int options, int& ioptions)
{
	ioptions = 0;
	if (options & LINUX_WNOHANG)
		ioptions |= WNOHANG;
	if (options & LINUX_WUNTRACED)
		ioptions |= WUNTRACED;

	if (options & (LINUX_WEXITED | LINUX_WCONTINUED | LINUX_WNOWAIT
			| LINUX___WNOTHREAD | LINUX___WALL | LINUX___WCLONE))
	{
		Warning("unsupported waitpid options %08x", options);
	}

	return ioptions;
}

SYSCALL(sys32_waitpid)
{
	pid_t pid = arg.a0.u;
	int* stat_addr = (int*) arg.a1.p;
	int options = arg.a2.s;

	int ioptions;
	convert_waitoptions_l2i(options, ioptions);

	/* Luckily, Interix and Linux wait statuses are compatible. */
	int result = waitpid(pid, stat_addr, ioptions);
	return SysError(result);
}
