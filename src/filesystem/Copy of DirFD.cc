#include "globals.h"
#include "DirFD.h"

#pragma pack(push, 1)
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

DirFD::DirFD():
	_realfd(-1),
	_pos(0)
{
}

DirFD::~DirFD()
{
	if (_realfd != -1)
		close(_realfd);
}

void DirFD::Open(const string& path)
{
	_realfd = open(path.c_str(), O_RDONLY);
	if (_realfd == -1)
		throw errno;

	DIR* dir = opendir(path.c_str());
	if (!dir)
	{
		close(_realfd);
		throw errno;
	}

	//log("opened <%s> to dirp %p", pathname.c_str(), dir);
	for (;;)
	{
		struct dirent* de = readdir(dir);
		if (!de)
			break;

		string leafname = path + "/" + de->d_name;
		struct stat st;
		int i = stat(leafname.c_str(), &st);

		if (i == -1)
		{
			/* Something freaky happened, like the file being deleted while
			 * scanning the directory; so ignore it.
			 */
			continue;
		}

		_dirents.push_back(*de);
		_stats.push_back(st);
	}
	closedir(dir);
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

int DirFD::GetDents64(struct linux_dirent64* dirent, unsigned int count)
{
	unsigned int byteswritten = 0;
	for (;;)
	{
		if (_pos == _dirents.size())
			break;

		const struct dirent& de = _dirents[_pos];
		const struct stat& st = _stats[_pos];

		size_t reclen = sizeof(*dirent) + de.d_namlen + 1;
		reclen = (reclen + 7) & ~7; // align to 64 bit boundary
		if ((count - byteswritten) < reclen)
			break;

		dirent->d_ino = st.st_ino;
		dirent->d_off = _pos + 1;
		dirent->d_reclen = reclen;
		dirent->d_type = get_file_type(st);
		strcpy(dirent->d_name, de.d_name);

		dirent = (struct linux_dirent64*) (((u_int32_t)dirent) + reclen);
		byteswritten += reclen;

		_pos++;
	}

	return byteswritten;
}

void DirFD::Fstat(struct stat& st)
{
	int result = fstat(_realfd, &st);
	if (result == -1)
		throw errno;
}
