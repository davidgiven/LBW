#include "globals.h"
#include "VFS.h"
#include "VFSNode.h"

VFSNode::VFSNode(VFSNode* parent, const string& name):
	_parent(parent),
	_name(name)
{
}

VFSNode::~VFSNode()
{
}

Ref<VFSNode> VFSNode::GetParent()
{
	return _parent;
}

const string& VFSNode::GetName() const
{
	return _name;
}

string VFSNode::GetPath()
{
	Ref<VFSNode> parent = GetParent();

	if (parent == this)
		return "";
	else
		return parent->GetPath() + "/" + _name;
}

int VFSNode::GetFileType(const string& name)
{
	if ((name == ".") || (name == ".."))
		return DIRECTORY;

	try
	{
		struct stat st;
		StatFile(name, st);

		if (S_ISREG(st.st_mode))
			return FILE;
		if (S_ISDIR(st.st_mode))
			return DIRECTORY;
		if (S_ISLNK(st.st_mode))
			return LINK;
		if (S_ISCHR(st.st_mode))
			return CHAR;
		if (S_ISBLK(st.st_mode))
			return BLOCK;
		if (S_ISSOCK(st.st_mode))
			return SOCKET;
		if (S_ISFIFO(st.st_mode))
			return FIFO;
		error("strange file type!");
	}
	catch (int e)
	{
		if (e == ENOENT)
			return MISSING;
		throw e;
	}
}

void VFSNode::Resolve(const string& path, Ref<VFSNode>& node, string& leaf,
		bool followlink)
{
	node = this;
	string::size_type left = 0;
	for (;;)
	{
		string::size_type right = path.find_first_of('/', left);
		string element;
		if (right == string::npos)
			element = path.substr(left);
		else
			element = path.substr(left, right-left);

		if (element == "")
			element = ".";

		int type;
		for (;;)
		{
			type = node->GetFileType(element);
			if (type != LINK)
				break;

			if (!followlink && (right == string::npos))
				break;

			string target = node->ReadLink(element);
			Ref<VFSNode> linktarget;

			if (!target.empty() && (target[0] == '/'))
				VFS::Resolve(target, linktarget, element, true);
			else
				node->Resolve(target, linktarget, element, true);
			node = linktarget;
		}

		if (right == string::npos)
		{
			leaf = element;
			return;
		}

		switch (type)
		{
			case MISSING:
				throw ENOENT;

			case FILE:
			case BLOCK:
			case CHAR:
			case FIFO:
			case SOCKET:
				throw ENOTDIR;

			case DIRECTORY:
			{
				node = node->Traverse(element);
				left = right + 1;
				break;
			}
		}
	}
}

void VFSNode::StatFile(const string& name, struct stat& st)
{
	throw ENOENT;
}

Ref<VFSNode> VFSNode::Traverse(const string& name)
{
	log("fallback Traverse");
	throw ENOENT;
}

Ref<FD> VFSNode::OpenFile(const string& name, int flags, int mode)
{
	log("fallback OpenFile");
	throw ENOENT;
}

string VFSNode::ReadLink(const string& name)
{
	assert(false);
	throw EINVAL;
}

deque<string> VFSNode::Enumerate()
{
	assert(false);
	throw EINVAL;
}

void VFSNode::MkDir(const string& name, int mode)
{
	assert(false);
	throw EINVAL;
}

void VFSNode::RmDir(const string& name)
{
	assert(false);
	throw EINVAL;
}

int VFSNode::Access(const string& name, int mode)
{
	assert(false);
	throw EINVAL;
}

void VFSNode::Rename(const string& from, VFSNode* tonode, const string& to)
{
	assert(false);
	throw EINVAL;
}

void VFSNode::Chmod(const string& name, int mode)
{
	assert(false);
	throw EINVAL;
}

void VFSNode::Link(const string& from, VFSNode* tonode, const string& to)
{
	assert(false);
	throw EINVAL;
}

void VFSNode::Unlink(const string& name)
{
	assert(false);
	throw EINVAL;
}

void VFSNode::Symlink(const string& name, const string& target)
{
	assert(false);
	throw EINVAL;
}

