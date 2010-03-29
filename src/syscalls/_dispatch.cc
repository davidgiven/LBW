/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "syscalls.h"
#include <stdexcept>

//#define VERBOSE

using std::logic_error;

extern "C" int32_t Linux_MCE_Handler(Registers& regs)
	__attribute__ ((regparm (1)))
	__attribute__ ((force_align_arg_pointer));

__asm (
	".globl _Linux_MCE; "
	".extern _Linux_MCE; "
	"_Linux_MCE: "
		"pushf; "
		"pushl %gs; "
		"pushl %ebp; "
        "pushl %edi; "
        "pushl %esi; "
        "pushl %edx; "
        "pushl %ecx; "
        "pushl %ebx; "
        "pushl %eax; "
		"mov %esp, %eax; "
		"call _Linux_MCE_Handler; "
		"mov %eax, (%esp, 1); "
		"popl %eax; "
		"popl %ebx; "
		"popl %ecx; "
		"popl %edx; "
		"popl %esi; "
		"popl %edi; "
		"popl %ebp; "
		"popl %gs; "
		"popf; "
		"ret; "
	);

static void printregs(Registers& regs)
{
	printf("\n");
	printf("syscall #%d %s\n", regs.arg.syscall, SyscallNames[regs.arg.syscall]);
	printf("a0 = %08x\n", regs.arg.a0.u);
	printf("a1 = %08x\n", regs.arg.a1.u);
	printf("a2 = %08x\n", regs.arg.a2.u);
	printf("a3 = %08x\n", regs.arg.a3.u);
	printf("a4 = %08x\n", regs.arg.a4.u);
	printf("a5 = %08x\n", regs.arg.a5.u);
	printf("gs = %08x\n", regs.gs);
	printf("eip = %08x\n", regs.eip);
	printf("esp = %08x\n", ((u_int32_t)&regs) + sizeof(regs));
}

