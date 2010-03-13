/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "syscalls.h"
#include "filesystem/IPSocketFD.h"
#include "filesystem/UnixSocketFD.h"
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

#define LINUX_SOL_SOCKET      1
#define LINUX_SOL_IP          0
#define LINUX_SOL_TCP         6
#define LINUX_SOL_UDP         17

#define LINUX_SO_DEBUG        1
#define LINUX_SO_REUSEADDR    2
#define LINUX_SO_TYPE         3
#define LINUX_SO_ERROR        4
#define LINUX_SO_DONTROUTE    5
#define LINUX_SO_BROADCAST    6
#define LINUX_SO_SNDBUF       7
#define LINUX_SO_RCVBUF       8
#define LINUX_SO_SNDBUFFORCE  32
#define LINUX_SO_RCVBUFFORCE  33
#define LINUX_SO_KEEPALIVE    9
#define LINUX_SO_OOBINLINE    10
#define LINUX_SO_NO_CHECK     11
#define LINUX_SO_PRIORITY     12
#define LINUX_SO_LINGER       13
#define LINUX_SO_BSDCOMPAT    14
#define LINUX_SO_PASSCRED     16
#define LINUX_SO_PEERCRED     17
#define LINUX_SO_RCVLOWAT     18
#define LINUX_SO_SNDLOWAT     19
#define LINUX_SO_RCVTIMEO     20
#define LINUX_SO_SNDTIMEO     21

#define LINUX_IP_TOS          1
#define LINUX_IP_TTL          2
#define LINUX_IP_HDRINCL      3
#define LINUX_IP_OPTIONS      4
#define LINUX_IP_RECVOPTS     6

#define LINUX_TCP_NODELAY      1
#define LINUX_TCP_MAXSEG       2
#define LINUX_TCP_KEEPIDLE     4

#define LINUX_MSG_OOB             0x01 /* Process out-of-band data.  */
#define LINUX_MSG_PEEK            0x02 /* Peek at incoming messages.  */
#define LINUX_MSG_DONTROUTE       0x04 /* Don't use local routing.  */
#define LINUX_MSG_CTRUNC          0x08 /* Control data lost before delivery.  */
#define LINUX_MSG_PROXY           0x10 /* Supply or ask second address.  */
#define LINUX_MSG_TRUNC           0x20
#define LINUX_MSG_DONTWAIT        0x40 /* Nonblocking IO.  */
#define LINUX_MSG_EOR             0x80 /* End of record.  */
#define LINUX_MSG_WAITALL         0x100 /* Wait for a full request.  */
#define LINUX_MSG_FIN             0x200
#define LINUX_MSG_SYN             0x400
#define LINUX_MSG_CONFIRM         0x800 /* Confirm path validity.  */
#define LINUX_MSG_RST             0x1000
#define LINUX_MSG_ERRQUEUE        0x2000 /* Fetch message from error queue.  */
#define LINUX_MSG_NOSIGNAL        0x4000 /* Do not generate SIGPIPE.  */
#define LINUX_MSG_MORE            0x8000  /* Sender will send more.  */
#define LINUX_MSG_CMSG_CLOEXEC    0x40000000

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

	Ref<SocketFD> ref;
	if (protofamily == AF_INET)
		ref = new IPSocketFD();
	else if (protofamily == AF_UNIX)
		ref = new UnixSocketFD();
	else
		throw EPROTONOSUPPORT;

	ref->Open(protofamily, itype, protocol);
	int newfd = FD::New(ref);

	if (type & LINUX_SOCK_CLOEXEC)
		FD::SetCloexec(newfd, true);
	if (type & LINUX_SOCK_NONBLOCK)
		ref->Fcntl(LINUX_F_SETFL, LINUX_O_NONBLOCK);

	return newfd;
}

static int do_bind(int fd, const struct sockaddr* addr, int addrlen)
{
	Ref<FD> ref = FD::Get(fd);
	SocketFD* fdo = SocketFD::Cast(ref);
	fdo->Bind(addr, addrlen);
	return 0;
}

static int do_connect(int fd, const struct sockaddr* addr, int addrlen)
{
	Ref<FD> ref = FD::Get(fd);
	SocketFD* fdo = SocketFD::Cast(ref);
	fdo->Connect(addr, addrlen);
	return 0;
}

static int do_setsockopt(int fd, int level, int optname, const void* optval,
		int optlen)
{
//	log("setsockopt(%d, %d, %d, %p, %p)", fd, level, optname, optval, optlen);
	int ilevel, ioptname;
	convert_sockopt(level, optname, ilevel, ioptname);

	Ref<FD> ref = FD::Get(fd);
	SocketFD* fdo = SocketFD::Cast(ref);
	fdo->SetSockopt(ilevel, ioptname, optval, optlen);
	return 0;
}

static int do_getsockopt(int fd, int level, int optname, void* optval,
		int* optlen)
{
	int ilevel, ioptname;
	convert_sockopt(level, optname, ilevel, ioptname);

	Ref<FD> ref = FD::Get(fd);
	SocketFD* fdo = SocketFD::Cast(ref);
	fdo->GetSockopt(ilevel, ioptname, optval, optlen);
	return 0;
}

static int do_getsockname(int fd, struct sockaddr* sa, int* namelen)
{
	Ref<FD> ref = FD::Get(fd);
	SocketFD* fdo = SocketFD::Cast(ref);
	fdo->GetSockname(sa, namelen);
	return 0;
}

static int do_send(int fd, const void *msg, size_t len, int flags)
{
	int iflags = convert_msg_flags(flags);

	Ref<FD> ref = FD::Get(fd);
	SocketFD* fdo = SocketFD::Cast(ref);
	return fdo->Send(msg, len, iflags);
}

static int do_recv(int fd, void *msg, size_t len, int flags)
{
	int iflags = convert_msg_flags(flags);

	Ref<FD> ref = FD::Get(fd);
	SocketFD* fdo = SocketFD::Cast(ref);
	return fdo->Recv(msg, len, iflags);
}

static int do_recvfrom(int fd, void *buf, size_t len, int flags,
		struct sockaddr *from, int *fromlen)
{
	int iflags = convert_msg_flags(flags);

	Ref<FD> ref = FD::Get(fd);
	SocketFD* fdo = SocketFD::Cast(ref);
	return fdo->RecvFrom(buf, len, iflags, from, fromlen);
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

		case LINUX_SYS_RECVFROM:
			return do_recvfrom(args[0], (void*) args[1], args[2],
					args[3], (struct sockaddr*) args[4], (int*) args[5]);

//		case LINUX_SYS_RECVMSG:
//			return do_recvmsg(
		default:
			error("unimplemented compat_sys_socketcall opcode %d", call);
	}

	throw EINVAL;
}
