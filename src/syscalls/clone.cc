/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "syscalls.h"
#include <unistd.h>

#define LINUX_CSIGNAL                 0x000000ff      /* signal mask to be sent at exit */
#define LINUX_CLONE_VM                0x00000100      /* set if VM shared between processes */
#define LINUX_CLONE_FS                0x00000200      /* set if fs info shared between processes */
#define LINUX_CLONE_FILES             0x00000400      /* set if open files shared between processes */
#define LINUX_CLONE_SIGHAND           0x00000800      /* set if signal handlers and blocked signals shared */
#define LINUX_CLONE_PTRACE            0x00002000      /* set if we want to let tracing continue on the child too */
#define LINUX_CLONE_VFORK             0x00004000      /* set if the parent wants the child towake it up on mm_release */
#define LINUX_CLONE_PARENT            0x00008000      /* set if we want to have the same parent as the cloner */
#define LINUX_CLONE_THREAD            0x00010000      /* Same thread group? */
#define LINUX_CLONE_NEWNS             0x00020000      /* New namespace group? */
#define LINUX_CLONE_SYSVSEM           0x00040000      /* share system V SEM_UNDO semantics */
#define LINUX_CLONE_SETTLS            0x00080000      /* create a new TLS for the child */
#define LINUX_CLONE_PARENT_SETTID     0x00100000      /* set the TID in the parent */
#define LINUX_CLONE_CHILD_CLEARTID    0x00200000      /* clear the TID in the child */
#define LINUX_CLONE_DETACHED          0x00400000      /* Unused, ignored */
#define LINUX_CLONE_UNTRACED          0x00800000      /* set if the tracing process can't force CLONE_PTRACE on this clone */
#define LINUX_CLONE_CHILD_SETTID      0x01000000      /* set the TID in the child */
#define LINUX_CLONE_STOPPED           0x02000000      /* Start in stopped state */
#define LINUX_CLONE_NEWUTS            0x04000000      /* New utsname group? */
#define LINUX_CLONE_NEWIPC            0x08000000      /* New ipcs */
#define LINUX_CLONE_NEWUSER           0x10000000      /* New user namespace */
#define LINUX_CLONE_NEWPID            0x20000000      /* New pid namespace */
#define LINUX_CLONE_NEWNET            0x40000000      /* New network namespace */
#define LINUX_CLONE_IO                0x80000000      /* Clone io context */

/* We can't implement clone() on Interix --- it's just not possible.
 * However, Linux uses clone() to do a whole bunch of stuff, and we can
 * implement some of the common use cases.
 */

SYSCALL(stub32_fork)
{
	int result = fork();

	switch (result)
	{
		case 0: /* child */
			InitProcess();
			InstallExceptionHandler();
			//error("fork() needs work");
			//StartMonitor();
			break;

		case -1: /* error */
			throw errno;
	}

	return result;
}

SYSCALL(stub32_vfork)
{
	int result = fork(); /* vfork doesn't work */

	switch (result)
	{
		case 0: /* child */
			InitProcess();
			InstallExceptionHandler();
			//error("fork() needs work");
			//StartMonitor();
			break;

		case -1: /* error */
			throw errno;
	}

	return result;
}

SYSCALL(sys32_clone)
{
	u32 clone_flags = arg.a0.u;
	u32 newsp = arg.a1.u;
	void* parent_tid = arg.a2.p;
	void* child_tid = arg.a4.p;

	switch (clone_flags)
	{
		case 0x01200011: /* fork, I think */
		case 0x00100011:
		{
			pid_t newpid = stub32_fork(arg);
			switch (newpid)
			{
				case 0: /* child */
					if (clone_flags & LINUX_CLONE_CHILD_SETTID)
					{
						if (child_tid)
							*(pid_t*)child_tid = getpid();
					}
					if (clone_flags & LINUX_CLONE_PARENT_SETTID)
					{
						if (parent_tid)
							*(pid_t*)parent_tid = getpid();
					}
					break;

				default: /* parent */
					if (clone_flags & LINUX_CLONE_PARENT_SETTID)
					{
						if (parent_tid)
							*(pid_t*)parent_tid = newpid;
					}
					break;
			}
			return newpid;
		}

		default:
			log("clone flags=%08x newsp=%08x ptid=%p ctid=%p", clone_flags, newsp,
					parent_tid, child_tid);
			error("unimplemented clone scenario");
	}
}
