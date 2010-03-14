/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "filesystem/UnixSocketFD.h"
#include "filesystem/InterixVFSNode.h"
#include "filesystem/VFS.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

UnixSocketFD* UnixSocketFD::Cast(FD* fd)
{
	UnixSocketFD* s = dynamic_cast<UnixSocketFD*>(fd);
	if (!s)
		throw ENOTSOCK;
	return s;
}

void UnixSocketFD::Open(int family, int type, int protocol)
{
	assert(family == AF_UNIX);
	SocketFD::Open(family, type, protocol);
}

void UnixSocketFD::Connect(const struct sockaddr* sa, int addrlen)
{
	//log("connect:");
	//DumpMemory(sa, addrlen);

	if (sa->sa_family != AF_UNIX)
		throw EAFNOSUPPORT;

	/* This is a Unix domain socket, which needs to be created
	 * in the VFS.
	 */

	const struct sockaddr_un* sun = (const struct sockaddr_un*) sa;

	RAIILock locked;
	Ref<VFSNode> node;
	string leaf;
	VFS::Resolve(NULL, sun->sun_path, node, leaf, false);

	InterixVFSNode* inode = dynamic_cast<InterixVFSNode*>((VFSNode*)node);
	if (!inode)
		throw EINVAL;

	int i = fchdir(inode->GetRealFD());
	if (i == -1)
		throw errno;

	struct sockaddr_un childsun;
	childsun.sun_family = AF_UNIX;
	strcpy(childsun.sun_path, leaf.c_str());

	i = connect(GetRealFD(), (const struct sockaddr*) &childsun,
			sizeof(childsun));
	if (i == -1)
		throw errno;
}

void UnixSocketFD::Bind(const struct sockaddr* sa, int addrlen)
{
	//log("bind:");
	//DumpMemory(sa, addrlen);
	throw ENOSYS;
}
