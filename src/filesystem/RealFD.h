/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef RealFD_H
#define RealFD_H

#include "FD.h"
#include <deque>

using std::deque;

class RealFD : public FD
{
public:
	RealFD(int fd);
	RealFD(int fd, VFSNode* node);
	~RealFD();

public:
	/* Basic operations */

	int ReadV(const struct iovec* iov, int iovcnt);
	int WriteV(const struct iovec* iov, int iovcnt);
	int Read(void* buffer, size_t size);
	int Write(const void* buffer, size_t size);
	int64_t Seek(int whence, int64_t offset);
	void Truncate(int64_t length);
	void Fsync();
	void Flock(int operation);

	void Fstat(struct stat& st);
	void Fchmod(mode_t mode);
	void Fchown(uid_t owner, gid_t group);

	int Fcntl(int cmd, u_int32_t argument);
	int Ioctl(int cmd, u_int32_t argument);

	/* Socket operations */

	void Connect(const struct sockaddr* addr, int addrlen);
	void Bind(const struct sockaddr* addr, int addrlen);

	void SetSockopt(int level, int optname, const void* option,	int optlen);
	void GetSockopt(int level, int optname, void* option, int* optlen);
	void GetSockname(struct sockaddr* addr, int* namelen);

	int SendTo(const void *buf, size_t len, int flags,
			const struct sockaddr* to, int tolen);
	int RecvFrom(void *buf, size_t len, int flags,
			struct sockaddr* from, int* fromlen);
};

#endif
