/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "filesystem/RawFD.h"
#include "filesystem/InterixVFSNode.h"
#include "filesystem/FakeVFSNode.h"
#include "filesystem/FakeFile.h"

FakeFile::~FakeFile()
{
}

string FakeFile::GetName()
{
	assert(false);
	throw EINVAL;
}

Ref<VFSNode> FakeFile::OpenDirectory(VFSNode* parent)
{
	throw EINVAL;
}

Ref<FD> FakeFile::OpenFile(int flags, int mode)
{
	throw EINVAL;
}

void FakeFile::Stat(struct stat& st)
{
	throw EINVAL;
}

int FakeFile::Access(int mode)
{
	throw EINVAL;
}

FakeDirectory::FakeDirectory(VFSNode* node):
		_node(node)
{
}

FakeDirectory::~FakeDirectory()
{
}

string FakeDirectory::GetName()
{
	return _node->GetName();
}

Ref<VFSNode> FakeDirectory::OpenDirectory(VFSNode* parent)
{
	return _node;
}

void FakeDirectory::Stat(struct stat& st)
{
	memset(&st, 0, sizeof(st));
	st.st_mode = S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO;
	st.st_ino = 1;
}

int FakeDirectory::Access(int mode)
{
	return mode;
}

TunnelledFakeFileOrDirectory::TunnelledFakeFileOrDirectory(
			const string& localname, const string& destname):
		_localname(localname),
		_destname(destname)
{
}

string TunnelledFakeFileOrDirectory::GetName()
{
	return _localname;
}

void TunnelledFakeFileOrDirectory::Stat(struct stat& st)
{
	int i = stat(_destname.c_str(), &st);
	if (i == -1)
		throw errno;
}

int TunnelledFakeFileOrDirectory::Access(int mode)
{
	int i = access(_destname.c_str(), mode);
	if (i == -1)
		throw errno;
	return i;
}

TunnelledFakeFile::TunnelledFakeFile(
			const string& localname, const string& destname):
		TunnelledFakeFileOrDirectory(localname, destname)
{
}

Ref<FD> TunnelledFakeFile::OpenFile(int flags, int mode)
{
	Ref<RawFD> ref = new RawFD();
	ref->Open(_destname.c_str(), flags, mode);
	return (FD*) ref;
}

TunnelledFakeDirectory::TunnelledFakeDirectory(
			const string& localname, const string& destname):
		TunnelledFakeFileOrDirectory(localname, destname)
{
}

Ref<VFSNode> TunnelledFakeDirectory::OpenDirectory(VFSNode* parent)
{
	return new InterixVFSNode(parent, _localname, _destname);
}
