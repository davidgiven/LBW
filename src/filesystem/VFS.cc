#include "globals.h"
#include "VFS.h"
#include "RootVFSNode.h"
#include <typeinfo>

static Ref<RootVFSNode> root;
static Ref<VFSNode> cwd;

void VFS::SetRoot(const string& path)
{
	root = new RootVFSNode(path);
	cwd = (VFSNode*) root;
}

Ref<VFSNode> VFS::GetRoot()
{
	return (VFSNode*) root;
}

void VFS::SetCWD(const string& path)
{
	RAIILock locked;

	Ref<VFSNode> node;
	string leaf;
	Resolve(path, node, leaf);

	cwd = node->Traverse(leaf);
}

string VFS::GetCWD()
{
	RAIILock locked;

	string s = cwd->GetPath();
	if (s.empty())
		return "/";
	return s;
}

void VFS::Resolve(const string& path, Ref<VFSNode>& node,
		string& leaf, bool followlink)
{
	RAIILock locked;

	string p = path;
	if (!p.empty() && (p[0] == '/'))
		root->Resolve(p.substr(1), node, leaf, followlink);
	else
		cwd->Resolve(p, node, leaf, followlink);
}

Ref<FD> VFS::OpenDirectory(const string& path, bool nofollow)
{
	Ref<VFSNode> node;
	string leaf;
	Resolve(path, node, leaf, !nofollow);

	node = node->Traverse(leaf);

	Ref<DirFD> dirfd = new DirFD();
	dirfd->Open(node);
	return (FD*) dirfd;
}

Ref<FD> VFS::OpenFile(const string& path, int flags, int mode, bool nofollow)
{
	Ref<VFSNode> node;
	string leaf;
	Resolve(path, node, leaf, !nofollow);

	return node->OpenFile(leaf, flags, mode);
}

void VFS::Stat(const string& path, struct stat& st)
{
	Ref<VFSNode> node;
	string leaf;
	Resolve(path, node, leaf);

	node->StatFile(leaf, st);
}

void VFS::Lstat(const string& path, struct stat& st)
{
	Ref<VFSNode> node;
	string leaf;
	Resolve(path, node, leaf, false);

	node->StatFile(leaf, st);
}

void VFS::MkDir(const string& path, int mode)
{
	Ref<VFSNode> node;
	string leaf;
	Resolve(path, node, leaf, false);

	node->MkDir(leaf, mode);
}

void VFS::RmDir(const string& path)
{
	Ref<VFSNode> node;
	string leaf;
	Resolve(path, node, leaf, false);

	node->RmDir(leaf);
}

string VFS::ReadLink(const string& path)
{
	Ref<VFSNode> node;
	string leaf;
	Resolve(path, node, leaf, false);

	return node->ReadLink(leaf);
}

int VFS::Access(const string& path, int mode)
{
	Ref<VFSNode> node;
	string leaf;
	Resolve(path, node, leaf, true);

	return node->Access(leaf, mode);
}

void VFS::Rename(const string& from, const string& to)
{
	Ref<VFSNode> fromnode;
	string fromleaf;
	Resolve(from, fromnode, fromleaf, false);

	Ref<VFSNode> tonode;
	string toleaf;
	Resolve(to, tonode, toleaf, false);

	if (typeid((VFSNode*)fromnode) != typeid((VFSNode*)tonode))
		throw EXDEV;

	fromnode->Rename(fromleaf, tonode, toleaf);
}

void VFS::Chmod(const string& path, int mode)
{
	Ref<VFSNode> node;
	string leaf;
	Resolve(path, node, leaf, true);

	node->Chmod(leaf, mode);
}

void VFS::Link(const string& from, const string& to)
{
	Ref<VFSNode> fromnode;
	string fromleaf;
	Resolve(from, fromnode, fromleaf, false);

	Ref<VFSNode> tonode;
	string toleaf;
	Resolve(to, tonode, toleaf, false);

	if (typeid((VFSNode*)fromnode) != typeid((VFSNode*)tonode))
		throw EXDEV;

	fromnode->Link(fromleaf, tonode, toleaf);
}

void VFS::Unlink(const string& path)
{
	Ref<VFSNode> node;
	string leaf;
	Resolve(path, node, leaf, false);

	node->Unlink(leaf);
}

void VFS::Symlink(const string& from, const string& to)
{
	Ref<VFSNode> node;
	string leaf;
	Resolve(from, node, leaf, false);

	node->Symlink(leaf, to);
}
