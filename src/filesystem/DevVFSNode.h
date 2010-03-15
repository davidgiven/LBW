/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef DEVVFSNODE_H
#define DEVVFSNODE_H

#include "VFSNode.h"
#include <map>

using std::map;

class DevVFSNode : public VFSNode
{
public:
	DevVFSNode(VFSNode* parent, const string& name);
	~DevVFSNode();

public:
	void StatFile(const string& name, struct stat& st);
	Ref<VFSNode> Traverse(const string& name);
	Ref<FD> OpenFile(const string& name, int flags = O_RDONLY,
			int mode = 0);
	deque<string> Enumerate();
	int Access(const string& name, int mode);

public:
	struct DevFile;
private:
	const DevFile& finddevice(const string& name);

private:
	map<string, const DevFile*> _devicemap;
};

#endif
