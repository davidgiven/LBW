#include "globals.h"
#include "RawFD.h"
#include "termios.h"

#define LINUX_F_RDLCK         0
#define LINUX_F_WRLCK         1
#define LINUX_F_UNLCK         2

#pragma pack(push, 1)
struct linux_flock
{
	u16 l_type;
	u16 l_whence;
	u32 l_start;
	u32 l_len;
	u16 l_pid;
};
#pragma pack(pop)

static void convert(struct linux_flock& lfl, struct flock& ifl)
{
	switch (lfl.l_type)
	{
		case LINUX_F_RDLCK: ifl.l_type = F_RDLCK; break;
		case LINUX_F_WRLCK: ifl.l_type = F_WRLCK; break;
		case LINUX_F_UNLCK: ifl.l_type = F_UNLCK; break;
		default: throw EINVAL;
	}

	ifl.l_whence = lfl.l_whence;
	ifl.l_start = lfl.l_start;
	ifl.l_len = lfl.l_len;
	ifl.l_pid = lfl.l_pid;
}

int RawFD::Fcntl(int cmd, u_int32_t argument)
{
	switch (cmd)
	{
		case LINUX_F_GETFL:
		{
			int result = fcntl(_realfd, F_GETFL, NULL);
			if (result == -1)
				throw errno;
			return FileFlagsI2L(result);
		}

		case LINUX_F_SETFL:
		{
			int iflags = FileFlagsL2I(argument);
			log("setting flags for realfd %d to %x", _realfd, iflags);
			int result = fcntl(_realfd, F_SETFL, iflags);
			return SysError(result);
		}

		case LINUX_F_SETLK:
		{
			struct linux_flock& lfl = *(struct linux_flock*) argument;
			struct flock ifl;
			convert(lfl, ifl);

			int result = fcntl(_realfd, F_SETLK, &ifl);
			return SysError(result);
		}
	}

	return FD::Fcntl(cmd, argument);
}
