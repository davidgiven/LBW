/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"

int ErrnoI2L(int e)
{
	switch (e)
	{
		case EAGAIN:          return LINUX_EAGAIN;
		case EBADF:           return LINUX_EBADF;
		case ECHILD:          return LINUX_ECHILD;
		case EEXIST:          return LINUX_EEXIST;
		case EFAULT:          return LINUX_EFAULT;
		case EINPROGRESS:     return LINUX_EINPROGRESS;
		case EINTR:           return LINUX_EINTR;
		case EINVAL:          return LINUX_EINVAL;
		case EIO:             return LINUX_EIO;
		case EISCONN:         return LINUX_EISCONN;
		case EISDIR:          return LINUX_EISDIR;
		case ENOENT:          return LINUX_ENOENT;
		case ENOEXEC:         return LINUX_ENOEXEC;
		case ENOMEM:          return LINUX_ENOMEM;
		case ENOPROTOOPT:     return LINUX_ENOPROTOOPT;
		case ENOSYS:          return LINUX_ENOSYS;
		case ENOTDIR:         return LINUX_ENOTDIR;
		case ENOTEMPTY:       return LINUX_ENOTEMPTY;
		case ENOTTY:          return LINUX_ENOTTY;
		case ENXIO:           return LINUX_ENXIO;
		case EOPNOTSUPP:      return LINUX_EOPNOTSUPP;
		case EPERM:           return LINUX_EPERM;
		case EPIPE:           return LINUX_EPIPE;
		case EPROTONOSUPPORT: return LINUX_EPROTONOSUPPORT;
		case EPROTOTYPE:      return LINUX_EPROTOTYPE;
		case ESPIPE:          return LINUX_ESPIPE;
		case ESRCH:           return LINUX_ESRCH;
		case ETIMEDOUT:       return LINUX_ETIMEDOUT;
		case EXDEV:           return LINUX_EXDEV;
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
