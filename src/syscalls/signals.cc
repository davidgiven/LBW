/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "syscalls.h"
#include <signal.h>

typedef u_int64_t linux_compat_sigset_t;

struct linux_sigaction32 {
	void (*sa_handler)(int);
	unsigned int sa_flags;
	void* sa_restorer;
	linux_compat_sigset_t sa_mask;
};

#define LINUX_SIGHUP		 1
#define LINUX_SIGINT		 2
#define LINUX_SIGQUIT		 3
#define LINUX_SIGILL		 4
#define LINUX_SIGTRAP		 5
#define LINUX_SIGABRT		 6
#define LINUX_SIGBUS		 7
#define LINUX_SIGFPE		 8
#define LINUX_SIGKILL		 9
#define LINUX_SIGUSR1		10
#define LINUX_SIGSEGV		11
#define LINUX_SIGUSR2		12
#define LINUX_SIGPIPE		13
#define LINUX_SIGALRM		14
#define LINUX_SIGTERM		15
#define LINUX_SIGSTKFLT     16
#define LINUX_SIGCHLD		17
#define LINUX_SIGCONT		18
#define LINUX_SIGSTOP		19
#define LINUX_SIGTSTP		20
#define LINUX_SIGTTIN		21
#define LINUX_SIGTTOU		22
#define LINUX_SIGURG		23
#define LINUX_SIGXCPU		24
#define LINUX_SIGXFSZ		25
#define LINUX_SIGVTALRM	    26
#define LINUX_SIGPROF		27
#define LINUX_SIGWINCH      28
#define LINUX_SIGIO         29
#define LINUX_SIGPWR		30
#define LINUX_SIGSYS		31

#define LINUX_SIG_BLOCK          0	/* for blocking signals */
#define LINUX_SIG_UNBLOCK        1	/* for unblocking signals */
#define LINUX_SIG_SETMASK        2	/* for setting the signal mask */

#define LINUX_SA_NOCLDSTOP	0x00000001u
#define LINUX_SA_NOCLDWAIT	0x00000002u
#define LINUX_SA_SIGINFO	0x00000004u
#define LINUX_SA_ONSTACK	0x08000000u
#define LINUX_SA_RESTART	0x10000000u
#define LINUX_SA_NODEFER	0x40000000u
#define LINUX_SA_RESETHAND	0x80000000u

static bool linux_sigismember(linux_compat_sigset_t& ls, int member)
{
	return ls & (1 << (member-1));
}

static void linux_sigaddset(linux_compat_sigset_t& ls, int member)
{
	ls |= (1 << (member-1));
}

