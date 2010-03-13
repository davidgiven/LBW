/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "filesystem/IPSocketFD.h"
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

IPSocketFD* IPSocketFD::Cast(FD* fd)
{
	IPSocketFD* s = dynamic_cast<IPSocketFD*>(fd);
	if (!s)
		throw ENOTSOCK;
	return s;
}

void IPSocketFD::Open(int family, int type, int protocol)
{
	assert(family == AF_INET);
	//log("family=%d type=%d protocol=%d", family, type, protocol);
	SocketFD::Open(family, type, protocol);
	//log("Created socket, realfd = %d", GetRealFD());
}

void IPSocketFD::Connect(const struct sockaddr* sa, int addrlen)
{
	//assert(sa->sa_family == AF_INET) || (sa->sa);

	if (addrlen < sizeof(struct sockaddr_in))
		throw EINVAL;

	const struct sockaddr_in* sin = (const struct sockaddr_in*) sa;

	int fd = GetRealFD();
#if 0
	int flags = fcntl(fd, F_GETFL, 0);
	log("connect fd flags are %o", flags);
#endif
	int result = connect(fd, sa, sizeof(*sin));
	if (result == -1)
		throw errno;

#if 0
	fd_set reads, writes, excepts;

	FD_ZERO(&reads); FD_SET(fd, &reads);
	FD_ZERO(&writes); FD_SET(fd, &writes);
	FD_ZERO(&excepts); FD_SET(fd, &excepts);
	select(fd+1, &reads, &writes, &excepts, NULL);
	log("fd %d state %c%c%c", fd,
			FD_ISSET(fd, &reads) ? 'r' : '.',
			FD_ISSET(fd, &writes) ? 'w' : '.',
			FD_ISSET(fd, &excepts) ? 'x' : '.');
#endif
}

void IPSocketFD::Bind(const struct sockaddr* sa, int addrlen)
{
#if 0
	log("bind:");
	DumpMemory(sa, addrlen);
#endif

	int fd = GetRealFD();
	int result = bind(fd, sa, addrlen);
	//log("result=%d errno=%d", result, errno);
	if (result == -1)
		throw errno;
}

void IPSocketFD::GetSockname(struct sockaddr* sa, int* namelen)
{
	int fd = GetRealFD();
	int result = getsockname(fd, sa, namelen);
	if (result == -1)
		throw errno;
}

