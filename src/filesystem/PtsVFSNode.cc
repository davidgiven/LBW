/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "filesystem/RealFD.h"
#include "filesystem/InterixVFSNode.h"
#include "filesystem/PtsVFSNode.h"
#include "filesystem/FakeFile.h"
#include <stdlib.h>
#include <limits.h>
#include <sys/mkdev.h>

PtsVFSNode::PtsVFSNode(VFSNode* parent, const string& name):
	FakeVFSNode(parent, name)
{
	AddFile(new TunnelledFakeDirectory("ptms", "/dev/ptmx"));
}

PtsVFSNode::~PtsVFSNode()
{
}

static string findpty(const string& name)
{
	u32 number = strtoul(name.c_str(), NULL, 10);
	if ((number == 0) || (number == ULONG_MAX))
		return "";

	char buffer[16] = "/dev/tty";
	*(u32*) (buffer+8) = number;
	buffer[12] = '\0';

	/* A brief gesture to security checking. */
	if (!buffer[8])
		return "";
	for (int i = 8; i < 12; i++)
	{
		if (buffer[i] &&
		    ((buffer[i] == '/') || (buffer[i] < 32) || (buffer[i] > 126)))
			return "";
	}

	return buffer;
}

void PtsVFSNode::StatFile(const string& name, struct stat& st)
{
	string f = findpty(name);
	if (!f.empty())
	{
		int i = stat(f.c_str(), &st);
		if (i == -1)
			throw errno;

		/* libc expects slave devices to have a particular rdev.
		 * So we have to fake it here.
		 */
		st.st_rdev = mkdev(3, 0);
		return;
	}
	return FakeVFSNode::StatFile(name, st);
}

Ref<FD> PtsVFSNode::OpenFile(const string& name, int flags, int mode)
{
	string f = findpty(name);
	if (!f.empty())
	{
		int newfd = open(f.c_str(), flags, mode);
		if (newfd == -1)
			throw errno;

		return new RealFD(newfd);
	}
	return FakeVFSNode::OpenFile(name, flags, mode);
}

int PtsVFSNode::Access(const string& name, int mode)
{
	string f = findpty(name);
	if (!f.empty())
	{
		int i = access(f.c_str(), mode);
		if (i == -1)
			throw errno;
		return i;
	}
	return FakeVFSNode::Access(name, mode);
}

void PtsVFSNode::Chown(const string& name, uid_t owner, gid_t group)
{
	string f = findpty(name);
	if (!f.empty())
	{
		if (Options.FakeRoot)
			return;

		int i = chown(f.c_str(), owner, group);
		if (i == -1)
			throw errno;
		return;
	}
	return FakeVFSNode::Chown(name, owner, group);
}
