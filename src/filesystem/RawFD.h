#ifndef RawFD_H
#define RawFD_H

#include "FileFD.h"

class RawFD : public FileFD
{
public:
	RawFD();
	~RawFD();

public:
	void Open(const string& filename, int flags, int mode=0);
	void Open(int realfd);

	int GetRealFD() const { return _realfd; }

	int ReadV(const struct iovec* iov, int iovcnt);
	int WriteV(const struct iovec* iov, int iovcnt);
	int Read(void* buffer, size_t size);
	int Write(const void* buffer, size_t size);
	int64_t Seek(int whence, int64_t offset);

	void Fstat(struct stat& st);

	int Fcntl(int cmd, u_int32_t argument);
	int Ioctl(int cmd, u_int32_t argument);

private:
	int _realfd;
};

#endif