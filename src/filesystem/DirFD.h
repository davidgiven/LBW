/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef DIRFD_H
#define DIRFD_H

#include "RawFD.h"
#include <deque>
#include <dirent.h>

using std::deque;

class VFSNode;
class DirectoryEntry;

class DirFD : public FD
{
public:
	static DirFD* Cast(FD* fd);

public:
	DirFD();
	~DirFD();

public:
	void Open(VFSNode* vfsnode);

	VFSNode* GetVFSNode() const { return _vfsnode; }

public:
	virtual int GetDents(void* buffer, size_t count);
	virtual int GetDents64(void* buffer, size_t count);
	virtual void Fstat(struct stat& ls);
	virtual void Fchmod(mode_t mode);

	virtual int Fcntl(int cmd, u_int32_t argument);

private:
	Ref<VFSNode> _vfsnode;
	unsigned int _pos;
	deque<string> _contents;
};

#endif
