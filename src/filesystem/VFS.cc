/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "VFS.h"
#include "RootVFSNode.h"
#include <typeinfo>

//#define VERBOSE

static Ref<RootVFSNode> root;
static Ref<VFSNode> cwd;

void VFS::SetRoot(const string& path)
{
	root = new RootVFSNode(path);
	cwd = (VFSNode*) root;
}

Ref<VFSNode> VFS::GetRootNode()
{
	return (VFSNode*) root;
}

void VFS::SetCWD(VFSNode* cwd, const string& path)
{
	RAIILock locked;

#if defined VERBOSE
	log("SetCWD(%p, %s)", cwd, path.c_str());
#endif

	Ref<VFSNode> node;
	string leaf;
	Resolve(cwd, path, node, leaf);

	::cwd = node->Traverse(leaf);
}

string VFS::GetCWD()
{
	RAIILock locked;

	string s = cwd->GetPath();
	if (s.empty())
		return "/";
	return s;
}

Ref<VFSNode> VFS::GetCWDNode()
{
	return (VFSNode*) cwd;
}

void VFS::Resolve(VFSNode* cwd, const string& path, Ref<VFSNode>& node,
		string& leaf, bool followlink)
{
	RAIILock locked;

#if defined VERBOSE
	//log("Resolve(%s)", path.c_str());
#endif

	string p = path;
	if (!p.empty() && (p[0] == '/'))
		root->Resolve(p.substr(1), node, leaf, followlink);
	else
	{
		if (!cwd)
			cwd = ::cwd;
		cwd->Resolve(p, node, leaf, followlink);
	}
}

Ref<FD> VFS::OpenDirectory(VFSNode* cwd, const string& path, bool nofollow)
{
	Ref<VFSNode> node;
	string leaf;
	Resolve(cwd, path, node, leaf, !nofollow);

#if defined VERBOSE
	log("%s(%s)", __FUNCTION__, path.c_str());
#endif

	node = node->Traverse(leaf);
	return node->OpenDirectory();
}

Ref<FD> VFS::OpenFile(VFSNode* cwd, const string& path, int flags, int mode,
		bool nofollow)
{
#if defined VERBOSE
	log("%s(%s)", __FUNCTION__, path.c_str());
#endif

	Ref<VFSNode> node;
	string leaf;
	Resolve(cwd, path, node, leaf, !nofollow);

	return node->OpenFile(leaf, flags, mode);
}

void VFS::Stat(VFSNode* cwd, const string& path, struct stat& st)
{
#if defined VERBOSE
	log("%s(%s)", __FUNCTION__, path.c_str());
#endif

	Ref<VFSNode> node;
	string leaf;
	Resolve(cwd, path, node, leaf);

	node->StatFile(leaf, st);
}

void VFS::StatFS(VFSNode* cwd, const string& path, struct statvfs& st)
{
#if defined VERBOSE
	log("%s(%s)", __FUNCTION__, path.c_str());
#endif

	Ref<VFSNode> node;
	string leaf;
	Resolve(cwd, path, node, leaf);

	if (node->GetFileType(leaf) == VFSNode::DIRECTORY)
		node = node->Traverse(leaf);

	node->StatFS(st);
}

void VFS::Lstat(VFSNode* cwd, const string& path, struct stat& st)
{
#if defined VERBOSE
	log("%s(%s)", __FUNCTION__, path.c_str());
#endif

	Ref<VFSNode> node;
	string leaf;
	Resolve(cwd, path, node, leaf, false);

	node->StatFile(leaf, st);
}

void VFS::MkDir(VFSNode* cwd, const string& path, int mode)
{
#if defined VERBOSE
	log("%s(%s)", __FUNCTION__, path.c_str());
#endif

	Ref<VFSNode> node;
	string leaf;
	Resolve(cwd, path, node, leaf, false);

	node->MkDir(leaf, mode);
}

void VFS::RmDir(VFSNode* cwd, const string& path)
{
#if defined VERBOSE
	log("%s(%s)", __FUNCTION__, path.c_str());
#endif

	Ref<VFSNode> node;
	string leaf;
	Resolve(cwd, path, node, leaf, false);

	if ((leaf == ".") || (leaf.empty()))
	{
		leaf = node->GetName();
		node = node->Traverse("..");
	}

	node->RmDir(leaf);
}

