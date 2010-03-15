/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "filesystem/RawFD.h"
#include "filesystem/InterixVFSNode.h"
#include "filesystem/DevVFSNode.h"

struct DevVFSNode::DevFile
{
	const char* name;
	u32 filetype;
	u32 rdev;
	const char* interixpath;
};

#define DEVICE(major, minor) ((major<<8) | minor)

static const DevVFSNode::DevFile devices[] =
{
	{ "null",      S_IFCHR, DEVICE(1, 3), "/dev/null" },
	{ "zero",      S_IFCHR, DEVICE(1, 5), "/dev/zero" },
	{ "tty",       S_IFCHR, DEVICE(5, 0), "/dev/tty" },
	{ "random",    S_IFCHR, DEVICE(1, 8), "/dev/random" },
	{ "urandom",   S_IFCHR, DEVICE(1, 9), "/dev/urandom" },
	{ "full",      S_IFCHR, DEVICE(1, 7), "/dev/full" },
	{ "fs",        S_IFDIR, 0,            "/dev/fs" }
};

#define lengthof(array) (sizeof(array) / sizeof(*array))

DevVFSNode::DevVFSNode(VFSNode* parent, const string& name):
	VFSNode(parent, name)
{
	for (size_t i = 0; i < lengthof(devices); i++)
	{
		const DevFile& df = devices[i];
		_devicemap[df.name] = &df;
	}
}

DevVFSNode::~DevVFSNode()
{
}

const DevVFSNode::DevFile& DevVFSNode::finddevice(const string& name)
{
	map<string, const DevFile*>::const_iterator i = _devicemap.find(name);
	if (i == _devicemap.end())
		throw ENOENT;

	return *(i->second);
}

void DevVFSNode::StatFile(const string& name, struct stat& st)
{
	if (name == "..")
		return GetParent()->StatFile(".", st);
	if (name == ".")
		return GetParent()->StatFile(GetName(), st);

	const DevFile& df = finddevice(name);
	int i = lstat(df.interixpath, &st);
	if (i == -1)
		throw errno;

	if ((df.filetype == S_IFBLK) || (df.filetype == S_IFCHR))
		st.st_rdev = df.rdev;
}

Ref<VFSNode> DevVFSNode::Traverse(const string& name)
{
	if ((name == ".") || (name.empty()))
		return this;
	else if (name == "..")
		return GetParent();

	const DevFile& df = finddevice(name);
	if (df.filetype == S_IFDIR)
		return new InterixVFSNode(this, df.name, df.interixpath);
	throw ENOTDIR;
}

Ref<FD> DevVFSNode::OpenFile(const string& name, int flags,	int mode)
{
	if ((name == ".") || (name == "..") || name.empty())
		throw EISDIR;

	const DevFile& df = finddevice(name);
	if (df.filetype == S_IFDIR)
		throw EISDIR;

	Ref<RawFD> ref = new RawFD();
	ref->Open(df.interixpath, flags, mode);
	return (FD*) ref;
}

deque<string> DevVFSNode::Enumerate()
{
	deque<string> files;
	files.push_back(".");
	files.push_back("..");

	for (size_t i = 0; i < lengthof(devices); i++)
	{
		const DevFile& df = devices[i];
		files.push_back(df.name);
	}

	return files;
}

int DevVFSNode::Access(const string& name, int mode)
{
	if (name == "..")
		return GetParent()->Access(".", mode);

	const DevFile& df = finddevice(name);
	int i = access(df.interixpath, mode);
	if (i == -1)
		throw errno;
	return i;
}
