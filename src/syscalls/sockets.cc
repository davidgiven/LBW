/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "syscalls.h"
#include "filesystem/FD.h"
#include "filesystem/RealFD.h"
#include "filesystem/socket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <xti.h>

#define LINUX_SYS_SOCKET	1		/* sys_socket(2)		*/
#define LINUX_SYS_BIND	2		/* sys_bind(2)			*/
#define LINUX_SYS_CONNECT	3		/* sys_connect(2)		*/
#define LINUX_SYS_LISTEN	4		/* sys_listen(2)		*/
#define LINUX_SYS_ACCEPT	5		/* sys_accept(2)		*/
#define LINUX_SYS_GETSOCKNAME	6		/* sys_getsockname(2)		*/
#define LINUX_SYS_GETPEERNAME	7		/* sys_getpeername(2)		*/
#define LINUX_SYS_SOCKETPAIR	8		/* sys_socketpair(2)		*/
#define LINUX_SYS_SEND	9		/* sys_send(2)			*/
#define LINUX_SYS_RECV	10		/* sys_recv(2)			*/
#define LINUX_SYS_SENDTO	11		/* sys_sendto(2)		*/
#define LINUX_SYS_RECVFROM	12		/* sys_recvfrom(2)		*/
#define LINUX_SYS_SHUTDOWN	13		/* sys_shutdown(2)		*/
#define LINUX_SYS_SETSOCKOPT	14		/* sys_setsockopt(2)		*/
#define LINUX_SYS_GETSOCKOPT	15		/* sys_getsockopt(2)		*/
#define LINUX_SYS_SENDMSG	16		/* sys_sendmsg(2)		*/
#define LINUX_SYS_RECVMSG	17		/* sys_recvmsg(2)		*/
#define LINUX_SYS_ACCEPT4	18		/* sys_accept4(2)		*/

static void convert_sockopt(int level, int optname, int& ilevel, int& ioptname)
{
#define CONVERT(n) case LINUX_##n: ioptname = n; return
	switch (level)
	{
		case LINUX_SOL_SOCKET:
		{
			ilevel = SOL_SOCKET;
			switch (optname)
			{
				CONVERT(SO_DEBUG);
				CONVERT(SO_REUSEADDR);
				CONVERT(SO_TYPE);
				CONVERT(SO_ERROR);
				CONVERT(SO_DONTROUTE);
				CONVERT(SO_BROADCAST);
				CONVERT(SO_SNDBUF);
				CONVERT(SO_RCVBUF);
				CONVERT(SO_KEEPALIVE);
				CONVERT(SO_OOBINLINE);
				CONVERT(SO_LINGER);
				default:
					throw ENOPROTOOPT;
			}
		}

		case INET_IP:
		{
			ilevel = level; /* protocol number, universal */
			switch (optname)
			{
				CONVERT(IP_OPTIONS);
				CONVERT(IP_TOS);
				CONVERT(IP_TTL);
				//CONVERT(IP_REUSEADDR);
				//CONVERT(IP_DONTROUTE);
				//CONVERT(IP_BROADCAST);
				default:
					throw ENOPROTOOPT;
			}
		}

		case INET_TCP:
		{
			ilevel = level; /* protocol number, universal */
			switch (optname)
			{
				CONVERT(TCP_NODELAY);
				CONVERT(TCP_MAXSEG);
				case LINUX_TCP_KEEPIDLE: ioptname = TCP_KEEPALIVE; break; // FIXME
				default:
					throw ENOPROTOOPT;
			}
		}

		case INET_UDP:
		{
			ilevel = level; /* protocol number, universal */
			switch (optname)
			{
				//CONVERT(UDP_CHECKSUM);
				default:
					throw ENOPROTOOPT;
			}
		}

		default:
			throw ENOPROTOOPT;
	}
#undef CONVERT
}

static int convert_msg_flags(int lflags)
{
	int iflags = 0;
#define CONVERT(n) if (lflags & LINUX_##n) iflags |= n
	CONVERT(MSG_OOB);
	CONVERT(MSG_PEEK);
#if 0
	CONVERT(MSG_DONTROUTE);
	CONVERT(MSG_EOR);
	CONVERT(MSG_TRUNC);
	CONVERT(MSG_CTRUNC);
	CONVERT(MSG_WAITALL);
	CONVERT(MSG_DONTWAIT);
#endif
	return iflags;
}

static int do_socket(int protofamily, int type, int protocol)
{
	int itype = type & 0xff;

	int newfd;
	switch (protofamily)
	{
		case AF_INET:
		case AF_UNIX:
			newfd = socket(protofamily, type, protocol);
			break;

		default:
			throw EPROTONOSUPPORT;
	}

	Ref<FD> ref = new RealFD(newfd);
	if (type & LINUX_SOCK_CLOEXEC)
		ref->Fcntl(LINUX_F_SETFD, 1);
	if (type & LINUX_SOCK_NONBLOCK)
		ref->Fcntl(LINUX_F_SETFL, LINUX_O_NONBLOCK);

	return newfd;
}

static int do_bind(int fd, const struct sockaddr* addr, int addrlen)
{
	Ref<FD> ref = FD::Get(fd);
	ref->Bind(addr, addrlen);
	return 0;
}

