/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "syscalls.h"
#include "filesystem/FD.h"
#include "filesystem/RawFD.h"
#include "filesystem/DirFD.h"
#include "filesystem/VFS.h"
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <dirent.h>
#include <poll.h>
#include <map>

#define LINUX_POLLIN          0x001           /* There is data to read.  */
#define LINUX_POLLPRI         0x002           /* There is urgent data to read.  */
#define LINUX_POLLOUT         0x004           /* Writing now will not block.  */
#define LINUX_POLLRDNORM      0x040           /* Normal data may be read.  */
#define LINUX_POLLRDBAND      0x080           /* Priority data may be read.  */
#define LINUX_POLLWRNORM      0x100           /* Writing now will not block.  */
#define LINUX_POLLWRBAND      0x200           /* Priority data may be written.  */
#define LINUX_POLLMSG         0x400
#define LINUX_POLLREMOVE      0x1000
#define LINUX_POLLRDHUP       0x2000
#define LINUX_POLLERR         0x008           /* Error condition.  */
#define LINUX_POLLHUP         0x010           /* Hung up.  */
#define LINUX_POLLNVAL        0x020           /* Invalid polling request.  */

static int do_open(VFSNode* node, const char* filename, int flags, int mode)
{
	//log("open(%s)", filename);

	bool nofollow = flags & LINUX_O_NOFOLLOW;

	Ref<FD> ref;
	if (flags & LINUX_O_DIRECTORY)
		ref = VFS::OpenDirectory(NULL, filename, nofollow);
	else
	{
		int iflags = FileFlagsL2I(flags);
		ref = VFS::OpenFile(NULL, filename, iflags, mode, nofollow);
	}

	int fd = FD::New(ref);

	FD::SetCloexec(fd, !!(flags & LINUX_O_CLOEXEC));

	return fd;
}

SYSCALL(compat_sys_open)
{
	const char* filename = (const char*) arg.a0.p;
	int flags = arg.a1.s;
	int mode = arg.a2.s;

	return do_open(NULL, filename, flags, mode);
}

SYSCALL(compat_sys_openat)
{
	int dirfd = arg.a0.s;
	const char* filename = (const char*) arg.a1.p;
	int flags = arg.a2.s;
	int mode = arg.a3.s;

	Ref<VFSNode> node = FD::GetVFSNodeFor(dirfd);
	return do_open(node, filename, flags, mode);
}

SYSCALL(sys_close)
{
	int fd = arg.a0.s;
	FD::Delete(fd);
	return 0;
}

SYSCALL(sys_pipe)
{
	int* fds = (int*) arg.a0.p;

	Ref<RawFD> ref1 = new RawFD();
	Ref<RawFD> ref2 = new RawFD();

	int ifd[2];
	int i = pipe(ifd);
	if (i == -1)
		throw errno;

	ref1->Open(ifd[0]);
	ref2->Open(ifd[1]);
	fds[0] = FD::New(ref1);
	fds[1] = FD::New(ref2);

	return 0;
}

SYSCALL(sys_dup)
{
	int fd = arg.a0.s;

	return FD::Dup(fd);
}

SYSCALL(sys_dup2)
{
	int oldfd = arg.a0.s;
	int newfd = arg.a1.s;

	return FD::Dup(oldfd, newfd);
}

SYSCALL(sys_read)
{
	int fd = arg.a0.s;
	void* buf = arg.a1.p;
	size_t nbytes = arg.a2.s;

	Ref<FD> fdo = FD::Get(fd);
	return fdo->Read(buf, nbytes);
}

SYSCALL(sys_write)
{
	int fd = arg.a0.s;
	void* buf = arg.a1.p;
	size_t nbytes = arg.a2.s;

	Ref<FD> fdo = FD::Get(fd);
	return fdo->Write(buf, nbytes);
}

/* struct iovec and struct linux_compat_iovec are equivalent */
SYSCALL(compat_sys_writev)
{
	int fd = arg.a0.s;
	const struct iovec* vec = (const struct iovec*) arg.a1.p;
	unsigned int vlen = arg.a2.u;

	Ref<FD> fdo = FD::Get(fd);
	return fdo->WriteV(vec, vlen);
}

