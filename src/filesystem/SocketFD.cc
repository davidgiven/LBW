#include "globals.h"
#include "filesystem/SocketFD.h"
#include "filesystem/InterixVFSNode.h"
#include "filesystem/VFS.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

SocketFD* SocketFD::Cast(FD* fd)
{
	SocketFD* s = dynamic_cast<SocketFD*>(fd);
	if (!s)
		throw ENOTSOCK;
	return s;
}

void SocketFD::Open(int family, int type, int protocol)
{
	//log("family=%d type=%d protocol=%d", family, type, protocol);
	int fd = socket(family, type, protocol);
	if (fd == -1)
		throw errno;

	RawFD::Open(fd);
}

void SocketFD::Connect(const struct sockaddr* sa, int addrlen)
{
	throw EINVAL;
}

void SocketFD::Bind(const struct sockaddr* sa, int addrlen)
{
	throw EINVAL;
}

void SocketFD::SetSockopt(int level, int optname, const void* option,
		int optlen)
{
	int fd = GetRealFD();
	//log("setsockopt(%d, %d, %d, %p, %d)", fd, level, optname, option, optlen);
	int i = setsockopt(fd, level, optname, option, optlen);
	if (i == -1)
		throw errno;
}

int SocketFD::Send(const void* msg, size_t len, int flags)
{
	int fd = GetRealFD();
	int i = send(fd, msg, len, flags);
	if (i == -1)
		throw errno;
	return i;
}

int SocketFD::RecvFrom(void *buf, size_t len, int flags,
		struct sockaddr *from, int *fromlen)
{
	int fd = GetRealFD();
	int i = recvfrom(fd, buf, len, flags, from, fromlen);
	if (i == -1)
		throw errno;
	return i;
}
