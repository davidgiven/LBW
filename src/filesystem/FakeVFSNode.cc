/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "filesystem/RawFD.h"
#include "filesystem/InterixVFSNode.h"
#include "filesystem/FakeVFSNode.h"
#include "filesystem/FakeFile.h"

FakeVFSNode::FakeVFSNode(VFSNode* parent, const string& name):
	VFSNode(parent, name)
{
}

FakeVFSNode::~FakeVFSNode()
{
}

void FakeVFSNode::AddFile(FakeFile* file)
{
	string name = file->GetName();
	_files[name] = file;
}

void FakeVFSNode::StatFile(const string& name, struct stat& st)
{
	if (name == "..")
		return GetParent()->StatFile(".", st);
	else if (name == ".")
		return GetParent()->StatFile(GetName(), st);

	FilesMap::const_iterator i = _files.find(name);
	if (i == _files.end())
		throw ENOENT;

	i->second->Stat(st);
	if (st.st_ino == 0)
		st.st_ino = 1;
}

Ref<VFSNode> FakeVFSNode::Traverse(const string& name)
{
	if ((name == ".") || (name.empty()))
		return this;
	else if (name == "..")
		return GetParent();

	FilesMap::const_iterator i = _files.find(name);
	if (i == _files.end())
		throw ENOENT;

	return i->second->OpenDirectory(this);
}

Ref<FD> FakeVFSNode::OpenFile(const string& name, int flags, int mode)
{
	if ((name == ".") || (name == "..") || name.empty())
		throw EISDIR;

	FilesMap::const_iterator i = _files.find(name);
	if (i == _files.end())
		throw ENOENT;

	return i->second->OpenFile(flags, mode);
}

deque<string> FakeVFSNode::Enumerate()
{
	deque<string> files;
	files.push_back(".");
	files.push_back("..");

	FilesMap::const_iterator i = _files.begin();
	while (i != _files.end())
	{
		files.push_back(i->second->GetName());
		i++;
	}

	return files;
}

int FakeVFSNode::Access(const string& name, int mode)
{
	if (name == "..")
		return GetParent()->Access(".", mode);
	else if (name == ".")
		return GetParent()->Access(GetName(), mode);

	FilesMap::const_iterator i = _files.find(name);
	if (i == _files.end())
		throw ENOENT;

	struct stat st;
	i->second->Stat(st);


	throw ENOENT;
}
