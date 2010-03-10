#include "globals.h"
#include "syscalls.h"

//#define VERBOSE

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
	//printregs(regs);
	regs.eip += 2; /* don't retry the int when we return */

#if defined VERBOSE
	log("syscall #%d %s(%08x, %08x, %08x, %08x, %08x, %08x)", regs.arg.syscall, SyscallNames[regs.arg.syscall],
			regs.arg.a0.u, regs.arg.a1.u, regs.arg.a2.u,
			regs.arg.a3.u, regs.arg.a4.u, regs.arg.a5.u);
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
		CALL_SYSCALL( 12, sys_chdir);
		CALL_SYSCALL( 13, compat_sys_time);
		CALL_SYSCALL( 15, sys_chmod);
		CALL_SYSCALL( 20, sys_getpid);
		CALL_SYSCALL( 24, sys_getuid16);
		CALL_SYSCALL( 33, sys_access);
		CALL_SYSCALL( 36, sys_sync);
		CALL_SYSCALL( 38, sys_rename);
		CALL_SYSCALL( 39, sys_mkdir);
		CALL_SYSCALL( 40, sys_rmdir);
		CALL_SYSCALL( 41, sys_dup);
		CALL_SYSCALL( 42, sys_pipe);
		CALL_SYSCALL( 45, sys_brk);
		CALL_SYSCALL( 47, sys_getgid16);
		CALL_SYSCALL( 49, sys_geteuid16);
		CALL_SYSCALL( 50, sys_getegid16);
		CALL_SYSCALL( 52, sys_umount);
		CALL_SYSCALL( 54, compat_sys_ioctl);
		CALL_SYSCALL( 57, sys_setpgid);
		CALL_SYSCALL( 60, sys_umask);
		CALL_SYSCALL( 63, sys_dup2);
		CALL_SYSCALL( 64, sys_getppid);
		CALL_SYSCALL( 65, sys_getpgrp);
		CALL_SYSCALL( 78, compat_sys_gettimeofday);
		CALL_SYSCALL( 83, sys_symlink);
		CALL_SYSCALL( 85, sys_readlink);
		CALL_SYSCALL( 90, sys32_mmap);
		CALL_SYSCALL( 91, sys_unmap);
		CALL_SYSCALL( 99, compat_sys_statfs);
		CALL_SYSCALL(102, compat_sys_socketcall);
		CALL_SYSCALL(108, compat_sys_newfstat);
		CALL_SYSCALL(120, sys32_clone);
		CALL_SYSCALL(122, sys_uname);
		CALL_SYSCALL(125, sys32_mprotect);
		CALL_SYSCALL(136, sys_personality);
		CALL_SYSCALL(140, sys_llseek);
		CALL_SYSCALL(141, compat_sys_getdents);
		CALL_SYSCALL(142, compat_sys_select);
		CALL_SYSCALL(146, compat_sys_writev);
		CALL_SYSCALL(168, sys_poll);
		CALL_SYSCALL(174, sys32_rt_sigaction);
		CALL_SYSCALL(175, sys32_rt_sigprocmask);
		CALL_SYSCALL(183, sys_get_cwd);
		CALL_SYSCALL(191, compat_sys_getrlimit);
		CALL_SYSCALL(192, sys32_mmap2);
		CALL_SYSCALL(195, sys32_stat64);
		CALL_SYSCALL(196, sys32_lstat64);
		CALL_SYSCALL(197, sys32_fstat64);
		CALL_SYSCALL(199, sys_getuid);
		CALL_SYSCALL(200, sys_getgid);
		CALL_SYSCALL(201, sys_geteuid);
		CALL_SYSCALL(202, sys_getegid);
		CALL_SYSCALL(205, sys_getgroups);
		CALL_SYSCALL(213, sys_setuid);
		CALL_SYSCALL(214, sys_setgid);
		CALL_SYSCALL(220, compat_sys_getdents64);
		CALL_SYSCALL(221, compat_sys_fcntl64);
		CALL_SYSCALL(224, sys_gettid);
		CALL_SYSCALL(229, sys_getxattr);
		CALL_SYSCALL(230, sys_lgetxattr);
		CALL_SYSCALL(240, compat_sys_futex);
		CALL_SYSCALL(252, sys_exit_group);
		CALL_SYSCALL(258, sys_set_tid_address);
		CALL_SYSCALL(265, compat_sys_clock_gettime);
		CALL_SYSCALL(268, compat_sys_statfs64);
		CALL_SYSCALL(270, sys_tgkill);
		CALL_SYSCALL(311, compat_sys_set_robust_list);

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
		log("-> (failed) %d", -ErrnoI2L(e));
