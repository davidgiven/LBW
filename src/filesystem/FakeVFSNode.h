/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef FakeVFSNODE_H
#define FakeVFSNODE_H

#include "VFSNode.h"
#include <map>

using std::map;

class FakeFile;

class FakeVFSNode : public VFSNode
{
public:
	FakeVFSNode(VFSNode* parent, const string& name);
	~FakeVFSNode();

public:
	void StatFS(struct statvfs& st);

	void StatFile(const string& name, struct stat& st);
	Ref<VFSNode> Traverse(const string& name);
	Ref<FD> OpenDirectory();
	Ref<FD> OpenFile(const string& name, int flags = O_RDONLY,
			int mode = 0);
	deque<string> Enumerate();
	int Access(const string& name, int mode);

public:
	void AddFile(FakeFile* file);
	void AddDirectory(VFSNode* node);

private:
	typedef map<string, Ref<FakeFile> > FilesMap;
	FilesMap _files;
};

#endif
