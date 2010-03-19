/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef FD_H
#define FD_H

#include <map>
#include "file.h"

using std::map;

class SocketFD;
class VFSNode;

class FD : public HasRefCount
{
public:
	static int New(FD* fdo, int newfd=-1);
	static void Delete(int fd);
	static int Dup(int fd, int newfd=-1);
	static Ref<FD> Get(int fd);
	static void Flush();
	static map<int, int> GetFDMap();
	static Ref<VFSNode> GetVFSNodeFor(int fd);

	static bool GetCloexec(int fd);
	static void SetCloexec(int fd, bool f);

public:
	FD();
	virtual ~FD();

	virtual int GetRealFD() const;

	virtual int ReadV(const struct iovec* iov, int iovcnt);
	virtual int WriteV(const struct iovec* iov, int iovcnt);
	virtual int Read(void* buffer, size_t size);
	virtual int Write(const void* buffer, size_t size);
	virtual int64_t Seek(int whence, int64_t offset);
	virtual void Truncate(int64_t length);

	virtual void Fstat(struct stat& ls);
	virtual void Fchmod(mode_t mode);

	virtual int Fcntl(int cmd, u_int32_t argument);
	virtual int Ioctl(int cmd, u_int32_t argument);
};

#endif
