/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "filesystem/FD.h"
#include "filesystem/RealFD.h"
#include "filesystem/VFS.h"
#include "filesystem/VFSNode.h"
#include "filesystem/file.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <map>
#include <list>

//#define VERBOSE

using std::map;
using std::list;

typedef map<int, Ref<FD> > FDS;
static FDS fds;

#if defined VERBOSE
#define LOG log
#else
#define LOG(...)
#endif

/* --- FD management ----------------------------------------------------- */

int FD::CreateDummyFD()
{
	/* It doesn't matter what fd we create; we only want it for the number. */

	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	CheckError(fd);
	fcntl(fd, F_SETFD, 1);
	return fd;
}

static Ref<FD> create_new_fdo(int fd)
{
	/* First check to see if this is a valid FD. */

	CheckError(fcntl(fd, F_GETFD, 0));

	/* For now we'll assume it's a RealFD. */

	Ref<FD> fdo = new RealFD(fd);
	FD::Set(fd, fdo);
	return fdo;
}

Ref<FD> FD::Get(int fd)
{
	RAIILock locked;

	//LOG("FD::Get(%d)", fd);

	FDS::iterator i = fds.find(fd);
	if (i == fds.end())
		return create_new_fdo(fd);
	return i->second;
}

void FD::Set(int fd, FD* fdo)
{
	RAIILock locked;

	LOG("FD::Set(%d)", fd);
	fds[fd] = fdo;
}

void FD::Unset(int fd)
{
	RAIILock locked;

	FDS::iterator i = fds.find(fd);
	if (i != fds.end())
		fds.erase(i);
}

Ref<VFSNode> FD::GetVFSNodeFor(int fd)
{
	if (fd == LINUX_AT_FDCWD)
		return VFS::GetCWDNode();

	Ref<FD> ref = FD::Get(fd);
	Ref<VFSNode> vfsnode = ref->GetVFSNode();
	if (!vfsnode)
		throw ENOTDIR;
	return vfsnode;
}

/* --- General FD management --------------------------------------------- */

FD::FD(int fd):
	_fd(fd),
	_dirdata(NULL)
{
	FD::Set(fd, this);
}

FD::FD(int fd, VFSNode* node):
	_fd(fd),
	_node(node),
	_dirdata(NULL)
{
	FD::Set(fd, this);
}

FD::~FD()
{
	delete _dirdata;
}

/* --- Default methods --------------------------------------------------- */

void FD::Close()
{
	LOG("%p: close(%d)", this, _fd);
	int i = close(_fd);
	CheckError(i);
	FD::Unset(_fd);
	_fd = -1;
}

int FD::Dup(int destfd)
{
	int fd = GetFD();

	int result;
	if (destfd == -1)
		result = destfd = dup(fd);
	else
		result = dup2(fd, destfd);

	CheckError(result);
	return result;
}

int FD::Fcntl(int cmd, u_int32_t argument)
{
	switch (cmd)
	{
		case LINUX_F_DUPFD:
			return Dup();
		{
			int i = dup(_fd);
			CheckError(i);
			return i;
		}

		case LINUX_F_GETFD:
		{
			int i = fcntl(_fd, F_GETFD, 0) ? LINUX_FD_CLOEXEC : 0;
			CheckError(i);
			return i;
		}

		case LINUX_F_SETFD:
		{
			int i = fcntl(_fd, F_SETFD, argument & LINUX_FD_CLOEXEC);
			CheckError(i);
			return 0;
		}
	}

	error("unsupported fcntl %08x", cmd);
}

int FD::Ioctl(int cmd, u_int32_t argument)
{
	error("unsupported ioctl %08x", cmd);
}

FD::DirData& FD::get_dirdata()
{
	if (_dirdata)
		return *_dirdata;

	_dirdata = new DirData();
	if (!_dirdata)
		throw ENOMEM;

	_dirdata->pos = 0;
	_dirdata->contents = GetVFSNode()->Enumerate();
	return *_dirdata;
}

static int get_file_type(const struct stat& st)
{
	if (S_ISDIR(st.st_mode))
		return LINUX_DT_DIR;
	if (S_ISCHR(st.st_mode))
		return LINUX_DT_CHR;
	if (S_ISBLK(st.st_mode))
		return LINUX_DT_BLK;
	if (S_ISREG(st.st_mode))
		return LINUX_DT_REG;
	if (S_ISFIFO(st.st_mode))
		return LINUX_DT_FIFO;
	if (S_ISSOCK(st.st_mode))
		return LINUX_DT_SOCK;
	if (S_ISLNK(st.st_mode))
		return LINUX_DT_LNK;
	return LINUX_DT_UNKNOWN;
}

int FD::GetDents(void* buffer, size_t count)
{
	RAIILock locked;
	Ref<VFSNode>& vfsnode = GetVFSNode();
	if (!vfsnode)
		throw ENOTDIR;

	u8* ptr = (u8*) buffer;
	DirData& dd = get_dirdata();
	unsigned int byteswritten = 0;
	for (;;)
	{
		if (dd.pos == dd.contents.size())
			break;

		try
		{
			const string& filename = dd.contents[dd.pos];

			struct stat st;
			vfsnode->StatFile(filename, st);

			size_t reclen = sizeof(struct compat_linux_dirent) +
					filename.size() + 1;
			reclen = (reclen + 3) & ~3; // align to 32 bit boundary
			if ((count - byteswritten) < reclen)
				break;

			struct compat_linux_dirent* dirent = (struct compat_linux_dirent*) ptr;
			dirent->d_ino = st.st_ino;
			dirent->d_off = dd.pos + 1;
			dirent->d_reclen = reclen;
			strcpy(dirent->d_name, filename.c_str());

			ptr += reclen;
			byteswritten += reclen;
		}
		catch (int e)
		{
			/* Something weird happened while enumerating the directory; just
			 * ignore this file.
			 */
		}

		dd.pos++;
	}

	return byteswritten;
}

int FD::GetDents64(void* buffer, size_t count)
{
	RAIILock locked;
	Ref<VFSNode>& vfsnode = GetVFSNode();
	if (!vfsnode)
		throw ENOTDIR;

	u8* ptr = (u8*) buffer;
	DirData& dd = get_dirdata();
	unsigned int byteswritten = 0;
	for (;;)
	{
		if (dd.pos == dd.contents.size())
			break;

		try
		{
			const string& filename = dd.contents[dd.pos];

			struct stat st;
			vfsnode->StatFile(filename, st);

			size_t reclen = sizeof(struct linux_dirent64) +
					filename.size() + 1;
			reclen = (reclen + 7) & ~7; // align to 64 bit boundary
			if ((count - byteswritten) < reclen)
				break;

			struct linux_dirent64* dirent = (struct linux_dirent64*) ptr;
			dirent->d_ino = st.st_ino;
			dirent->d_off = dd.pos + 1;
			dirent->d_reclen = reclen;
			dirent->d_type = get_file_type(st);
			strcpy(dirent->d_name, filename.c_str());

			ptr += reclen;
			byteswritten += reclen;
		}
		catch (int e)
		{
			/* Something weird happened while enumerating the directory; just
			 * ignore this file.
			 */
		}

		dd.pos++;
	}

	return byteswritten;
}
