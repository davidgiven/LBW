/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "filesystem/RawFD.h"
#include "filesystem/InterixVFSNode.h"
#include "filesystem/DevVFSNode.h"
#include "filesystem/PtsVFSNode.h"
#include "filesystem/FakeFile.h"

struct DevFile
{
	const char* name;
	const char* interixpath;
};

static const DevFile devices[] =
{
	{ "null",      "/dev/null" },
	{ "zero",      "/dev/zero" },
	{ "tty",       "/dev/tty" },
	{ "random",    "/dev/random" },
	{ "urandom",   "/dev/urandom" },
	{ "full",      "/dev/full" },
	{ "ptmx",      "/dev/ptmx" },
	{ "ptyp0",     "/dev/ptyp0" },
};

#define lengthof(array) (sizeof(array) / sizeof(*array))

DevVFSNode::DevVFSNode(VFSNode* parent, const string& name):
	FakeVFSNode(parent, name),
	_ptsnode(new PtsVFSNode(this, "pts"))
{
	for (size_t i = 0; i < lengthof(devices); i++)
	{
		const DevFile& df = devices[i];
		AddFile(new TunnelledFakeFile(df.name, df.interixpath));
	}

	AddFile(new TunnelledFakeDirectory("fs", "/dev/fs"));

	/* Add any /dev/pty* and /dev/tty* devices we can see. */

	DIR* d = opendir("/dev");
	if (d)
	{
		try
		{
			for (;;)
			{
				struct dirent* de = readdir(d);
				if (!de)
					break;

				if ((memcmp(de->d_name, "pty", 3) == 0) ||
					(memcmp(de->d_name, "tty", 3) == 0))
				{
					string destname = string("/dev/") + de->d_name;
					AddFile(new TunnelledFakeFile(de->d_name, destname));
				}
			}
		}
		catch (int e)
		{
		}
		closedir(d);
	}

	AddDirectory(_ptsnode);
}

DevVFSNode::~DevVFSNode()
{
}