static int do_connect(int fd, const struct sockaddr* addr, int addrlen)
{
	Ref<FD> ref = FD::Get(fd);
	ref->Connect(addr, addrlen);
	return 0;
}

static int do_setsockopt(int fd, int level, int optname, const void* optval,
		int optlen)
{
//	log("setsockopt(%d, %d, %d, %p, %p)", fd, level, optname, optval, optlen);
	int ilevel, ioptname;
	convert_sockopt(level, optname, ilevel, ioptname);

	Ref<FD> ref = FD::Get(fd);
	ref->SetSockopt(ilevel, ioptname, optval, optlen);
	return 0;
}

static int do_getsockopt(int fd, int level, int optname, void* optval,
		int* optlen)
{
	int ilevel, ioptname;
	convert_sockopt(level, optname, ilevel, ioptname);

	Ref<FD> ref = FD::Get(fd);
	ref->GetSockopt(ilevel, ioptname, optval, optlen);
	return 0;
}

static int do_getsockname(int fd, struct sockaddr* sa, int* namelen)
{
	Ref<FD> ref = FD::Get(fd);
	ref->GetSockname(sa, namelen);
	return 0;
}

static int do_send(int fd, const void *msg, size_t len, int flags)
{
	int iflags = convert_msg_flags(flags);

	Ref<FD> ref = FD::Get(fd);
	return ref->SendTo(msg, len, iflags, NULL, NULL);
}

static int do_recv(int fd, void *msg, size_t len, int flags)
{
	int iflags = convert_msg_flags(flags);

	Ref<FD> ref = FD::Get(fd);
	return ref->RecvFrom(msg, len, iflags, NULL, NULL);
}

static int do_sendto(int fd, const void* buf, size_t len, int flags,
		const struct sockaddr* to, int tolen)
{
	int iflags = convert_msg_flags(flags);

	Ref<FD> ref = FD::Get(fd);
	return ref->SendTo(buf, len, iflags, to, tolen);
}

static int do_recvfrom(int fd, void *buf, size_t len, int flags,
		struct sockaddr *from, int *fromlen)
{
	int iflags = convert_msg_flags(flags);

	Ref<FD> ref = FD::Get(fd);
	return ref->RecvFrom(buf, len, iflags, from, fromlen);
}

static ssize_t do_sendmsg(int fd, const struct linux_msghdr *msg, int flags)
{
	if (msg->msg_controllen > 0)
		throw EOPNOTSUPP;

	if (msg->msg_iovlen == 1)
	{
		return do_sendto(fd, msg->msg_iov->iov_base, msg->msg_iov->iov_len,
				flags,
				(const struct sockaddr*) msg->msg_name, msg->msg_namelen);
	}
	else
	{
		error("sendmsg() with more than one iov element not supported yet");
	}
}

static ssize_t do_recvmsg(int fd, struct linux_msghdr *msg, int flags)
{
	msg->msg_controllen = 0;

	if (msg->msg_iovlen == 1)
	{
		return do_recvfrom(fd, msg->msg_iov->iov_base, msg->msg_iov->iov_len,
				flags,
				(struct sockaddr*) msg->msg_name, &msg->msg_namelen);
	}
	else
	{
		error("sendmsg() with more than one iov element not supported yet");
	}
}

SYSCALL(compat_sys_socketcall)
{
	int call = arg.a0.s;
	u32* args = (u32*) arg.a1.p;

	switch (call)
	{
		case LINUX_SYS_SOCKET:
			return do_socket(args[0], args[1], args[2]);

		case LINUX_SYS_BIND:
			return do_bind(args[0], (const struct sockaddr*) args[1],
					args[2]);

		case LINUX_SYS_CONNECT:
			return do_connect(args[0], (const struct sockaddr*) args[1],
					args[2]);

		case LINUX_SYS_SETSOCKOPT:
			return do_setsockopt(args[0], args[1], args[2],
					(const void*) args[3], args[4]);

		case LINUX_SYS_GETSOCKOPT:
			return do_getsockopt(args[0], args[1], args[2],
					(void*) args[3], (int*) args[4]);

		case LINUX_SYS_GETSOCKNAME:
			return do_getsockname(args[0], (struct sockaddr*) args[1],
					(int*) args[2]);

		case LINUX_SYS_SEND:
			return do_send(args[0], (const void*) args[1], args[2],
					args[3]);

		case LINUX_SYS_RECV:
			return do_recv(args[0], (void*) args[1], args[2],
					args[3]);

		case LINUX_SYS_SENDTO:
			return do_sendto(args[0], (const void*) args[1], args[2],
					args[3], (const struct sockaddr*) args[4], args[5]);

		case LINUX_SYS_RECVFROM:
			return do_recvfrom(args[0], (void*) args[1], args[2],
					args[3], (struct sockaddr*) args[4], (int*) args[5]);

		case LINUX_SYS_SENDMSG:
			return do_sendmsg(args[0], (const struct linux_msghdr*) args[1], args[2]);

		case LINUX_SYS_RECVMSG:
			return do_recvmsg(args[0], (struct linux_msghdr*) args[1], args[2]);

		default:
			error("unimplemented compat_sys_socketcall opcode %d", call);
	}

	throw EINVAL;
}