#define CONVERT_SIG_L2I(name) \
	if (linux_sigismember(ls, LINUX_##name)) sigaddset(&is, name)

static void convert_sigset_l2i(linux_compat_sigset_t& ls, sigset_t& is)
{
	sigemptyset(&is);
	CONVERT_SIG_L2I(SIGHUP);
	CONVERT_SIG_L2I(SIGINT);
	CONVERT_SIG_L2I(SIGQUIT);
	CONVERT_SIG_L2I(SIGILL);
	CONVERT_SIG_L2I(SIGTRAP);
	CONVERT_SIG_L2I(SIGABRT);
	CONVERT_SIG_L2I(SIGBUS);
	CONVERT_SIG_L2I(SIGFPE);
	CONVERT_SIG_L2I(SIGKILL);
	CONVERT_SIG_L2I(SIGUSR1);
	CONVERT_SIG_L2I(SIGSEGV);
	CONVERT_SIG_L2I(SIGUSR2);
	CONVERT_SIG_L2I(SIGPIPE);
	CONVERT_SIG_L2I(SIGALRM);
	CONVERT_SIG_L2I(SIGTERM);
	CONVERT_SIG_L2I(SIGCHLD);
	CONVERT_SIG_L2I(SIGCONT);
	CONVERT_SIG_L2I(SIGSTOP);
	CONVERT_SIG_L2I(SIGTSTP);
	CONVERT_SIG_L2I(SIGTTIN);
	CONVERT_SIG_L2I(SIGTTOU);
	CONVERT_SIG_L2I(SIGURG);
	CONVERT_SIG_L2I(SIGXCPU);
	CONVERT_SIG_L2I(SIGXFSZ);
	CONVERT_SIG_L2I(SIGVTALRM);
	CONVERT_SIG_L2I(SIGPROF);
	CONVERT_SIG_L2I(SIGWINCH);
	CONVERT_SIG_L2I(SIGIO);
	CONVERT_SIG_L2I(SIGSYS);
}

#define CONVERT_SIG_I2L(name) \
	if (sigismember(&is, name)) linux_sigaddset(ls, LINUX_##name)

static void convert_sigset_i2l(sigset_t& is, linux_compat_sigset_t& ls)
{
	sigemptyset(&is);
	CONVERT_SIG_I2L(SIGHUP);
	CONVERT_SIG_I2L(SIGINT);
	CONVERT_SIG_I2L(SIGQUIT);
	CONVERT_SIG_I2L(SIGILL);
	CONVERT_SIG_I2L(SIGTRAP);
	CONVERT_SIG_I2L(SIGABRT);
	CONVERT_SIG_I2L(SIGBUS);
	CONVERT_SIG_I2L(SIGFPE);
	CONVERT_SIG_I2L(SIGKILL);
	CONVERT_SIG_I2L(SIGUSR1);
	CONVERT_SIG_I2L(SIGSEGV);
	CONVERT_SIG_I2L(SIGUSR2);
	CONVERT_SIG_I2L(SIGPIPE);
	CONVERT_SIG_I2L(SIGALRM);
	CONVERT_SIG_I2L(SIGTERM);
	CONVERT_SIG_I2L(SIGCHLD);
	CONVERT_SIG_I2L(SIGCONT);
	CONVERT_SIG_I2L(SIGSTOP);
	CONVERT_SIG_I2L(SIGTSTP);
	CONVERT_SIG_I2L(SIGTTIN);
	CONVERT_SIG_I2L(SIGTTOU);
	CONVERT_SIG_I2L(SIGURG);
	CONVERT_SIG_I2L(SIGXCPU);
	CONVERT_SIG_I2L(SIGXFSZ);
	CONVERT_SIG_I2L(SIGVTALRM);
	CONVERT_SIG_I2L(SIGPROF);
	CONVERT_SIG_I2L(SIGWINCH);
	CONVERT_SIG_I2L(SIGIO);
	CONVERT_SIG_I2L(SIGSYS);
}

#define CONVERT_SIGNAL_L2I(name) \
		case LINUX_##name: return name

static int convert_signal_l2i(int signal)
{
	switch(signal)
	{
		CONVERT_SIGNAL_L2I(SIGHUP);
		CONVERT_SIGNAL_L2I(SIGINT);
		CONVERT_SIGNAL_L2I(SIGQUIT);
		CONVERT_SIGNAL_L2I(SIGILL);
		CONVERT_SIGNAL_L2I(SIGTRAP);
		CONVERT_SIGNAL_L2I(SIGABRT);
		CONVERT_SIGNAL_L2I(SIGBUS);
		CONVERT_SIGNAL_L2I(SIGFPE);
		CONVERT_SIGNAL_L2I(SIGKILL);
		CONVERT_SIGNAL_L2I(SIGUSR1);
		CONVERT_SIGNAL_L2I(SIGSEGV);
		CONVERT_SIGNAL_L2I(SIGUSR2);
		CONVERT_SIGNAL_L2I(SIGPIPE);
		CONVERT_SIGNAL_L2I(SIGALRM);
		CONVERT_SIGNAL_L2I(SIGTERM);
		CONVERT_SIGNAL_L2I(SIGCHLD);
		CONVERT_SIGNAL_L2I(SIGCONT);
		CONVERT_SIGNAL_L2I(SIGSTOP);
		CONVERT_SIGNAL_L2I(SIGTSTP);
		CONVERT_SIGNAL_L2I(SIGTTIN);
		CONVERT_SIGNAL_L2I(SIGTTOU);
		CONVERT_SIGNAL_L2I(SIGURG);
		CONVERT_SIGNAL_L2I(SIGXCPU);
		CONVERT_SIGNAL_L2I(SIGXFSZ);
		CONVERT_SIGNAL_L2I(SIGVTALRM);
		CONVERT_SIGNAL_L2I(SIGPROF);
		CONVERT_SIGNAL_L2I(SIGWINCH);
		CONVERT_SIGNAL_L2I(SIGIO);
		CONVERT_SIGNAL_L2I(SIGSYS);
	}
	return -1;
}

#define COPYBIT_I2L(field, name) \
	if (is.field & name) ls.field |= LINUX_ ## name

static void convert_sigaction_i2l(struct sigaction& is, struct linux_sigaction32& ls)
{
	memset(&ls, 0, sizeof(ls));

	COPYBIT_I2L(sa_flags, SA_NOCLDSTOP);
	COPYBIT_I2L(sa_flags, SA_RESETHAND);
	COPYBIT_I2L(sa_flags, SA_RESTART);

	ls.sa_handler = is.sa_handler;
	convert_sigset_i2l(is.sa_mask, ls.sa_mask);
}

#define COPYBIT_L2I(field, name) \
	if (ls.field & LINUX_##name) is.field |= name

#define CHECKBIT_L2I(field, name) \
	if (ls.field & LINUX_##name) Warning("unsupported sigaction() flag " #name)

static void convert_sigaction_l2i(struct linux_sigaction32& ls, struct sigaction& is)
{
	memset(&is, 0, sizeof(is));

	COPYBIT_L2I(sa_flags, SA_NOCLDSTOP);
	COPYBIT_L2I(sa_flags, SA_RESETHAND);
	COPYBIT_L2I(sa_flags, SA_RESTART);
#if 0
	CHECKBIT_L2I(sa_flags, SA_NOCLDWAIT);
	CHECKBIT_L2I(sa_flags, SA_SIGINFO);
	CHECKBIT_L2I(sa_flags, SA_ONSTACK);
	CHECKBIT_L2I(sa_flags, SA_NODEFER);
#endif

	is.sa_handler = ls.sa_handler;
	convert_sigset_l2i(ls.sa_mask, is.sa_mask);
}

SYSCALL(sys32_rt_sigprocmask)
{
	int how = arg.a0.s;
	linux_compat_sigset_t* liset = (linux_compat_sigset_t*) arg.a1.p;
	linux_compat_sigset_t* loset = (linux_compat_sigset_t*) arg.a2.p;
	int sigsetsize = arg.a3.s;
	assert(sigsetsize == sizeof(linux_compat_sigset_t));

	int ihow;
	switch (how)
	{
		case LINUX_SIG_BLOCK:   ihow = SIG_BLOCK; break;
		case LINUX_SIG_UNBLOCK: ihow = SIG_UNBLOCK; break;
		case LINUX_SIG_SETMASK: ihow = SIG_SETMASK; break;
		default:
			return -LINUX_EINVAL;
	}

	sigset_t iiset;
	if (liset)
		convert_sigset_l2i(*liset, iiset);

	sigset_t ioset;
	int result = sigprocmask(ihow,
			liset ? &iiset : NULL,
			&ioset);
	if (result == -1)
		throw errno;

	if (loset)
		convert_sigset_i2l(ioset, *loset);
	return 0;
}

SYSCALL(sys32_rt_sigaction)
{
	int sig = arg.a0.s;
	struct linux_sigaction32* lact = (struct linux_sigaction32*) arg.a1.p;
	struct linux_sigaction32* loact = (struct linux_sigaction32*) arg.a2.p;
	unsigned int sigsetsize = arg.a3.u;
	assert(sigsetsize == sizeof(linux_compat_sigset_t));

	int isig = convert_signal_l2i(sig);
	if (isig == -1)
	{
		Warning("signal %d not supported on Interix", sig);
		return -LINUX_EINVAL;
	}

	struct sigaction iact;
	struct sigaction ioact;

	if (lact)
		convert_sigaction_l2i(*lact, iact);

	int result = 0;
	if (lact && loact)
		result = sigaction(isig, &iact, &ioact);
	else if (lact && !loact)
		result = sigaction(isig, &iact, NULL);
	else if (!lact && loact)
		result = sigaction(isig, NULL, &ioact);

	if (result == -1)
		return -ErrnoI2L(errno);

	if (loact)
		convert_sigaction_i2l(ioact, *loact);
	return result;
}

SYSCALL(sys_tgkill)
{
	pid_t tgid = arg.a0.u;
	pid_t pid = arg.a1.u;
	int sig = arg.a2.u;

	if (tgid != pid)
	{
		log("tgkill(%d, %d, %d) not supported yet", tgid, pid, sig);
		throw EINVAL;
	}

	int isig = convert_signal_l2i(sig);
	if (isig == -1)
	{
		log("signal %d not supported on Interix", sig);
		throw EINVAL;
	}

	int result = kill(pid, isig);
	CheckError(result);
	return 0;
}


SYSCALL(sys32_kill)
{
	pid_t pid = arg.a0.u;
	int lsig = arg.a1.s;

	int isig = convert_signal_l2i(lsig);
	if (isig == -1)
	{
		log("signal %d not supported on Interix", lsig);
		return -LINUX_EINVAL;
	}

	int result = kill(pid, isig);
	if (result == -1)
		throw errno;
	return 0;
}

SYSCALL(stub32_sigaltstack)
{
	throw ENOSYS;
}
