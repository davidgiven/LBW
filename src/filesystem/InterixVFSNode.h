/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef INTERIXVFSNODE_H
#define INTERIXVFSNODE_H

#include "VFSNode.h"

class InterixVFSNode : public VFSNode
{
public:
	InterixVFSNode(VFSNode* parent, const string& name, const string& path);
	InterixVFSNode(VFSNode* parent, const string& name);
	~InterixVFSNode();

private:
	void init(VFSNode* parent, const string& name, const string& path);

public:
	int GetRealFD() { return _realfd; }
	string GetPath();

	void StatFile(const string& name, struct stat& st);
	Ref<VFSNode> Traverse(const string& name);
	Ref<FD> OpenFile(const string& name, int flags = O_RDONLY,
			int mode = 0);
	string ReadLink(const string& name);
	deque<string> Enumerate();
	void MkDir(const string& name, int mode = 0);
	void RmDir(const string& name);
	int Access(const string& name, int mode);
	void Rename(const string& from, VFSNode* other, const string& to);
	void Chmod(const string& name, int mode);
	void Link(const string& from, VFSNode* other, const string& to);
	void Unlink(const string& name);
	void Symlink(const string& name, const string& target);
	void Utime(const string& name, const struct utimbuf& ub);

private:
	void setup();
	void setup(const string& name, int e = EINVAL);

private:
	int _realfd;
};

#endif
