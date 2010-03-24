/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "syscalls.h"
#include "filesystem/FD.h"
#include "filesystem/VFS.h"
#include <sys/uio.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>
#include <utime.h>
#include <map>

static int copystring1(const string& s, char* buffer, size_t len)
{
	size_t slen = s.size() + 1;
	if (len < slen)
		throw ERANGE;

	memcpy(buffer, s.c_str(), slen);
	return slen;
}

static int copystring2(const string& s, char* buffer, size_t len)
{
	size_t slen = s.size() + 1;
	if (slen > len)
		slen = len;

	memcpy(buffer, s.c_str(), slen);
	return slen;
}

SYSCALL(sys_sync)
{
	/* FIXME: this is a nop, as Interix can't do this. We could, however,
	 * run down the list of open file descriptors and fsync() each one.
	 */
	return 0;
}

SYSCALL(sys_umount)
{
	return -LINUX_ENOSYS;
}

SYSCALL(sys_access)
{
	const char* path = (const char*) arg.a0.p;
	int mode = arg.a1.s;

	/* F_OK, R_OK, W_OK, X_OK are standard. */
	return VFS::Access(NULL, path, mode);
}

SYSCALL(sys_readlink)
{
	const char* path = (const char*) arg.a0.p;
	char* buffer = (char*) arg.a1.p;
	size_t len = arg.a2.u;

	string target = VFS::ReadLink(NULL, path);
	return copystring2(target, buffer, len);
}

SYSCALL(sys_umask)
{
	mode_t mask = arg.a0.u;
	return umask(mask);
}

SYSCALL(sys_chmod)
{
	const char* path = (const char*) arg.a0.p;
	unsigned int mode = arg.a1.u;

	VFS::Chmod(NULL, path, mode);
	return 0;
}

SYSCALL(sys_chown)
{
	const char* path = (const char*) arg.a0.p;
	uid_t owner = arg.a1.u;
	gid_t group = arg.a2.u;

	VFS::Chown(NULL, path, owner, group);
	return 0;
}

SYSCALL(sys_lchown)
{
	const char* path = (const char*) arg.a0.p;
	uid_t owner = arg.a1.u;
	gid_t group = arg.a2.u;

	VFS::Lchown(NULL, path, owner, group);
	return 0;
}

SYSCALL(sys_fchownat)
{
	int dirfd = arg.a0.s;
	const char* path = (const char*) arg.a1.p;
	uid_t owner = arg.a2.u;
	gid_t group = arg.a3.u;
	int flags = arg.a4.s;

	Ref<VFSNode> node = FD::GetVFSNodeFor(dirfd);
	if (flags & LINUX_AT_SYMLINK_NOFOLLOW)
		VFS::Lchown(node, path, owner, group);
	else
		VFS::Chown(node, path, owner, group);
	return 0;
}

SYSCALL(sys_get_cwd)
{
	char* buffer = (char*) arg.a0.p;
	size_t len = arg.a1.u;

	if (len == 0)
		throw EINVAL;

	string cwd = VFS::GetCWD();
	return copystring1(cwd, buffer, len);
}

SYSCALL(sys_chdir)
{
	const char* path = (const char*) arg.a0.p;

	VFS::SetCWD(NULL, path);
	return 0;
}

SYSCALL(sys_fchdir)
{
	int dirfd = arg.a0.s;

	Ref<VFSNode> node = FD::GetVFSNodeFor(dirfd);
	VFS::SetCWD(node, ".");
	return 0;
}

SYSCALL(sys_mkdir)
{
	const char* path = (const char*) arg.a0.p;
	int mode = arg.a1.s;

	VFS::MkDir(NULL, path, mode);
	return 0;
}

SYSCALL(sys_rmdir)
{
	const char* path = (const char*) arg.a0.p;

	VFS::RmDir(NULL, path);
	return 0;
}

SYSCALL(sys_rename)
{
	const char* from = (const char*) arg.a0.p;
	const char* to = (const char*) arg.a1.p;

	VFS::Rename(NULL, from, to);
	return 0;
}

SYSCALL(sys_link)
{
	const char* target = (const char*) arg.a0.p;
	const char* path = (const char*) arg.a1.p;

	log("link target=<%s> path=<%s>", target, path);
	VFS::Link(NULL, target, path);
	return 0;
}

