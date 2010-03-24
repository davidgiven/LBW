/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef LINUX_SOCKET_H
#define LINUX_SOCKET_H

#define LINUX_SOCK_CLOEXEC   02000000
#define LINUX_SOCK_NONBLOCK  00004000

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

#pragma pack(push, 1)
struct linux_msghdr
{
	void* msg_name;	        /* Socket name			*/
	s32 msg_namelen;	    /* Length of name		*/
	struct iovec* msg_iov;	/* Data blocks			*/
	u32 msg_iovlen;	        /* Number of blocks		*/
	void* msg_control;	    /* Per protocol magic (eg BSD file descriptor passing) */
	u32 msg_controllen;	    /* Length of cmsg list */
	u32 msg_flags;
};
#pragma pack(pop)

#endif
