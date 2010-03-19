/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "syscalls.h"
#include "filesystem/file.h"
#include <sys/statvfs.h>

#pragma pack(push, 1)
struct linux_compat_statfs {
	u32	f_type;
	u32	f_bsize;
	u32	f_blocks;
	u32	f_bfree;
	u32	f_bavail;
	u32	f_files;
	u32	f_ffree;
	u64	f_fsid;
	u32	f_namelen;	/* SunOS ignores this field. */
	u32	f_frsize;
	u32	f_spare[5];
};

struct linux_compat_statfs64 {
	u32 f_type;
	u32 f_bsize;
	u64 f_blocks;
	u64 f_bfree;
	u64 f_bavail;
	u64 f_files;
	u64 f_ffree;
	u64 f_fsid;
	u32 f_namelen;
	u32 f_frsize;
	u32 f_spare[5];
};

#pragma pack(pop)

static void convert_statvfs(struct statvfs& is, struct linux_compat_statfs& ls)
{
	ls.f_type = 'LBW1';
	ls.f_bsize = is.f_bsize;
	ls.f_blocks = is.f_blocks;
	ls.f_bfree = is.f_bfree;
	ls.f_bavail = is.f_bavail;
	ls.f_files = is.f_files;
	ls.f_ffree = is.f_ffree;
	ls.f_fsid = is.f_fsid;
	ls.f_namelen = is.f_namemax;
	ls.f_frsize = is.f_frsize;
}

static void convert_statvfs(struct statvfs& is, struct linux_compat_statfs64& ls)
{
	ls.f_type = 'LBW1';
	ls.f_bsize = is.f_bsize;
	ls.f_blocks = is.f_blocks;
	ls.f_bfree = is.f_bfree;
	ls.f_bavail = is.f_bavail;
	ls.f_files = is.f_files;
	ls.f_ffree = is.f_ffree;
	ls.f_fsid = is.f_fsid;
	ls.f_namelen = is.f_namemax;
	ls.f_frsize = is.f_frsize;
}

SYSCALL(compat_sys_statfs)
{
	const char* path = (const char*) arg.a0.p;
	struct linux_compat_statfs& ls = *(struct linux_compat_statfs*) arg.a1.p;

	struct statvfs is;
	int result = statvfs(path, &is);
	if (result == -1)
		throw errno;

	convert_statvfs(is, ls);
	return 0;
}

SYSCALL(compat_sys_statfs64)
{
	const char* path = (const char*) arg.a0.p;
	size_t sz = arg.a1.u;
	struct linux_compat_statfs64& ls = *(struct linux_compat_statfs64*) arg.a2.p;

	if (sz != sizeof(struct linux_compat_statfs64))
		throw EINVAL;

	struct statvfs is;
	int result = statvfs(path, &is);
	if (result == -1)
		throw errno;

	convert_statvfs(is, ls);
	return 0;
}

