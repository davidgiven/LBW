#ifndef IPSOCKETFD_H
#define IPSOCKETFD_H

#include "SocketFD.h"
#include <sys/socket.h>
#include <sys/types.h>

class IPSocketFD : public SocketFD
{
public:
	static IPSocketFD* Cast(FD* fd);

public:
	virtual void Open(int family, int type, int protocol);

	virtual void Connect(const struct sockaddr* addr, int addrlen);
	virtual void Bind(const struct sockaddr* addr, int addrlen);
};

#endif