SYSCALL(sys32_fstat64)
{
	int fd = arg.a0.s;
	struct linux_stat64& ls = *(struct linux_stat64*) arg.a1.p;

	Ref<FD> fdo = FD::Get(fd);

	struct stat is;
	fdo->Fstat(is);

	Convert(is, ls);
	return 0;
}

SYSCALL(compat_sys_fcntl64)
{
	int fd = arg.a0.s;
	int cmd = arg.a1.s;
	u_int32_t argument = arg.a2.u;

	switch (cmd)
	{
		case LINUX_F_DUPFD:
			return FD::Dup(fd);

		case LINUX_F_GETFD:
			return FD::GetCloexec(fd) ? LINUX_FD_CLOEXEC : 0;

		case LINUX_F_SETFD:
			FD::SetCloexec(fd, argument & LINUX_FD_CLOEXEC);
			return 0;
	}

	Ref<FD> fdo = FD::Get(fd);
	return fdo->Fcntl(cmd, argument);
}

SYSCALL(compat_sys_ioctl)
{
	int fd = arg.a0.s;
	int cmd = arg.a1.s;
	u_int32_t argument = arg.a2.u;

	Ref<FD> fdo = FD::Get(fd);
	return fdo->Ioctl(cmd, argument);
}

/* whence values are compatible. */
SYSCALL(sys32_lseek)
{
	unsigned int fd = arg.a0.u;
	int32_t offset = arg.a1.s;
	unsigned int whence = arg.a2.u;

	Ref<FD> fdo = FD::Get(fd);
	return fdo->Seek(offset, whence);
}

/* whence values are compatible. */
SYSCALL(sys_llseek)
{
	unsigned int fd = arg.a0.u;
	int64_t offset = (((int64_t) arg.a1.u)<<32) | arg.a2.u;
	int64_t* result = (int64_t*) arg.a3.p;
	unsigned int whence = arg.a4.u;

	Ref<FD> fdo = FD::Get(fd);
	*result = fdo->Seek(offset, whence);
	return 0;
}

SYSCALL(sys_fsync)
{
	int fd = arg.a0.u;

	Ref<FD> fdo = FD::Get(fd);
	fdo->Fsync();
	return 0;
}

SYSCALL(sys_ftruncate)
{
	int fd = arg.a0.u;
	u32 length = arg.a1.u;

	Ref<FD> fdo = FD::Get(fd);
	fdo->Truncate(length);
	return 0;
}

SYSCALL(sys_fchmod)
{
	int fd = arg.a0.u;
	mode_t mode = arg.a1.u;

	Ref<FD> fdo = FD::Get(fd);
	fdo->Fchmod(mode);
	return 0;
}

SYSCALL(sys_fchown)
{
	int fd = arg.a0.u;
	uid_t owner = arg.a1.s;
	gid_t group = arg.a2.s;

	Ref<FD> fdo = FD::Get(fd);
	fdo->Fchown(owner, group);
	return 0;
}

/* Lock operations are compatible. */
SYSCALL(sys_flock)
{
	int fd = arg.a0.s;
	int operation = arg.a1.s;

	if (operation & ~(LOCK_SH|LOCK_EX|LOCK_NB|LOCK_UN))
	{
		Warning("flock() GNU extensions not supported yet");
		throw EINVAL;
	}

	Ref<FD> fdo = FD::Get(fd);
	fdo->Flock(operation);
	return 0;
}

SYSCALL(compat_sys_getdents64)
{
	unsigned int fd = arg.a0.u;
	struct linux_dirent64* dirent = (struct linux_dirent64*) arg.a1.p;
	unsigned int count = arg.a2.u;

	Ref<FD> fdo = FD::Get(fd);

	DirFD* dirfdo = dynamic_cast<DirFD*>((FD*) fdo);
	if (!dirfdo)
		return -LINUX_ENOTDIR;

	return dirfdo->GetDents64(dirent, count);
}

SYSCALL(compat_sys_getdents)
{
	unsigned int fd = arg.a0.u;
	struct linux_dirent64* dirent = (struct linux_dirent64*) arg.a1.p;
	unsigned int count = arg.a2.u;

	Ref<FD> fdo = FD::Get(fd);

	DirFD* dirfdo = dynamic_cast<DirFD*>((FD*) fdo);
	if (!dirfdo)
		return -LINUX_ENOTDIR;

	return dirfdo->GetDents(dirent, count);
}
