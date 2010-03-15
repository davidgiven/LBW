/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "RootVFSNode.h"

RootVFSNode::RootVFSNode(const string& path):
	InterixVFSNode(NULL, "", path),
	_devfs(new DevVFSNode(this, "dev"))
{
}

RootVFSNode::~RootVFSNode()
{
}

Ref<VFSNode> RootVFSNode::GetParent()
{
	return this;
}

Ref<VFSNode> RootVFSNode::Traverse(const string& name)
{
	if (name == "dev")
		return _devfs;

	return InterixVFSNode::Traverse(name);
}

