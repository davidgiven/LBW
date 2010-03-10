/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef ROOTVFSNODE_H
#define ROOTVFSNODE_H

#include "InterixVFSNode.h"

class RootVFSNode : public InterixVFSNode
{
public:
	RootVFSNode(const string& path);
	~RootVFSNode();

	Ref<VFSNode> GetParent();
};

#endif
