/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef FD_H
#define FD_H

#include <map>
#include <deque>
#include "file.h"

using std::map;
using std::deque;

class SocketFD;
class VFSNode;

class FD : public HasRefCount
{
public:
	static int CreateDummyFD();
	static Ref<FD> Get(int fd);
	static void Set(int fd, FD* fdo);
	static void Unset(int fd);
	static Ref<VFSNode> GetVFSNodeFor(int fd);

public:
	FD(int fd);
	FD(int fd, VFSNode* node);
	virtual ~FD();

	virtual void Close();
	virtual int Dup(int destfd=-1);

	int GetFD() const { return _fd; }
	Ref<VFSNode>& GetVFSNode() { return _node; }

	/* Basic operations */

	virtual int ReadV(const struct iovec* iov, int iovcnt) { throw EINVAL; }
	virtual int WriteV(const struct iovec* iov, int iovcnt) { throw EINVAL; }
	virtual int Read(void* buffer, size_t size) { throw EINVAL; }
	virtual int Write(const void* buffer, size_t size) { throw EINVAL; }
	virtual int64_t Seek(int whence, int64_t offset) { throw EINVAL; }
	virtual void Truncate(int64_t length) { throw EINVAL; }
	virtual void Fsync() { }
	virtual void Flock(int operation) { throw EINVAL; }

	virtual void Fstat(struct stat& ls) { throw EINVAL; }
	virtual void Fchmod(mode_t mode) { throw EINVAL; }
	virtual void Fchown(uid_t owner, gid_t group) { throw EINVAL; }

	virtual int Fcntl(int cmd, u_int32_t argument);
	virtual int Ioctl(int cmd, u_int32_t argument);

	/* Directory operations */

	virtual int GetDents(void* buffer, size_t count);
	virtual int GetDents64(void* buffer, size_t count);

	/* Socket operations */

	virtual void Connect(const struct sockaddr* addr, int addrlen) { throw EINVAL; }
	virtual void Bind(const struct sockaddr* addr, int addrlen) { throw EINVAL; }

	virtual void SetSockopt(int level, int optname, const void* option,
			int optlen) { throw EINVAL; }
	virtual void GetSockopt(int level, int optname, void* option,
			int* optlen) { throw EINVAL; }
	virtual void GetSockname(struct sockaddr* addr, int* namelen) { throw EINVAL; }
	virtual void GetPeername(struct sockaddr* addr, int* namelen) { throw EINVAL; }

	virtual int SendTo(const void *buf, size_t len, int flags,
			const struct sockaddr* to, int tolen) { throw EINVAL; }
	virtual int RecvFrom(void *buf, size_t len, int flags,
			struct sockaddr* from, int* fromlen) { throw EINVAL; }


private:
	class DirData;
	DirData& get_dirdata();

private:
	struct DirData
	{
		unsigned int pos;
		deque<string> contents;
	};

	int _fd;
	Ref<VFSNode> _node;
	DirData* _dirdata;
};

#endif
