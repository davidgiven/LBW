#include "globals.h"
#include "syscalls.h"

const char* const SyscallNames[NUM_SYSCALLS] =
{
	"sys_restart_syscall",
	"sys_exit",
	"stub32_fork",
	"sys_read",
	"sys_write",
	"compat_sys_open", 		/* 5 */
	"sys_close",
	"sys32_waitpid",
	"sys_creat",
	"sys_link",
	"sys_unlink", 		/* 10 */
	"stub32_execve",
	"sys_chdir",
	"compat_sys_time",
	"sys_mknod",
	"sys_chmod", 		/* 15 */
	"sys_lchown16",
	"quiet_ni_syscall", 			/* old break syscall holder */
	"sys_stat",
	"sys32_lseek",
	"sys_getpid", 		/* 20 */
	"compat_sys_mount", 	/* mount  */
	"sys_oldumount", 	/* old_umount  */
	"sys_setuid16",
	"sys_getuid16",
	"compat_sys_stime", 	/* stime */		/* 25 */
	"compat_sys_ptrace", 	/* ptrace */
	"sys_alarm",
	"sys_fstat", 	/* (old)fstat */
	"sys_pause",
	"compat_sys_utime", 	/* 30 */
	"quiet_ni_syscall", 	/* old stty syscall holder */
	"quiet_ni_syscall", 	/* old gtty syscall holder */
	"sys_access",
	"sys_nice",
	"quiet_ni_syscall", 	/* 35 */	/* old ftime syscall holder */
	"sys_sync",
	"sys32_kill",
	"sys_rename",
	"sys_mkdir",
	"sys_rmdir", 		/* 40 */
	"sys_dup",
	"sys_pipe",
	"compat_sys_times",
	"quiet_ni_syscall", 			/* old prof syscall holder */
	"sys_brk", 		/* 45 */
	"sys_setgid16",
	"sys_getgid16",
	"sys_signal",
	"sys_geteuid16",
	"sys_getegid16", 	/* 50 */
	"sys_acct",
	"sys_umount", 			/* new_umount */
	"quiet_ni_syscall", 			/* old lock syscall holder */
	"compat_sys_ioctl",
	"compat_sys_fcntl64", 		/* 55 */
	"quiet_ni_syscall", 			/* old mpx syscall holder */
	"sys_setpgid",
	"quiet_ni_syscall", 			/* old ulimit syscall holder */
	"sys32_olduname",
	"sys_umask", 		/* 60 */
	"sys_chroot",
	"compat_sys_ustat",
	"sys_dup2",
	"sys_getppid",
	"sys_getpgrp", 		/* 65 */
	"sys_setsid",
	"sys32_sigaction",
	"sys_sgetmask",
	"sys_ssetmask",
	"sys_setreuid16", 	/* 70 */
	"sys_setregid16",
	"sys32_sigsuspend",
	"compat_sys_sigpending",
	"sys_sethostname",
	"compat_sys_setrlimit", 	/* 75 */
	"compat_sys_old_getrlimit", 	/* old_getrlimit */
	"compat_sys_getrusage",
	"compat_sys_gettimeofday",
	"compat_sys_settimeofday",
	"sys_getgroups16", 	/* 80 */
	"sys_setgroups16",
	"sys32_old_select",
	"sys_symlink",
	"sys_lstat",
	"sys_readlink", 		/* 85 */
	"sys_uselib",
	"sys_swapon",
	"sys_reboot",
	"compat_sys_old_readdir",
	"sys32_mmap", 		/* 90 */
	"sys_munmap",
	"sys_truncate",
	"sys_ftruncate",
	"sys_fchmod",
	"sys_fchown16", 		/* 95 */
	"sys_getpriority",
	"sys_setpriority",
	"quiet_ni_syscall", 			/* old profil syscall holder */
	"compat_sys_statfs",
	"compat_sys_fstatfs", 		/* 100 */
	"sys_ioperm",
	"compat_sys_socketcall",
	"sys_syslog",
	"compat_sys_setitimer",
	"compat_sys_getitimer", 	/* 105 */
	"compat_sys_newstat",
	"compat_sys_newlstat",
	"compat_sys_newfstat",
	"sys32_uname",
	"stub32_iopl", 		/* 110 */
	"sys_vhangup",
	"quiet_ni_syscall", 	/* old "idle" system call */
	"sys32_vm86_warning", 	/* vm86old */
	"compat_sys_wait4",
	"sys_swapoff", 		/* 115 */
	"compat_sys_sysinfo",
	"sys32_ipc",
	"sys_fsync",
	"stub32_sigreturn",
	"stub32_clone", 		/* 120 */
	"sys_setdomainname",
	"sys_uname",
	"sys_modify_ldt",
	"compat_sys_adjtimex",
	"sys32_mprotect", 		/* 125 */
	"compat_sys_sigprocmask",
	"quiet_ni_syscall", 		/* create_module */
	"sys_init_module",
	"sys_delete_module",
	"quiet_ni_syscall", 		/* 130  get_kernel_syms */
	"sys32_quotactl",
	"sys_getpgid",
	"sys_fchdir",
	"quiet_ni_syscall", 	/* bdflush */
	"sys_sysfs", 		/* 135 */
	"sys_personality",
	"quiet_ni_syscall", 	/* for afs_syscall */
	"sys_setfsuid16",
	"sys_setfsgid16",
	"sys_llseek", 		/* 140 */
	"compat_sys_getdents",
	"compat_sys_select",
	"sys_flock",
	"sys_msync",
	"compat_sys_readv", 		/* 145 */
	"compat_sys_writev",
	"sys_getsid",
	"sys_fdatasync",
	"sys32_sysctl", 	/* sysctl */
	"sys_mlock", 		/* 150 */
	"sys_munlock",
	"sys_mlockall",
	"sys_munlockall",
	"sys_sched_setparam",
	"sys_sched_getparam",    /* 155 */
	"sys_sched_setscheduler",
	"sys_sched_getscheduler",
	"sys_sched_yield",
	"sys_sched_get_priority_max",
	"sys_sched_get_priority_min",   /* 160 */
	"sys32_sched_rr_get_interval",
	"compat_sys_nanosleep",
	"sys_mremap",
	"sys_setresuid16",
	"sys_getresuid16", 	/* 165 */
	"sys32_vm86_warning", 	/* vm86 */
	"quiet_ni_syscall", 	/* query_module */
	"sys_poll",
	"compat_sys_nfsservctl",
	"sys_setresgid16", 	/* 170 */
	"sys_getresgid16",
	"sys_prctl",
	"stub32_rt_sigreturn",
	"sys32_rt_sigaction",
	"sys32_rt_sigprocmask", 	/* 175 */
	"sys32_rt_sigpending",
	"compat_sys_rt_sigtimedwait",
	"sys32_rt_sigqueueinfo",
	"sys_rt_sigsuspend",
	"sys32_pread", 		/* 180 */
	"sys32_pwrite",
	"sys_chown16",
	"sys_getcwd",
	"sys_capget",
	"sys_capset",
	"stub32_sigaltstack",
	"sys32_sendfile",
	"quiet_ni_syscall", 		/* streams1 */
	"quiet_ni_syscall", 		/* streams2 */
	"stub32_vfork",             /* 190 */
	"compat_sys_getrlimit",
	"sys32_mmap2",
	"sys32_truncate64",
	"sys32_ftruncate64",
	"sys32_stat64", 		/* 195 */
	"sys32_lstat64",
	"sys32_fstat64",
	"sys_lchown",
	"sys_getuid",
	"sys_getgid", 		/* 200 */
	"sys_geteuid",
	"sys_getegid",
	"sys_setreuid",
	"sys_setregid",
	"sys_getgroups", 	/* 205 */
	"sys_setgroups",
	"sys_fchown",
	"sys_setresuid",
	"sys_getresuid",
	"sys_setresgid", 	/* 210 */
	"sys_getresgid",
	"sys_chown",
	"sys_setuid",
	"sys_setgid",
	"sys_setfsuid", 		/* 215 */
	"sys_setfsgid",
	"sys_pivot_root",
	"sys_mincore",
	"sys_madvise",
	"compat_sys_getdents64", 	/* 220 getdents64 */
	"compat_sys_fcntl64",
	"quiet_ni_syscall", 		/* tux */
	"quiet_ni_syscall",     	/* security */
	"sys_gettid",
	"sys32_readahead", 	/* 225 */
	"sys_setxattr",
	"sys_lsetxattr",
	"sys_fsetxattr",
	"sys_getxattr",
	"sys_lgetxattr", 	/* 230 */
	"sys_fgetxattr",
	"sys_listxattr",
	"sys_llistxattr",
	"sys_flistxattr",
	"sys_removexattr", 	/* 235 */
	"sys_lremovexattr",
	"sys_fremovexattr",
	"sys_tkill",
	"sys_sendfile64",
	"compat_sys_futex", 		/* 240 */
	"compat_sys_sched_setaffinity",
	"compat_sys_sched_getaffinity",
	"sys_set_thread_area",
	"sys_get_thread_area",
	"compat_sys_io_setup", 	/* 245 */
	"sys_io_destroy",
	"compat_sys_io_getevents",
	"compat_sys_io_submit",
	"sys_io_cancel",
	"sys32_fadvise64", 		/* 250 */
	"quiet_ni_syscall",  	/* free_huge_pages */
	"sys_exit_group",
	"sys32_lookup_dcookie",
	"sys_epoll_create",
	"sys_epoll_ctl", 		/* 255 */
	"sys_epoll_wait",
	"sys_remap_file_pages",
	"sys_set_tid_address",
	"compat_sys_timer_create",
	"compat_sys_timer_settime", 	/* 260 */
	"compat_sys_timer_gettime",
	"sys_timer_getoverrun",
	"sys_timer_delete",
	"compat_sys_clock_settime",
	"compat_sys_clock_gettime", 	/* 265 */
	"compat_sys_clock_getres",
	"compat_sys_clock_nanosleep",
	"compat_sys_statfs64",
	"compat_sys_fstatfs64",
	"sys_tgkill", 		/* 270 */
	"compat_sys_utimes",
	"sys32_fadvise64_64",
	"quiet_ni_syscall", 	/* sys_vserver */
	"sys_mbind",
	"compat_sys_get_mempolicy", 	/* 275 */
	"sys_set_mempolicy",
	"compat_sys_mq_open",
	"sys_mq_unlink",
	"compat_sys_mq_timedsend",
	"compat_sys_mq_timedreceive", 	/* 280 */
	"compat_sys_mq_notify",
	"compat_sys_mq_getsetattr",
	"compat_sys_kexec_load", 	/* reserved for kexec */
	"compat_sys_waitid",
	"quiet_ni_syscall", 		/* 285: sys_altroot */
	"sys_add_key",
	"sys_request_key",
	"sys_keyctl",
	"sys_ioprio_set",
	"sys_ioprio_get", 		/* 290 */
	"sys_inotify_init",
	"sys_inotify_add_watch",
	"sys_inotify_rm_watch",
	"sys_migrate_pages",
	"compat_sys_openat", 		/* 295 */
	"sys_mkdirat",
	"sys_mknodat",
	"sys_fchownat",
	"compat_sys_futimesat",
	"sys32_fstatat", 		/* 300 */
	"sys_unlinkat",
	"sys_renameat",
	"sys_linkat",
	"sys_symlinkat",
	"sys_readlinkat", 		/* 305 */
	"sys_fchmodat",
	"sys_faccessat",
	"compat_sys_pselect6",
	"compat_sys_ppoll",
	"sys_unshare", 		/* 310 */
	"compat_sys_set_robust_list",
	"compat_sys_get_robust_list",
	"sys_splice",
	"sys32_sync_file_range",
	"sys_tee", 			/* 315 */
	"compat_sys_vmsplice",
	"compat_sys_move_pages",
	"sys_getcpu",
	"sys_epoll_pwait",
	"compat_sys_utimensat", 	/* 320 */
	"compat_sys_signalfd",
	"sys_timerfd_create",
	"sys_eventfd",
	"sys32_fallocate",
	"compat_sys_timerfd_settime", 	/* 325 */
	"compat_sys_timerfd_gettime",
	"compat_sys_signalfd4",
	"sys_eventfd2",
	"sys_epoll_create1",
	"sys_dup3", 				/* 330 */
	"sys_pipe2",
	"sys_inotify_init1",
	"compat_sys_preadv",
	"compat_sys_pwritev",
	"compat_sys_rt_tgsigqueueinfo", 	/* 335 */
	"sys_perf_event_open"
};
