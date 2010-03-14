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

	static void SetCWD(const string& path);
	static string GetCWD();
	static Ref<VFSNode> GetCWDNode();

	static void Resolve(const string& path, Ref<VFSNode>& node,
			string& leaf, bool followlink = true);

	static Ref<FD> OpenDirectory(const string& path, bool nofollow = false);
	static Ref<FD> OpenFile(const string& path, int flags = O_RDONLY,
			int mode = 0, bool nofollow = false);

	static void Stat(const string& path, struct stat& st);
	static void Lstat(const string& path, struct stat& st);
	static void MkDir(const string& path, int mode = 0);
	static void RmDir(const string& path);
	static string ReadLink(const string& path);
	static int Access(const string& path, int mode);
	static void Rename(const string& from, const string& to);
	static void Chmod(const string& path, int mode);
	static void Link(const string& from, const string& to);
	static void Unlink(const string& path);
	static void Symlink(const string& from, const string& to);
	static void Utime(const string& path, const struct utimbuf& ub);
};

#endif
