/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef SOCKETFD_H
#define SOCKETFD_H

#include "RawFD.h"
#include <sys/socket.h>
#include <sys/types.h>

#define LINUX_SOCK_CLOEXEC   02000000
#define LINUX_SOCK_NONBLOCK  00004000

class SocketFD : public RawFD
{
public:
	static SocketFD* Cast(FD* fd);

public:
	virtual void Open(int family, int type, int protocol);

	virtual void Connect(const struct sockaddr* addr, int addrlen);
	virtual void Bind(const struct sockaddr* addr, int addrlen);

	virtual void SetSockopt(int level, int optname, const void* option,
			int optlen);
	virtual int Send(const void* msg, size_t len, int flags);
	virtual int RecvFrom(void *buf, size_t len, int flags,
			struct sockaddr *from, int *fromlen);

};

#endif
