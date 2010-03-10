/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "RootVFSNode.h"

RootVFSNode::RootVFSNode(const string& path):
	InterixVFSNode(NULL, "", path)
{
}

RootVFSNode::~RootVFSNode()
{
}

Ref<VFSNode> RootVFSNode::GetParent()
{
	return this;
}
