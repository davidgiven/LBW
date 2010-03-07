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
