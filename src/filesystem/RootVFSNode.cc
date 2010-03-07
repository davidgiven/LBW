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