int32_t Linux_MCE_Handler(Registers& regs)
{
#if defined VERBOSE
	log("syscall #%d %s(%08x, %08x, %08x, %08x, %08x, %08x) from %08x", regs.arg.syscall, SyscallNames[regs.arg.syscall],
			regs.arg.a0.u, regs.arg.a1.u, regs.arg.a2.u,
			regs.arg.a3.u, regs.arg.a4.u, regs.arg.a5.u,
			regs.eip);
#endif

	Syscall* syscall;
	switch (regs.arg.syscall)
	{
#define CALL_SYSCALL(number, name) \
		extern SYSCALL(name); \
		case number: \
			syscall = name; \
			break

		CALL_SYSCALL(  1, sys_exit);
		CALL_SYSCALL(  2, stub32_fork);
		CALL_SYSCALL(  3, sys_read);
		CALL_SYSCALL(  4, sys_write);
		CALL_SYSCALL(  5, compat_sys_open);
		CALL_SYSCALL(  6, sys_close);
		CALL_SYSCALL(  7, sys32_waitpid);
		CALL_SYSCALL(  9, sys_link);
		CALL_SYSCALL( 10, sys_unlink);
		CALL_SYSCALL( 11, sys32_execve);
		CALL_SYSCALL( 12, sys_chdir);
		CALL_SYSCALL( 13, compat_sys_time);
		CALL_SYSCALL( 14, sys_mknod);
		CALL_SYSCALL( 15, sys_chmod);
		CALL_SYSCALL( 19, sys32_lseek);
		CALL_SYSCALL( 20, sys_getpid);
		CALL_SYSCALL( 24, sys_getuid16);
		CALL_SYSCALL( 27, sys_alarm);
		CALL_SYSCALL( 30, compat_sys_utime);
		CALL_SYSCALL( 33, sys_access);
		CALL_SYSCALL( 36, sys_sync);
		CALL_SYSCALL( 37, sys32_kill);
		CALL_SYSCALL( 38, sys_rename);
		CALL_SYSCALL( 39, sys_mkdir);
		CALL_SYSCALL( 40, sys_rmdir);
		CALL_SYSCALL( 41, sys_dup);
		CALL_SYSCALL( 42, sys_pipe);
		CALL_SYSCALL( 43, compat_sys_times);
		CALL_SYSCALL( 45, sys_brk);
		CALL_SYSCALL( 47, sys_getgid16);
		CALL_SYSCALL( 49, sys_geteuid16);
		CALL_SYSCALL( 50, sys_getegid16);
		CALL_SYSCALL( 52, sys_umount);
		CALL_SYSCALL( 54, compat_sys_ioctl);
		CALL_SYSCALL( 55, compat_sys_fcntl64);
		CALL_SYSCALL( 57, sys_setpgid);
		CALL_SYSCALL( 60, sys_umask);
		CALL_SYSCALL( 61, sys_chroot);
		CALL_SYSCALL( 63, sys_dup2);
		CALL_SYSCALL( 64, sys_getppid);
		CALL_SYSCALL( 65, sys_getpgrp);
		CALL_SYSCALL( 66, sys_setsid);
		CALL_SYSCALL( 75, compat_sys_setrlimit);
		CALL_SYSCALL( 77, compat_sys_getrusage);
		CALL_SYSCALL( 78, compat_sys_gettimeofday);
		CALL_SYSCALL( 83, sys_symlink);
		CALL_SYSCALL( 85, sys_readlink);
		CALL_SYSCALL( 90, sys32_mmap);
		CALL_SYSCALL( 91, sys_unmap);
		CALL_SYSCALL( 93, sys_ftruncate);
		CALL_SYSCALL( 94, sys_fchmod);
		CALL_SYSCALL( 99, compat_sys_statfs);
		CALL_SYSCALL(102, compat_sys_socketcall);
		CALL_SYSCALL(104, compat_sys_setitimer);
		CALL_SYSCALL(108, compat_sys_newfstat);
		CALL_SYSCALL(114, compat_sys_wait4);
		CALL_SYSCALL(116, compat_sys_sysinfo);
		CALL_SYSCALL(118, sys_fsync);
		CALL_SYSCALL(120, sys32_clone);
		CALL_SYSCALL(122, sys_uname);
		CALL_SYSCALL(125, sys32_mprotect);
		CALL_SYSCALL(133, sys_fchdir);
		CALL_SYSCALL(136, sys_personality);
		CALL_SYSCALL(140, sys_llseek);
		CALL_SYSCALL(141, compat_sys_getdents);
		CALL_SYSCALL(142, compat_sys_select);
		CALL_SYSCALL(143, sys_flock);
		CALL_SYSCALL(144, sys_msync);
		CALL_SYSCALL(146, compat_sys_writev);
		CALL_SYSCALL(150, sys_mlock);
		CALL_SYSCALL(154, enosys); // sys_sched_setparam
		CALL_SYSCALL(155, enosys); // sys_sched_getparam
		CALL_SYSCALL(156, enosys); // sys_sched_setscheduler
		CALL_SYSCALL(157, enosys); // sys_sched_getscheduler
		CALL_SYSCALL(158, enosys); // sys_sched_yield
		CALL_SYSCALL(159, enosys); // sys_sched_get_priority_max
		CALL_SYSCALL(160, enosys); // sys_sched_get_priority_min
		CALL_SYSCALL(162, compat_sys_nanosleep);
		CALL_SYSCALL(163, sys_mremap);
		CALL_SYSCALL(168, sys_poll);
		CALL_SYSCALL(174, sys32_rt_sigaction);
		CALL_SYSCALL(175, sys32_rt_sigprocmask);
		CALL_SYSCALL(183, sys_get_cwd);
		CALL_SYSCALL(186, stub32_sigaltstack);
		CALL_SYSCALL(190, stub32_vfork);
		CALL_SYSCALL(191, compat_sys_getrlimit);
		CALL_SYSCALL(192, sys32_mmap2);
		CALL_SYSCALL(194, sys32_ftruncate64);
		CALL_SYSCALL(195, sys32_stat64);
		CALL_SYSCALL(196, sys32_lstat64);
		CALL_SYSCALL(197, sys32_fstat64);
		CALL_SYSCALL(198, sys_lchown);
		CALL_SYSCALL(199, sys_getuid);
		CALL_SYSCALL(200, sys_getgid);
		CALL_SYSCALL(201, sys_geteuid);
		CALL_SYSCALL(202, sys_getegid);
		CALL_SYSCALL(203, sys_setreuid);
		CALL_SYSCALL(204, sys_setregid);
		CALL_SYSCALL(205, sys_getgroups);
		CALL_SYSCALL(207, sys_fchown);
		CALL_SYSCALL(208, sys_setresuid);
		CALL_SYSCALL(210, sys_setresgid);
		CALL_SYSCALL(212, sys_chown);
		CALL_SYSCALL(213, sys_setuid);
		CALL_SYSCALL(214, sys_setgid);
		CALL_SYSCALL(219, sys_madvise);
		CALL_SYSCALL(220, compat_sys_getdents64);
		CALL_SYSCALL(221, compat_sys_fcntl64);
		CALL_SYSCALL(224, sys_gettid);
		CALL_SYSCALL(228, sys_fsetxattr);
		CALL_SYSCALL(229, sys_getxattr);
		CALL_SYSCALL(230, sys_lgetxattr);
		CALL_SYSCALL(231, sys_fgetxattr);
		CALL_SYSCALL(240, compat_sys_futex);
		CALL_SYSCALL(252, sys_exit_group);
		CALL_SYSCALL(258, sys_set_tid_address);
		CALL_SYSCALL(265, compat_sys_clock_gettime);
		CALL_SYSCALL(266, compat_sys_clock_getres);
		CALL_SYSCALL(268, compat_sys_statfs64);
		CALL_SYSCALL(270, sys_tgkill);
		CALL_SYSCALL(271, compat_sys_utimes);
		CALL_SYSCALL(295, compat_sys_openat);
		CALL_SYSCALL(298, sys_fchownat);
		CALL_SYSCALL(299, compat_sys_futimesat);
		CALL_SYSCALL(300, sys32_fstatat);
		CALL_SYSCALL(301, sys_unlinkat);
		CALL_SYSCALL(306, sys_fchmodat);
		CALL_SYSCALL(308, compat_sys_pselect6);
		CALL_SYSCALL(311, compat_sys_set_robust_list);
		CALL_SYSCALL(320, compat_sys_utimensat);

		case 243: /* special handling for sys_set_thread_area */
			extern int32_t sys_set_thread_area(Registers& regs);
			return sys_set_thread_area(regs);

		default:
			printregs(regs);
			//DumpMap(getpid());
			error("unimplemented syscall %s (%d)", SyscallNames[regs.arg.syscall],
					regs.arg.syscall);
	}

	try
	{
#if defined VERBOSE
		u_int32_t result = syscall(regs.arg);
		log("-> %08x", result);
		return result;
#else
		return syscall(regs.arg);
#endif
	}
	catch (int e)
	{
#if defined VERBOSE
		log("-> (failed) %d", e);
#endif
		return -ErrnoI2L(e);
	}
	catch (logic_error e)
	{
		error("internal error: %s", e.what());
	}
}

SYSCALL(enosys)
{
	throw ENOSYS;
}
