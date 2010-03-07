#include "globals.h"
#include "syscalls.h"
#include "filesystem/VFS.h"
#include "filesystem/FD.h"
#include "filesystem/file.h"

SYSCALL(compat_sys_newfstat)
{
	int fd = arg.a0.s;
	struct linux_compat_stat& ls = *(struct linux_compat_stat*) arg.a1.p;

	Ref<FD> fdo = FD::Get(fd);
	struct stat is;
	fdo->Fstat(is);

	if (is.st_dev > 0x10000)
		throw -LINUX_EOVERFLOW;
	ls.st_dev = is.st_dev;
	ls.st_ino = is.st_ino;
	ls.st_mode = is.st_mode;
	ls.st_nlink = is.st_nlink;
	if (is.st_uid > 0x10000)
		throw -LINUX_EOVERFLOW;
	ls.st_uid = is.st_uid;
	if (is.st_gid > 0x10000)
		throw -LINUX_EOVERFLOW;
	ls.st_gid = is.st_gid;
	if (is.st_rdev > 0x10000)
		throw -LINUX_EOVERFLOW;
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
	return 0;
}

SYSCALL(sys32_stat64)
{
	const char* filename = (const char*) arg.a0.p;
	struct linux_stat64& ls = *(struct linux_stat64*) arg.a1.p;

	//log("stat64ing <%p %s> to %p", filename, filename, &ls);
	struct stat is;
	VFS::Stat(filename, is);
	Convert(is, ls);
	return 0;
}

SYSCALL(sys32_lstat64)
{
	const char* filename = (const char*) arg.a0.p;
	struct linux_stat64& ls = *(struct linux_stat64*) arg.a1.p;

	struct stat is;
	VFS::Lstat(filename, is);
	Convert(is, ls);
	return 0;
}
