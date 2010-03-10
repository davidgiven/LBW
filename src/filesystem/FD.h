#ifndef FD_H
#define FD_H

#include "file.h"

class SocketFD;

class FD : public HasRefCount
{
public:
	static int New(FD* fdo);
	static void Delete(int fd);
	static int Dup(int fd, int newfd=-1);
	static Ref<FD> Get(int fd);
	static void Flush();

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

	virtual void Fstat(struct stat& ls);

	virtual int Fcntl(int cmd, u_int32_t argument);
	virtual int Ioctl(int cmd, u_int32_t argument);
};

#endif
