/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef VFS_H
#define VFS_H

#include "VFSNode.h"

class VFS
{
public:
	static void SetRoot(const string& path);
	static Ref<VFSNode> GetRootNode();

	static void SetCWD(VFSNode* cwd, const string& path);
	static string GetCWD();
	static Ref<VFSNode> GetCWDNode();

	static void Resolve(VFSNode* cwd, const string& path, Ref<VFSNode>& node,
			string& leaf, bool followlink = true);

	static Ref<FD> OpenDirectory(VFSNode* cwd, const string& path,
			bool nofollow = false);
	static Ref<FD> OpenFile(VFSNode* cwd, const string& path,
			int flags = O_RDONLY, int mode = 0, bool nofollow = false);

	static void Stat(VFSNode* cwd, const string& path, struct stat& st);
	static void StatFS(VFSNode* cwd, const string& path, struct statvfs& st);
	static void Lstat(VFSNode* cwd, const string& path, struct stat& st);
	static void MkDir(VFSNode* cwd, const string& path, int mode = 0);
	static void RmDir(VFSNode* cwd, const string& path);
	static void Mknod(VFSNode* cwd, const string& path, mode_t mode, dev_t dev);
	static string ReadLink(VFSNode* cwd, const string& path);
	static int Access(VFSNode* cwd, const string& path, int mode);
	static void Rename(VFSNode* cwd, const string& from, const string& to);
	static void Lchmod(VFSNode* cwd, const string& path, int mode);
	static void Chmod(VFSNode* cwd, const string& path, int mode);
	static void Chown(VFSNode* cwd, const string& path, uid_t owner, gid_t group);
	static void Lchown(VFSNode* cwd, const string& path, uid_t owner, gid_t group);
	static void Link(VFSNode* cwd, const string& target, const string& path);
	static void Unlink(VFSNode* cwd, const string& path);
	static void Symlink(VFSNode* cwd, const string& target, const string& path);
	static void Utimes(VFSNode* cwd, const string& path, const struct timeval times[2]);
};

#endif
