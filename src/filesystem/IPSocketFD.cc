#include "globals.h"
#include "filesystem/IPSocketFD.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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
}

void IPSocketFD::Connect(const struct sockaddr* sa, int addrlen)
{
	assert(sa->sa_family == AF_INET);

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
}

void IPSocketFD::Bind(const struct sockaddr* sa, int addrlen)
{
#if 0
	log("bind:");
	DumpMemory(sa, addrlen);
#endif

	int fd = GetRealFD();
	int result = bind(fd, sa, addrlen);
	log("result=%d errno=%d", result, errno);
	if (result == -1)
		throw errno;
}