SYSCALL(sys_unlink)
{
	const char* path = (const char*) arg.a0.p;

	VFS::Unlink(NULL, path);
	return 0;
}

SYSCALL(sys_unlinkat)
{
	int dirfd = arg.a0.s;
	const char* path = (const char*) arg.a1.p;
	int flags = arg.a2.s;

	Ref<VFSNode> node = FD::GetVFSNodeFor(dirfd);
	if (flags & LINUX_AT_REMOVEDIR)
		VFS::RmDir(node, path);
	else
		VFS::Unlink(node, path);
	return 0;
}

SYSCALL(sys_symlink)
{
	const char* target = (const char*) arg.a0.p;
	const char* path = (const char*) arg.a1.p;

	log("symlink target=<%s> path=<%s>", target, path);
	VFS::Symlink(NULL, target, path);
	return 0;
}

/* compat_utimbuf is compatible with Interix. */
SYSCALL(compat_sys_utime)
{
	const char* path = (const char*) arg.a0.p;
	struct utimbuf& ub = *(struct utimbuf*) arg.a1.p;

	struct timeval times[2];
	times[0].tv_usec = times[1].tv_usec = 0;
	times[0].tv_sec = ub.actime;
	times[1].tv_sec = ub.modtime;

	VFS::Utimes(NULL, path, times);
	return 0;
}

/* struct timeval is compatible with Interix. */
SYSCALL(compat_sys_utimes)
{
	const char* path = (const char*) arg.a0.p;
	const struct timeval* times = (const struct timeval*) arg.a1.p;

	VFS::Utimes(NULL, path, times);
	return 0;
}

/* struct timeval is compatible with Interix. */
SYSCALL(compat_sys_futimesat)
{
	int dirfd = arg.a0.s;
	const char* path = (const char*) arg.a1.p;
	const struct timeval* times = (const struct timeval*) arg.a2.p;

	Ref<VFSNode> node = FD::GetVFSNodeFor(dirfd);
	VFS::Utimes(node, path, times);
	return 0;
}

static void copytimeval(const struct timespec& from, struct timeval& to,
		time_t fallback)
{
	if (from.tv_nsec == LINUX_UTIME_NOW)
		time(&to.tv_sec);
	else if (from.tv_nsec == LINUX_UTIME_OMIT)
		to.tv_sec = fallback;
	else
		to.tv_sec = from.tv_sec;

	to.tv_usec = 0;
}

/* compat_utimbuf is compatible with Interix. */
SYSCALL(compat_sys_utimensat)
{
	int dirfd = arg.a0.s;
	const char* path = (const char*) arg.a1.p;
	const struct timespec* times = (const struct timespec*) arg.a2.p;
	int flags = arg.a3.s;

	if (flags & LINUX_AT_SYMLINK_NOFOLLOW)
	{
		/* We can't update the time on symlinks in Interix. As hardly anyone
		 * cares, just pretend it worked.
		 */
		return 0;
	}

	Ref<VFSNode> node = FD::GetVFSNodeFor(dirfd);

	if (times)
	{
		struct timeval timescopy[2];
		struct stat st;

		if ((times[0].tv_nsec == LINUX_UTIME_OMIT) ||
			(times[1].tv_nsec == LINUX_UTIME_OMIT))
		{
			VFS::Stat(node, path, st);
		}

		copytimeval(times[0], timescopy[0], st.st_atime);
		copytimeval(times[1], timescopy[1], st.st_mtime);

		VFS::Utimes(node, path, timescopy);
	}
	else
		VFS::Utimes(node, path, NULL);

	return 0;
}

SYSCALL(sys_getxattr)
{
	const char* path = (const char*) arg.a0.p;
	const char* name = (const char*) arg.a1.p;
	void* value = arg.a2.p;
	size_t size = arg.a3.u;

	throw EOPNOTSUPP;
}

SYSCALL(sys_lgetxattr)
{
	const char* path = (const char*) arg.a0.p;
	const char* name = (const char*) arg.a1.p;
	void* value = arg.a2.p;
	size_t size = arg.a3.u;

	throw EOPNOTSUPP;
}
