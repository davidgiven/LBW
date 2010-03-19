/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "FD.h"
#include "filesystem/VFS.h"
#include "filesystem/VFSNode.h"
#include <map>
#include <list>

// #define LOG_REFCOUNTING

using std::map;
using std::list;

typedef map<int, Ref<FD> > FDS;
static FDS fds;
static map<int, bool> cloexecs;

/* --- FD management ----------------------------------------------------- */

static int find_free()
{
	int fd = 0;
	for (;;)
	{
		FDS::const_iterator i = fds.find(fd);
		if (i == fds.end())
			return fd;
		fd++;
	}
}

int FD::New(FD* fdo, int fd)
{
	RAIILock locked;

	if (fd == -1)
		fd = find_free();

	fds[fd] = fdo;
	cloexecs[fd] = false;

#if 0
	try
	{
		int realfd = fdo->GetRealFD();
		log("mapped realfd %d -> linuxfd %d", realfd, fd);
	}
	catch (...)
	{
	}
#endif

	return fd;
}

void FD::Delete(int fd)
{
	RAIILock locked;

	FDS::iterator i = fds.find(fd);
	if (i == fds.end())
		throw EBADF;

	fds.erase(i);
}

int FD::Dup(int fd, int newfd)
{
	RAIILock locked;

	::Ref<FD> ref = FD::Get(fd);

	if (newfd == -1)
		newfd = find_free();

	fds[newfd] = ref;
	cloexecs[newfd] = cloexecs[fd];
	//log("dup linuxfd %d -> %d", fd, newfd);

	return newfd;
}

Ref<FD> FD::Get(int fd)
{
	FDS::iterator i = fds.find(fd);
	if (i == fds.end())
		throw EBADF;
	return i->second;
}

bool FD::GetCloexec(int fd)
{
	RAIILock locked;

	FDS::const_iterator i = fds.find(fd);
	if (i == fds.end())
		throw EBADF;
	return cloexecs[fd];
}


void FD::SetCloexec(int fd, bool f)
{
	RAIILock locked;

	FDS::const_iterator i = fds.find(fd);
	if (i == fds.end())
		throw EBADF;
	cloexecs[fd] = f;
}

void FD::Flush()
{
	RAIILock locked;

	FDS::iterator i = fds.begin();
	while (i != fds.end())
	{
		int fd = i->first;
		if (cloexecs[fd])
		{
			//log("closing fd %d", fd);
			fds.erase(i++);
		}
		else
		{
			//log("not closing fd %d", fd);
			i++;
		}
	}
}

map<int, int> FD::GetFDMap()
{
	map<int, int> fdmap;

	FDS::iterator i = fds.begin();
	while (i != fds.end())
	{
		try
		{
			int realfd = i->second->GetRealFD();
			fdmap[i->first] = realfd;
		}
		catch (int e)
		{
		}

		i++;
	}

	return fdmap;
}

Ref<VFSNode> FD::GetVFSNodeFor(int linuxfd)
{
	if (linuxfd == LINUX_AT_FDCWD)
		return VFS::GetCWDNode();

	Ref<FD> reffd = FD::Get(linuxfd);
	DirFD* dirfd = DirFD::Cast(reffd);
	return dirfd->GetVFSNode();
}

/* --- General FD management --------------------------------------------- */

FD::FD()
{
}

FD::~FD()
{
}

/* --- Default methods --------------------------------------------------- */

int FD::GetRealFD() const
{
	throw EINVAL;
}

int FD::ReadV(const struct iovec* iov, int iovcnt)
{
	throw EINVAL;
}

int FD::WriteV(const struct iovec* iov, int iovcnt)
{
	throw EINVAL;
}

int FD::Read(void* buffer, size_t size)
{
	throw EINVAL;
}

int FD::Write(const void* buffer, size_t size)
{
	throw EINVAL;
}

int64_t FD::Seek(int whence, int64_t offset)
{
	throw EINVAL;
}

void FD::Truncate(int64_t offset)
{
	throw EINVAL;
}

void FD::Fstat(struct stat& ls)
{
	throw EINVAL;
}

void FD::Fchmod(mode_t mode)
{
	throw EINVAL;
}

int FD::Fcntl(int cmd, u_int32_t argument)
{
	error("unsupported fcntl %08x", cmd);
}

int FD::Ioctl(int cmd, u_int32_t argument)
{
	error("unsupported ioctl %08x", cmd);
}
