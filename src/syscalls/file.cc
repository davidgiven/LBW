/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "syscalls.h"
#include "filesystem/RawFD.h"
#include "filesystem/DirFD.h"
#include "filesystem/VFS.h"
#include <sys/uio.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>
#include <map>

static void copystring1(const string& s, char* buffer, size_t len)
{
	size_t slen = s.size() + 1;
	if (len < slen)
		throw ERANGE;

	memcpy(buffer, s.c_str(), slen);
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
	return VFS::Access(path, mode);
}

SYSCALL(sys_readlink)
{
	const char* path = (const char*) arg.a0.p;
	char* buffer = (char*) arg.a1.p;
	size_t len = arg.a2.u;

	string target = VFS::ReadLink(path);
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

	VFS::Chmod(path, mode);
	return 0;
}

SYSCALL(sys_get_cwd)
{
	char* buffer = (char*) arg.a0.p;
	size_t len = arg.a1.u;

	if (len == 0)
		throw EINVAL;

	string cwd = VFS::GetCWD();
	copystring1(cwd, buffer, len);
	return 0;
}

SYSCALL(sys_chdir)
{
	const char* buffer = (const char*) arg.a0.p;

	VFS::SetCWD(buffer);
	return 0;
}

SYSCALL(sys_mkdir)
{
	const char* path = (const char*) arg.a0.p;
	int mode = arg.a1.s;

	VFS::MkDir(path, mode);
	return 0;
}

SYSCALL(sys_rmdir)
{
	const char* path = (const char*) arg.a0.p;

	VFS::RmDir(path);
	return 0;
}

SYSCALL(sys_rename)
{
	const char* from = (const char*) arg.a0.p;
	const char* to = (const char*) arg.a1.p;

	VFS::Rename(from, to);
	return 0;
}

SYSCALL(sys_link)
{
	const char* from = (const char*) arg.a0.p;
	const char* to = (const char*) arg.a1.p;

	VFS::Link(from, to);
	return 0;
}

SYSCALL(sys_unlink)
{
	const char* path = (const char*) arg.a0.p;

	VFS::Unlink(path);
	return 0;
}

SYSCALL(sys_symlink)
{
	const char* path1 = (const char*) arg.a0.p;
	const char* path2 = (const char*) arg.a1.p;

	VFS::Symlink(path1, path2);
	return 0;
}