string VFS::ReadLink(VFSNode* cwd, const string& path)
{
#if defined VERBOSE
	log("%s(%s)", __FUNCTION__, path.c_str());
#endif

	Ref<VFSNode> node;
	string leaf;
	Resolve(cwd, path, node, leaf, false);

	return node->ReadLink(leaf);
}

int VFS::Access(VFSNode* cwd, const string& path, int mode)
{
#if defined VERBOSE
	log("%s(%s)", __FUNCTION__, path.c_str());
#endif

	Ref<VFSNode> node;
	string leaf;
	Resolve(cwd, path, node, leaf, true);

	return node->Access(leaf, mode);
}

void VFS::Rename(VFSNode* cwd, const string& from, const string& to)
{
#if defined VERBOSE
	log("%s(%s, %s)", __FUNCTION__, from.c_str(), to.c_str());
#endif

	Ref<VFSNode> fromnode;
	string fromleaf;
	Resolve(cwd, from, fromnode, fromleaf, false);

	Ref<VFSNode> tonode;
	string toleaf;
	Resolve(cwd, to, tonode, toleaf, false);

	if (typeid((VFSNode*)fromnode) != typeid((VFSNode*)tonode))
		throw EXDEV;

	fromnode->Rename(fromleaf, tonode, toleaf);
}

void VFS::Chmod(VFSNode* cwd, const string& path, int mode)
{
#if defined VERBOSE
	log("%s(%s)", __FUNCTION__, path.c_str());
#endif

	Ref<VFSNode> node;
	string leaf;
	Resolve(cwd, path, node, leaf, true);

	node->Chmod(leaf, mode);
}

void VFS::Lchmod(VFSNode* cwd, const string& path, int mode)
{
#if defined VERBOSE
	log("%s(%s)", __FUNCTION__, path.c_str());
#endif

	Ref<VFSNode> node;
	string leaf;
	Resolve(cwd, path, node, leaf, false);

	node->Chmod(leaf, mode);
}

void VFS::Chown(VFSNode* cwd, const string& path, uid_t owner, gid_t group)
{
#if defined VERBOSE
	log("%s(%s)", __FUNCTION__, path.c_str());
#endif

	Ref<VFSNode> node;
	string leaf;
	Resolve(cwd, path, node, leaf, true);

	node->Chown(leaf, owner, group);
}

void VFS::Lchown(VFSNode* cwd, const string& path, uid_t owner, gid_t group)
{
#if defined VERBOSE
	log("%s(%s)", __FUNCTION__, path.c_str());
#endif

	Ref<VFSNode> node;
	string leaf;
	Resolve(cwd, path, node, leaf, false);

	node->Chown(leaf, owner, group);
}

void VFS::Link(VFSNode* cwd, const string& target, const string& path)
{
#if defined VERBOSE
	log("%s(%s, %s)", __FUNCTION__, from.c_str(), to.c_str());
#endif

	Ref<VFSNode> targetnode;
	string targetleaf;
	Resolve(cwd, target, targetnode, targetleaf, false);

	Ref<VFSNode> pathnode;
	string pathleaf;
	Resolve(cwd, path, pathnode, pathleaf, false);

	if (typeid((VFSNode*)pathnode) != typeid((VFSNode*)pathnode))
		throw EXDEV;

	pathnode->Link(pathleaf, targetnode, targetleaf);
}

void VFS::Unlink(VFSNode* cwd, const string& path)
{
#if defined VERBOSE
	log("%s(%s)", __FUNCTION__, path.c_str());
#endif

	Ref<VFSNode> node;
	string leaf;
	Resolve(cwd, path, node, leaf, false);

	node->Unlink(leaf);
}

void VFS::Symlink(VFSNode* cwd, const string& target, const string& path)
{
#if defined VERBOSE
	log("%s(%s, %s)", __FUNCTION__, from.c_str(), to.c_str());
#endif

	Ref<VFSNode> node;
	string leaf;
	Resolve(cwd, path, node, leaf, false);

	node->Symlink(leaf, target);
}

void VFS::Utimes(VFSNode* cwd, const string& path, const struct timeval times[2])
{
#if defined VERBOSE
	log("%s(%s)", __FUNCTION__, path.c_str());
#endif

	Ref<VFSNode> node;
	string leaf;
	Resolve(cwd, path, node, leaf, false);

	node->Utimes(leaf, times);
}

