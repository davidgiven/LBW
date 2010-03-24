/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "RealFD.h"
#include "filesystem/file.h"
#include "filesystem/VFS.h"
#include "filesystem/VFSNode.h"
#include "filesystem/InterixVFSNode.h"
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

RealFD::RealFD(int fd):
	FD(fd)
{
}

RealFD::RealFD(int fd, VFSNode* node):
	FD(fd, node)
{
}

RealFD::~RealFD()
{
}

int RealFD::ReadV(const struct iovec* iov, int iovcnt)
{
	int fd = GetFD();
	int result = readv(fd, iov, iovcnt);
	if (result == -1)
		throw errno;
	return result;
}

int RealFD::Read(void* buffer, size_t size)
{
	int fd = GetFD();
	int result = read(fd, buffer, size);
	if (result == -1)
		throw errno;
	return result;
}

int RealFD::Write(const void* buffer, size_t size)
{
	int fd = GetFD();
	int result = write(fd, buffer, size);
	if (result == -1)
		throw errno;
	return result;
}

int RealFD::WriteV(const struct iovec* iov, int iovcnt)
{
	int fd = GetFD();
	int result = writev(fd, iov, iovcnt);
	if (result == -1)
		throw errno;
	return result;
}

int64_t RealFD::Seek(int whence, int64_t offset)
{
	int fd = GetFD();
	off_t i = lseek(fd, whence, offset);
	if (i == -1)
		throw errno;
	return i;
}

void RealFD::Truncate(int64_t offset)
{
	if (offset > 0xffffffffLL)
		throw EFBIG;

	int fd = GetFD();
	int i = ftruncate(fd, offset);
	if (i == -1)
		throw errno;
}

void RealFD::Fsync()
{
	int fd = GetFD();
	int i = fsync(fd);
	if (i == -1)
		throw errno;
}

void RealFD::Flock(int operation)
{
	int fd = GetFD();
	int i = flock(fd, operation);
	if (i == -1)
		throw errno;
}

void RealFD::Fstat(struct stat& st)
{
	int fd = GetFD();
	int result = fstat(fd, &st);
	if (result == -1)
		throw errno;
}

void RealFD::Fchmod(mode_t mode)
{
	int fd = GetFD();
	int result = fchmod(fd, mode);
	if (result == -1)
		throw errno;
}

void RealFD::Fchown(uid_t owner, gid_t group)
{
	if (Options.FakeRoot && (owner == 0))
		return;

	int fd = GetFD();
	int result = fchown(fd, owner, group);
	if (result == -1)
		throw errno;
}

void RealFD::Connect(const struct sockaddr* sa, int addrlen)
{
	int fd = GetFD();

	switch (sa->sa_family)
	{
		case 0:
		case AF_INET:
		{
			int i = connect(fd, sa, sizeof(struct sockaddr_in));
			if (i == -1)
				throw errno;
			break;
		}

		case AF_UNIX:
		{
			/* This is a Unix domain socket, which needs to be created
			 * in the VFS.
			 */

			const struct sockaddr_un* sun = (const struct sockaddr_un*) sa;

			RAIILock locked;
			Ref<VFSNode> node;
			string leaf;
			VFS::Resolve(NULL, sun->sun_path, node, leaf, false);

			InterixVFSNode* inode = dynamic_cast<InterixVFSNode*>((VFSNode*)node);
			if (!inode)
				throw EINVAL;

			int i = fchdir(inode->GetRealFD());
			if (i == -1)
				throw errno;

			struct sockaddr_un childsun;
			childsun.sun_family = AF_UNIX;
			strcpy(childsun.sun_path, leaf.c_str());

			i = connect(fd, (const struct sockaddr*) &childsun,
					sizeof(childsun));
			if (i == -1)
				throw errno;
			break;
		}

		default:
			throw EAFNOSUPPORT;
	}
}

void RealFD::Bind(const struct sockaddr* sa, int addrlen)
{
	int fd = GetFD();

	switch (sa->sa_family)
	{
		case AF_INET:
		{
			int result = bind(fd, sa, addrlen);
			//log("result=%d errno=%d", result, errno);
			if (result == -1)
				throw errno;
			break;
		}

		default:
			throw EAFNOSUPPORT;
	}
}

void RealFD::SetSockopt(int level, int optname, const void* option,
		int optlen)
{
	int fd = GetFD();
	int i = setsockopt(fd, level, optname, option, optlen);
	if (i == -1)
		throw errno;
}

void RealFD::GetSockopt(int level, int optname, void* option,
		int* optlen)
{
	int fd = GetFD();
	int i = getsockopt(fd, level, optname, option, optlen);
	if (i == -1)
		throw errno;
}

void RealFD::GetSockname(struct sockaddr* sa, int* namelen)
{
	int fd = GetFD();
	int result = getsockname(fd, sa, namelen);
	if (result == -1)
		throw errno;

	switch (sa->sa_family)
	{
		case AF_INET:
			break;

		default:
			assert(false);
	}
}

int RealFD::RecvFrom(void *buf, size_t len, int flags,
		struct sockaddr *from, int *fromlen)
{
	int fd = GetFD();
	int i = recvfrom(fd, buf, len, flags, from, fromlen);
	if (i == -1)
		throw errno;
	return i;
}

int RealFD::SendTo(const void *buf, size_t len, int flags,
		const struct sockaddr* to, int tolen)
{
	int fd = GetFD();
	int i = sendto(fd, buf, len, flags, to, tolen);
	if (i == -1)
		throw errno;
	return i;
}
