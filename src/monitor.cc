#include "globals.h"
#include <sys/procfs.h>
#include <sys/fault.h>

class Monitor
{
public:
	Monitor(pid_t pid):
			_pid(pid),
			_ctl_fd(-1),
			_status_fd(-1),
			_as_fd(-1)
	{
		/* Connect to the debugger interface. */

		string path = StringF("/proc/%ld/ctl", pid);
		_ctl_fd = open(path.c_str(), O_WRONLY);
		if (_ctl_fd == -1)
			throw Exception("could not open "+path);

		path = StringF("/proc/%ld/status", pid);
		_status_fd = open(path.c_str(), O_RDONLY);
		if (_status_fd == -1)
			throw Exception("could not open "+path);

		path = StringF("/proc/%ld/as", pid);
		_as_fd = open(path.c_str(), O_RDWR);
		if (_as_fd == -1)
			throw Exception("could not open "+path);

		/* Initialise the debugger. */

		fltset_t faults;
		premptyset(&faults);
		praddset(&faults, FLTACCESS);
		cmd(PCSFAULT, faults);

		sigset_t signals;
		premptyset(&signals);
		praddset(&signals, SIGTRAP);
		cmd(PCSTRACE, signals);
	}

	~Monitor()
	{
		if (_ctl_fd == -1)
			close(_ctl_fd);
		if (_status_fd == -1)
			close(_status_fd);
		if (_as_fd == -1)
			close(_as_fd);
	}

private:
	template <class T> int cmd(PROC_CTL_WORD_TYPE opcode, const T& argument)
	{
		struct
		{
			PROC_CTL_WORD_TYPE opcode;
			T argument;
		} PACKED packet =
		{
			opcode,
			argument
		};

		return write(_ctl_fd, &packet, sizeof(packet));
	}

	int cmd(PROC_CTL_WORD_TYPE opcode)
	{
		return write(_ctl_fd, &opcode, sizeof(opcode));
	}

	template <class T> int userput(u_int32_t address, const T& value)
	{
		lseek(_as_fd, address, SEEK_SET);
		return write(_as_fd, &value, sizeof(value));
	}

	template <class T> int userget(u_int32_t address, T& value)
	{
		lseek(_as_fd, address, SEEK_SET);
		return read(_as_fd, &value, sizeof(value));
	}

	void printregs(gregset_t& regs)
	{
		log("eax = %08x", regs.gregs[EAX]);
		log("ebx = %08x", regs.gregs[EBX]);
		log("ecx = %08x", regs.gregs[ECX]);
		log("edx = %08x", regs.gregs[EDX]);
		log("esi = %08x", regs.gregs[ESI]);
		log("edi = %08x", regs.gregs[EDI]);
		log("ebp = %08x", regs.gregs[EBP]);
		log("eip = %08x", regs.gregs[EIP]);
		log("esp = %08x", regs.gregs[UESP]);
		log(" cs = %08x", regs.gregs[CS]);
		log(" ds = %08x", regs.gregs[DS]);
		log(" ss = %08x", regs.gregs[SS]);
		log(" es = %08x", regs.gregs[ES]);
		log(" fs = %08x", regs.gregs[FS]);
		log(" gs = %08x", regs.gregs[GS]);

		u_int32_t stack[8];
		int i = userget(regs.gregs[UESP], stack);
		if (i == sizeof(stack))
		{
			log("stack: %08x %08x %08x %08x",
					stack[0], stack[1], stack[2], stack[3]);
			log("       %08x %08x %08x %08x",
					stack[4], stack[5], stack[6], stack[7]);
		}

		u_int8_t code[8];
		i = userget(regs.gregs[EIP], code);
		if (i == sizeof(code))
		{
			log("code: %02x %02x %02x %02x %02x %02x %02x %02x",
					code[0], code[1], code[2], code[3],
					code[4], code[5], code[6], code[7]);
		}

	}

	void syscall(gregset_t& regs, MCE* mce)
	{
		//printregs(regs);

		/* Fake a call to SyscallDispatch. */

		regs.gregs[UESP] -= 4;
		int i = userput(regs.gregs[UESP], regs.gregs[EIP]);
		if (i != 4)
			log("unable to write to user space? i=%d errno=%d", i, errno);
		regs.gregs[EIP] = (u_int32_t) mce;

		cmd(PCSREG, regs);
	}

public:
	void Run()
	{
		for (;;)
		{
			int i = cmd(PCWSTOP);
			if (i == -1)
				break;

			pstatus_t status;
			lseek(_status_fd, 0, SEEK_SET);
			i = read(_status_fd, &status, sizeof(status));
			if (i != sizeof(status))
				break;

			int runflags = 0;
			switch (status.pr_lwp.pr_why)
			{
				case PR_FAULTED:
				{
					struct siginfo* siginfo = &status.pr_lwp.pr_info;
					if (siginfo->__data.__fault.__status == 0xc0000005)
					{
						gregset_t& regs = status.pr_lwp.pr_context.uc_mcontext.gregs;
						u16 instruction;
						if (userget(regs.gregs[EIP], instruction) == -1)
						{
							unknown:
							log("stray fault of type %08x!", 0xc0000005);
							printregs(regs);
							break;
						}

						//log("%08x %04x", regs.gregs[EIP], instruction);
						if (instruction == 0x80cd)
						{
							syscall(regs, Linux_MCE);
							runflags |= PRCFAULT;
						}
						else if (instruction == 0x65f0)
						{
							syscall(regs, InterpretGS_MCE);
							runflags |= PRCFAULT;
						}
						else if ((instruction & 0xff) == 0x65)
						{
							syscall(regs, InterpretGS_MCE);
							runflags |= PRCFAULT;
						}
						else
							goto unknown;
#if 0
						else if ((instruction & 0xff) == 0x65)
						{
							if (regs.gregs[GS] == 0)
							{
								//log("instruction %08x @ %08x", instruction, regs.gregs[EIP]);
								syscall(regs, ReloadGS_MCE);
								runflags |= PRCFAULT;
							}
							else
								goto unknown;
						}
#endif
					}
					break;

				}

				case PR_SIGNALLED:
				{
					struct siginfo* siginfo = &status.pr_lwp.pr_info;
					if (siginfo->si_signo == SIGTRAP)
					{
						gregset_t& regs = status.pr_lwp.pr_context.uc_mcontext.gregs;
						log("eip = %x", regs.gregs[EIP]);
						runflags |= PRCSIG;
					}
					break;
				}

				default:
					log("stopped due to %d?", status.pr_lwp.pr_why);
			}

			cmd(PCRUN, runflags);
		}
	}

private:
	pid_t _pid;
	int _ctl_fd;
	int _status_fd;
	int _as_fd;
};

void StartMonitor(void)
{
	pid_t slavepid = getpid();

	int fd[2];
	int result = pipe(fd);
	if (result == -1)
		error("unable to start monitor (pipe() errno %d)", errno);

	switch (fork())
	{
		case 0:
			close(fd[0]);
			try
			{
				Monitor monitor(slavepid);

				{
					int result = 0;
					write(fd[1], &result, sizeof(result));
					close(fd[1]);
				}

				monitor.Run();
				log("slave went away? terminating");
			}
			catch (const Exception& e)
			{
				Error(e);
			}
			_exit(0);

		case -1:
			error("unable to start syscall monitor (fork() errno %d)", errno);

		default:
		{
			close(fd[1]);

			/* Wait for master to start up. */
			int result;
			int i = read(fd[0], &result, sizeof(result));
			close(fd[0]);
			if (i == -1)
				error("couldn't communicate with syscall monitor");

			return;
		}
	}
}
