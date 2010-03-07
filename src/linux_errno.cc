#include "globals.h"

int ErrnoI2L(int e)
{
	switch (e)
	{
		case EBADF:           return LINUX_EBADF;
		case EEXIST:          return LINUX_EEXIST;
		case EFAULT:          return LINUX_EFAULT;
		case EINVAL:          return LINUX_EINVAL;
		case EIO:             return LINUX_EIO;
		case EISDIR:          return LINUX_EISDIR;
		case ENOENT:          return LINUX_ENOENT;
		case ENOMEM:          return LINUX_ENOMEM;
		case ENOPROTOOPT:     return LINUX_ENOPROTOOPT;
		case ENOSYS:          return LINUX_ENOSYS;
		case ENXIO:           return LINUX_ENXIO;
		case EOPNOTSUPP:      return LINUX_EOPNOTSUPP;
		case EPERM:           return LINUX_EPERM;
		case EPROTONOSUPPORT: return LINUX_EPROTONOSUPPORT;
		case EPROTOTYPE:      return LINUX_EPROTOTYPE;
		case ESRCH:           return LINUX_ESRCH;
	}

	printf("interix errno %d!\n", e);
	return e;
}

int SysError(int e)
{
	if (e == -1)
		return -ErrnoI2L(errno);
	return e;
}
