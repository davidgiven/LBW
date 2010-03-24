/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef VFSNODE_H
#define VFSNODE_H

#include <deque>
using std::deque;

#include "FD.h"

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
	virtual void StatFS(struct statvfs& st);

	virtual Ref<VFSNode> Traverse(const string& name);
	virtual Ref<FD> OpenDirectory();
	virtual Ref<FD> OpenFile(const string& name, int flags = O_RDONLY,
			int mode = 0);
	virtual string ReadLink(const string& name) { throw EINVAL; }
	virtual deque<string> Enumerate() { throw EINVAL; }
	virtual void MkDir(const string& name, int mode = 0) { throw EINVAL; }
	virtual void RmDir(const string& name) { throw EINVAL; }
	virtual void Mknod(const string& name, mode_t mode, dev_t dev) { throw EINVAL; }
	virtual int Access(const string& name, int mode) { throw EINVAL; }
	virtual void Rename(const string& from, VFSNode* tonode, const string& to) { throw EINVAL; }
	virtual void Chmod(const string& name, int mode) { throw EINVAL; }
	virtual void Chown(const string& name, uid_t owner, gid_t group) { throw EINVAL; }
	virtual void Link(const string& from, VFSNode* tonode, const string& to) { throw EINVAL; }
	virtual void Unlink(const string& name) { throw EINVAL; }
	virtual void Symlink(const string& name, const string& target) { throw EINVAL; }
	virtual void Utimes(const string& name, const struct timeval times[2]) { throw EINVAL; }

	void Resolve(const string& path, Ref<VFSNode>& node, string& leaf,
			bool followlink);

private:
	Ref<VFSNode> _parent;
	string _name;
};

#endif
