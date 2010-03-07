#ifndef DIRFD_H
#define DIRFD_H

#include "FileFD.h"
#include <deque>
#include <dirent.h>

using std::deque;

class VFSNode;
class DirectoryEntry;

class DirFD : public FD
{
public:
	DirFD();
	~DirFD();

public:
	void Open(VFSNode* vfsnode);

public:
	virtual int GetDents(void* buffer, size_t count);
	virtual int GetDents64(void* buffer, size_t count);
	virtual void Fstat(struct stat& ls);

private:
	Ref<VFSNode> _vfsnode;
	unsigned int _pos;
	deque<string> _contents;
};

#endif
