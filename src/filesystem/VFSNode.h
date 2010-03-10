/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef VFSNODE_H
#define VFSNODE_H

#include <deque>
using std::deque;

#include "FD.h"
#include "DirFD.h"

class VFSNode : public HasRefCount
{
public:
	VFSNode(VFSNode* parent, const string& name);
	virtual ~VFSNode();

public:
	enum
	{
		MISSING,
		FILE,
		DIRECTORY,
		LINK,
		BLOCK,
		CHAR,
		FIFO,
		SOCKET
	};

public:
	virtual Ref<VFSNode> GetParent();
	const string& GetName() const;
	string GetPath();

	int GetFileType(const string& name);
	virtual void StatFile(const string& name, struct stat& st);

	virtual Ref<VFSNode> Traverse(const string& name);
	virtual Ref<FD> OpenFile(const string& name, int flags = O_RDONLY,
			int mode = 0);
	virtual string ReadLink(const string& name);
	virtual deque<string> Enumerate();
	virtual void MkDir(const string& name, int mode = 0);
	virtual void RmDir(const string& name);
	virtual int Access(const string& name, int mode);
	virtual void Rename(const string& from, VFSNode* tonode, const string& to);
	virtual void Chmod(const string& name, int mode);
	virtual void Link(const string& from, VFSNode* tonode, const string& to);
	virtual void Unlink(const string& name);
	virtual void Symlink(const string& name, const string& target);

	void Resolve(const string& path, Ref<VFSNode>& node, string& leaf,
			bool followlink);

private:
	Ref<VFSNode> _parent;
	string _name;
};

#endif
