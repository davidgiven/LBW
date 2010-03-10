/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "file.h"

u_int32_t FileFlagsL2I(u_int32_t flags)
{
	int iflags = 0;
	switch (flags & LINUX_O_ACCMODE)
	{
		case LINUX_O_RDONLY: iflags = O_RDONLY; break;
		case LINUX_O_WRONLY: iflags = O_WRONLY; break;
		case LINUX_O_RDWR:   iflags = O_RDWR; break;
	}

	if (flags & LINUX_O_CREAT)
		iflags |= O_CREAT;
	if (flags & LINUX_O_EXCL)
		iflags |= O_EXCL;
	if (flags & LINUX_O_NOCTTY)
		iflags |= O_NOCTTY;
	if (flags & LINUX_O_TRUNC)
		iflags |= O_TRUNC;
	if (flags & LINUX_O_APPEND)
		iflags |= O_APPEND;
	if (flags & LINUX_O_NONBLOCK)
		iflags |= O_NONBLOCK;
	if (flags & LINUX_O_SYNC)
		iflags |= O_SYNC;

	/* Ignore LINUX_O_LARGEFILE and LINUX_O_NOATIME */

	return iflags;
}

u_int32_t FileFlagsI2L(u_int32_t flags)
{
	int lflags = 0;
	switch (flags & O_ACCMODE)
	{
		case O_RDONLY: lflags = LINUX_O_RDONLY; break;
		case O_WRONLY: lflags = LINUX_O_WRONLY; break;
		case O_RDWR:   lflags = LINUX_O_RDWR; break;
	}

	if (flags & O_CREAT)
		lflags |= LINUX_O_CREAT;
	if (flags & O_EXCL)
		lflags |= LINUX_O_EXCL;
	if (flags & O_NOCTTY)
		lflags |= LINUX_O_NOCTTY;
	if (flags & O_TRUNC)
		lflags |= LINUX_O_TRUNC;
	if (flags & O_APPEND)
		lflags |= LINUX_O_APPEND;
	if (flags & O_NONBLOCK)
		lflags |= LINUX_O_NONBLOCK;
	if (flags & O_SYNC)
		lflags |= LINUX_O_SYNC;

	return lflags;
}

void Convert(struct stat& is, struct linux_stat64& ls)
{
	ls.st_dev = is.st_dev;
	ls.st_ino = is.st_ino;
	ls.st_mode = is.st_mode;
	ls.st_nlink = is.st_nlink;
	ls.st_uid = is.st_uid;
	ls.st_gid = is.st_gid;
	ls.st_rdev = is.st_rdev;
	ls.st_size = is.st_size;
	ls.st_blksize = is.st_blksize;
	ls.st_blocks = is.st_blocks;
	ls.st_atime = is.st_atime;
	ls.st_atime_nsec = 0;
	ls.st_mtime = is.st_mtime;
	ls.st_mtime_nsec = 0;
	ls.st_ctime = is.st_ctime;
	ls.st_ctime_nsec = 0;
}
