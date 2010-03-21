/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef PTSVFSNODE_H
#define PTSVFSNODE_H

#include "FakeVFSNode.h"
#include <map>

class PtsVFSNode : public FakeVFSNode
{
public:
	PtsVFSNode(VFSNode* parent, const string& name);
	~PtsVFSNode();

public:
	void StatFile(const string& name, struct stat& st);
	Ref<FD> OpenFile(const string& name, int flags = O_RDONLY,
			int mode = 0);
	int Access(const string& name, int mode);

	void Chown(const string& name, uid_t owner, gid_t group);
};

#endif
