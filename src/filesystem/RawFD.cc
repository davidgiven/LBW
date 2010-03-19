/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "RawFD.h"
#include "file.h"
#include <sys/uio.h>

RawFD::RawFD():
	_realfd(-1)
{
}

RawFD::~RawFD()
{
//	log("closing real fd %d", _realfd);
	if (_realfd != -1)
		close(_realfd);
}

void RawFD::Open(const string& filename, int flags, int mode)
{
	_realfd = open(filename.c_str(), flags, mode);
	//log("open (%s) -> realfd %d", filename.c_str(), _realfd);
	if (_realfd == -1)
		throw errno;
}

void RawFD::Open(int rawfd)
{
	_realfd = rawfd;
}

int RawFD::ReadV(const struct iovec* iov, int iovcnt)
{
	int result = readv(_realfd, iov, iovcnt);
	if (result == -1)
		throw errno;
	return result;
}

int RawFD::Read(void* buffer, size_t size)
{
	int result = read(_realfd, buffer, size);
	if (result == -1)
		throw errno;
	return result;
}

int RawFD::Write(const void* buffer, size_t size)
{
	int result = write(_realfd, buffer, size);
	if (result == -1)
		throw errno;
	return result;
}

int RawFD::WriteV(const struct iovec* iov, int iovcnt)
{
	int result = writev(_realfd, iov, iovcnt);
	if (result == -1)
		throw errno;
	return result;
}

int64_t RawFD::Seek(int whence, int64_t offset)
{
	off_t i = lseek(_realfd, whence, offset);
	if (i == -1)
		throw errno;
	return i;
}

void RawFD::Truncate(int64_t offset)
{
	int i = ftruncate(_realfd, offset);
	if (i == -1)
		throw errno;
}

void RawFD::Fsync()
{
	int i = fsync(_realfd);
	if (i == -1)
		throw errno;
}

void RawFD::Flock(int operation)
{
	int i = flock(_realfd, operation);
	if (i == -1)
		throw errno;
}

void RawFD::Fstat(struct stat& st)
{
	int result = fstat(_realfd, &st);
	if (result == -1)
		throw errno;
}

void RawFD::Fchmod(mode_t mode)
{
	int result = fchmod(_realfd, mode);
	if (result == -1)
		throw errno;
}

void RawFD::Fchown(uid_t owner, gid_t group)
{
	int result = fchown(_realfd, owner, group);
	if (result == -1)
		throw errno;
}