#endif
		return -ErrnoI2L(e);
	}
}

#if 0
.quad sys_restart_syscall
.quad sys_exit
.quad stub32_fork
.quad sys_read
.quad sys_write
.quad compat_sys_open		/* 5 */
.quad sys_close
.quad sys32_waitpid
.quad sys_creat
.quad sys_link
.quad sys_unlink		/* 10 */
.quad stub32_execve
.quad sys_chdir
.quad compat_sys_time
.quad sys_mknod
.quad sys_chmod		/* 15 */
.quad sys_lchown16
.quad quiet_ni_syscall			/* old break syscall holder */
.quad sys_stat
.quad sys32_lseek
.quad sys_getpid		/* 20 */
.quad compat_sys_mount	/* mount  */
.quad sys_oldumount	/* old_umount  */
.quad sys_setuid16
.quad sys_getuid16
.quad compat_sys_stime	/* stime */		/* 25 */
.quad compat_sys_ptrace	/* ptrace */
.quad sys_alarm
.quad sys_fstat	/* (old)fstat */
.quad sys_pause
.quad compat_sys_utime	/* 30 */
.quad quiet_ni_syscall	/* old stty syscall holder */
.quad quiet_ni_syscall	/* old gtty syscall holder */
.quad sys_access
.quad sys_nice
.quad quiet_ni_syscall	/* 35 */	/* old ftime syscall holder */
.quad sys_sync
.quad sys32_kill
.quad sys_rename
.quad sys_mkdir
.quad sys_rmdir		/* 40 */
.quad sys_dup
.quad sys_pipe
.quad compat_sys_times
.quad quiet_ni_syscall			/* old prof syscall holder */
.quad sys_brk		/* 45 */
.quad sys_setgid16
.quad sys_getgid16
.quad sys_signal
.quad sys_geteuid16
.quad sys_getegid16	/* 50 */
.quad sys_acct
.quad sys_umount			/* new_umount */
.quad quiet_ni_syscall			/* old lock syscall holder */
.quad compat_sys_ioctl
.quad compat_sys_fcntl64		/* 55 */
.quad quiet_ni_syscall			/* old mpx syscall holder */
.quad sys_setpgid
.quad quiet_ni_syscall			/* old ulimit syscall holder */
.quad sys32_olduname
.quad sys_umask		/* 60 */
.quad sys_chroot
.quad compat_sys_ustat
.quad sys_dup2
.quad sys_getppid
.quad sys_getpgrp		/* 65 */
.quad sys_setsid
.quad sys32_sigaction
.quad sys_sgetmask
.quad sys_ssetmask
.quad sys_setreuid16	/* 70 */
.quad sys_setregid16
.quad sys32_sigsuspend
.quad compat_sys_sigpending
.quad sys_sethostname
.quad compat_sys_setrlimit	/* 75 */
.quad compat_sys_old_getrlimit	/* old_getrlimit */
.quad compat_sys_getrusage
.quad compat_sys_gettimeofday
.quad compat_sys_settimeofday
.quad sys_getgroups16	/* 80 */
.quad sys_setgroups16
.quad sys32_old_select
.quad sys_symlink
.quad sys_lstat
.quad sys_readlink		/* 85 */
.quad sys_uselib
.quad sys_swapon
.quad sys_reboot
.quad compat_sys_old_readdir
.quad sys32_mmap		/* 90 */
.quad sys_munmap
.quad sys_truncate
.quad sys_ftruncate
.quad sys_fchmod
.quad sys_fchown16		/* 95 */
.quad sys_getpriority
.quad sys_setpriority
.quad quiet_ni_syscall			/* old profil syscall holder */
.quad compat_sys_statfs
.quad compat_sys_fstatfs		/* 100 */
.quad sys_ioperm
.quad compat_sys_socketcall
.quad sys_syslog
.quad compat_sys_setitimer
.quad compat_sys_getitimer	/* 105 */
.quad compat_sys_newstat
.quad compat_sys_newlstat
.quad compat_sys_newfstat
.quad sys32_uname
.quad stub32_iopl		/* 110 */
.quad sys_vhangup
.quad quiet_ni_syscall	/* old "idle" system call */
.quad sys32_vm86_warning	/* vm86old */
.quad compat_sys_wait4
.quad sys_swapoff		/* 115 */
.quad compat_sys_sysinfo
.quad sys32_ipc
.quad sys_fsync
.quad stub32_sigreturn
.quad stub32_clone		/* 120 */
.quad sys_setdomainname
.quad sys_uname
.quad sys_modify_ldt
.quad compat_sys_adjtimex
.quad sys32_mprotect		/* 125 */
.quad compat_sys_sigprocmask
.quad quiet_ni_syscall		/* create_module */
.quad sys_init_module
.quad sys_delete_module
.quad quiet_ni_syscall		/* 130  get_kernel_syms */
.quad sys32_quotactl
.quad sys_getpgid
.quad sys_fchdir
.quad quiet_ni_syscall	/* bdflush */
.quad sys_sysfs		/* 135 */
.quad sys_personality
.quad quiet_ni_syscall	/* for afs_syscall */
.quad sys_setfsuid16
.quad sys_setfsgid16
.quad sys_llseek		/* 140 */
.quad compat_sys_getdents
.quad compat_sys_select
.quad sys_flock
.quad sys_msync
.quad compat_sys_readv		/* 145 */
.quad compat_sys_writev
.quad sys_getsid
.quad sys_fdatasync
.quad sys32_sysctl	/* sysctl */
.quad sys_mlock		/* 150 */
.quad sys_munlock
.quad sys_mlockall
.quad sys_munlockall
.quad sys_sched_setparam
.quad sys_sched_getparam   /* 155 */
.quad sys_sched_setscheduler
.quad sys_sched_getscheduler
.quad sys_sched_yield
.quad sys_sched_get_priority_max
.quad sys_sched_get_priority_min  /* 160 */
.quad sys32_sched_rr_get_interval
.quad compat_sys_nanosleep
.quad sys_mremap
.quad sys_setresuid16
.quad sys_getresuid16	/* 165 */
.quad sys32_vm86_warning	/* vm86 */
.quad quiet_ni_syscall	/* query_module */
.quad sys_poll
.quad compat_sys_nfsservctl
.quad sys_setresgid16	/* 170 */
.quad sys_getresgid16
.quad sys_prctl
.quad stub32_rt_sigreturn
.quad sys32_rt_sigaction
.quad sys32_rt_sigprocmask	/* 175 */
.quad sys32_rt_sigpending
.quad compat_sys_rt_sigtimedwait
.quad sys32_rt_sigqueueinfo
.quad sys_rt_sigsuspend
.quad sys32_pread		/* 180 */
.quad sys32_pwrite
.quad sys_chown16
.quad sys_getcwd
.quad sys_capget
.quad sys_capset
.quad stub32_sigaltstack
.quad sys32_sendfile
.quad quiet_ni_syscall		/* streams1 */
.quad quiet_ni_syscall		/* streams2 */
.quad stub32_vfork            /* 190 */
.quad compat_sys_getrlimit
.quad sys32_mmap2
.quad sys32_truncate64
.quad sys32_ftruncate64
.quad sys32_stat64		/* 195 */
.quad sys32_lstat64
.quad sys32_fstat64
.quad sys_lchown
.quad sys_getuid
.quad sys_getgid		/* 200 */
.quad sys_geteuid
.quad sys_getegid
.quad sys_setreuid
.quad sys_setregid
.quad sys_getgroups	/* 205 */
.quad sys_setgroups
.quad sys_fchown
.quad sys_setresuid
.quad sys_getresuid
.quad sys_setresgid	/* 210 */
.quad sys_getresgid
.quad sys_chown
.quad sys_setuid
.quad sys_setgid
.quad sys_setfsuid		/* 215 */
.quad sys_setfsgid
.quad sys_pivot_root
.quad sys_mincore
.quad sys_madvise
.quad compat_sys_getdents64	/* 220 getdents64 */
.quad compat_sys_fcntl64
.quad quiet_ni_syscall		/* tux */
.quad quiet_ni_syscall    	/* security */
.quad sys_gettid
.quad sys32_readahead	/* 225 */
.quad sys_setxattr
.quad sys_lsetxattr
.quad sys_fsetxattr
.quad sys_getxattr
.quad sys_lgetxattr	/* 230 */
.quad sys_fgetxattr
.quad sys_listxattr
.quad sys_llistxattr
.quad sys_flistxattr
.quad sys_removexattr	/* 235 */
.quad sys_lremovexattr
.quad sys_fremovexattr
.quad sys_tkill
.quad sys_sendfile64
.quad compat_sys_futex		/* 240 */
.quad compat_sys_sched_setaffinity
.quad compat_sys_sched_getaffinity
.quad sys_set_thread_area
.quad sys_get_thread_area
.quad compat_sys_io_setup	/* 245 */
.quad sys_io_destroy
.quad compat_sys_io_getevents
.quad compat_sys_io_submit
.quad sys_io_cancel
.quad sys32_fadvise64		/* 250 */
.quad quiet_ni_syscall 	/* free_huge_pages */
.quad sys_exit_group
.quad sys32_lookup_dcookie
.quad sys_epoll_create
.quad sys_epoll_ctl		/* 255 */
.quad sys_epoll_wait
.quad sys_remap_file_pages
.quad sys_set_tid_address
.quad compat_sys_timer_create
.quad compat_sys_timer_settime	/* 260 */
.quad compat_sys_timer_gettime
.quad sys_timer_getoverrun
.quad sys_timer_delete
.quad compat_sys_clock_settime
.quad compat_sys_clock_gettime	/* 265 */
.quad compat_sys_clock_getres
.quad compat_sys_clock_nanosleep
.quad compat_sys_statfs64
.quad compat_sys_fstatfs64
.quad sys_tgkill		/* 270 */
.quad compat_sys_utimes
.quad sys32_fadvise64_64
.quad quiet_ni_syscall	/* sys_vserver */
.quad sys_mbind
.quad compat_sys_get_mempolicy	/* 275 */
.quad sys_set_mempolicy
.quad compat_sys_mq_open
.quad sys_mq_unlink
.quad compat_sys_mq_timedsend
.quad compat_sys_mq_timedreceive	/* 280 */
.quad compat_sys_mq_notify
.quad compat_sys_mq_getsetattr
.quad compat_sys_kexec_load	/* reserved for kexec */
.quad compat_sys_waitid
.quad quiet_ni_syscall		/* 285: sys_altroot */
.quad sys_add_key
.quad sys_request_key
.quad sys_keyctl
.quad sys_ioprio_set
.quad sys_ioprio_get		/* 290 */
.quad sys_inotify_init
.quad sys_inotify_add_watch
.quad sys_inotify_rm_watch
.quad sys_migrate_pages
.quad compat_sys_openat		/* 295 */
.quad sys_mkdirat
.quad sys_mknodat
.quad sys_fchownat
.quad compat_sys_futimesat
.quad sys32_fstatat		/* 300 */
.quad sys_unlinkat
.quad sys_renameat
.quad sys_linkat
.quad sys_symlinkat
.quad sys_readlinkat		/* 305 */
.quad sys_fchmodat
.quad sys_faccessat
.quad compat_sys_pselect6
.quad compat_sys_ppoll
.quad sys_unshare		/* 310 */
.quad compat_sys_set_robust_list
.quad compat_sys_get_robust_list
.quad sys_splice
.quad sys32_sync_file_range
.quad sys_tee			/* 315 */
.quad compat_sys_vmsplice
.quad compat_sys_move_pages
.quad sys_getcpu
.quad sys_epoll_pwait
.quad compat_sys_utimensat	/* 320 */
.quad compat_sys_signalfd
.quad sys_timerfd_create
.quad sys_eventfd
.quad sys32_fallocate
.quad compat_sys_timerfd_settime	/* 325 */
.quad compat_sys_timerfd_gettime
.quad compat_sys_signalfd4
.quad sys_eventfd2
.quad sys_epoll_create1
.quad sys_dup3				/* 330 */
.quad sys_pipe2
.quad sys_inotify_init1
.quad compat_sys_preadv
.quad compat_sys_pwritev
.quad compat_sys_rt_tgsigqueueinfo	/* 335 */
.quad sys_perf_event_open
#endif