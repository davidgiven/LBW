#include "globals.h"
#include "filesystem/DirFD.h"
#include "filesystem/VFSNode.h"

#pragma pack(push, 1)
struct compat_linux_dirent {
	u32 d_ino;
	u32 d_off;
	u16 d_reclen;
	char d_name[1];
};

struct linux_dirent64 {
	u64	d_ino;
	s64	d_off;
	unsigned short d_reclen;
	unsigned char d_type;
	char d_name[0];
};
#pragma pack(pop)

#define LINUX_DT_UNKNOWN	0
#define LINUX_DT_FIFO		1
#define LINUX_DT_CHR		2
#define LINUX_DT_DIR		4
#define LINUX_DT_BLK		6
#define LINUX_DT_REG		8
#define LINUX_DT_LNK		10
#define LINUX_DT_SOCK		12
#define LINUX_DT_WHT		14

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

DirFD::DirFD():
	_pos(0)
{
}

DirFD::~DirFD()
{
}

void DirFD::Open(VFSNode* vfsnode)
{
	_vfsnode = vfsnode;
	_contents = _vfsnode->Enumerate();
}

int DirFD::GetDents(void* buffer, size_t count)
{
	u8* ptr = (u8*) buffer;

	unsigned int byteswritten = 0;
	for (;;)
	{
		if (_pos == _contents.size())
			break;

		try
		{
			const string& filename = _contents[_pos];

			struct stat st;
			_vfsnode->StatFile(filename, st);

			size_t reclen = sizeof(struct compat_linux_dirent) +
					filename.size() + 1;
			reclen = (reclen + 3) & ~3; // align to 32 bit boundary
			if ((count - byteswritten) < reclen)
				break;

			struct compat_linux_dirent* dirent = (struct compat_linux_dirent*) ptr;
			dirent->d_ino = st.st_ino;
			dirent->d_off = _pos + 1;
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

		_pos++;
	}

	return byteswritten;
}

int DirFD::GetDents64(void* buffer, size_t count)
{
	u8* ptr = (u8*) buffer;

	unsigned int byteswritten = 0;
	for (;;)
	{
		if (_pos == _contents.size())
			break;

		try
		{
			const string& filename = _contents[_pos];

			struct stat st;
			_vfsnode->StatFile(filename, st);

			size_t reclen = sizeof(struct linux_dirent64) +
					filename.size() + 1;
			reclen = (reclen + 7) & ~7; // align to 64 bit boundary
			if ((count - byteswritten) < reclen)
				break;

			struct linux_dirent64* dirent = (struct linux_dirent64*) ptr;
			dirent->d_ino = st.st_ino;
			dirent->d_off = _pos + 1;
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

		_pos++;
	}

	return byteswritten;
}

void DirFD::Fstat(struct stat& ls)
{
	_vfsnode->StatFile(".", ls);
}
