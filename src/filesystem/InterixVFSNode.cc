/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "filesystem/RawFD.h"
#include "filesystem/InterixVFSNode.h"
#include <sys/time.h>
#include <utime.h>
#include <typeinfo>

InterixVFSNode::InterixVFSNode(VFSNode* parent, const string& name, const string& path):
	VFSNode(parent, name)
{
	init(parent, name, path);
}

InterixVFSNode::InterixVFSNode(VFSNode* parent, const string& name):
	VFSNode(parent, name)
{
	init(parent, name, name);
}

void InterixVFSNode::init(VFSNode* parent, const string& name, const string& path)
{
	_realfd = open(path.c_str(), O_RDONLY);
	//log("open(%s) -> realfd %d", path.c_str(), _realfd);
	if (_realfd == -1)
		throw errno;

	fcntl(_realfd, F_SETFD, FD_CLOEXEC);
}

InterixVFSNode::~InterixVFSNode()
{
	//log("close(%d)", _realfd);
	close(_realfd);
}

string InterixVFSNode::GetRealPath()
{
	setup();

	char buffer[PATH_MAX];
	char* p = getcwd(buffer, sizeof(buffer));
	if (!p)
		throw errno;
	return buffer;
}

void InterixVFSNode::StatFile(const string& name, struct stat& st)
{
	if (name == "..")
		return GetParent()->StatFile(".", st);

	int i = fchdir(_realfd);
	if (i == -1)
		throw errno;

	i = lstat(name.c_str(), &st);
	if (i == -1)
		throw errno;
}

Ref<VFSNode> InterixVFSNode::Traverse(const string& name)
{
	if ((name == ".") || (name.empty()))
		return this;
	else if (name == "..")
		return GetParent();

	int i = fchdir(_realfd);
//	log("%s(%s/%s) -> %d %d", __FUNCTION__, GetRealPath().c_str(), name.c_str(), i, errno);
	if (i == -1)
		throw errno;

	return new InterixVFSNode(this, name);
}

void InterixVFSNode::setup()
{
	int i = fchdir(_realfd);
	if (i == -1)
		throw errno;
}

void InterixVFSNode::setup(const string& name, int e)
{
	if ((name == ".") || (name == "..") || name.empty())
		throw e;
	setup();
}

Ref<FD> InterixVFSNode::OpenFile(const string& name, int flags,	int mode)
{
	RAIILock locked;
	setup(name, EISDIR);

	//log("opening interix file <%s>", name.c_str());
	Ref<RawFD> ref = new RawFD();
	ref->Open(name, flags, mode);
	return (FD*) ref;
}

deque<string> InterixVFSNode::Enumerate()
{
	RAIILock locked;
	setup();

	deque<string> d;
	DIR* dir = opendir(".");
	for (;;)
	{
		struct dirent* de = readdir(dir);
		if (!de)
			break;
		d.push_back(de->d_name);
	}
	closedir(dir);

	return d;
}

string InterixVFSNode::ReadLink(const string& name)
{
	RAIILock locked;
	setup(name);

	char buffer[PATH_MAX];
	int i = readlink(name.c_str(), buffer, sizeof(buffer));
	if (i == -1)
		throw errno;

	return buffer;
}

void InterixVFSNode::MkDir(const string& name, int mode)
{
	RAIILock locked;
	setup(name);

	int i = mkdir(name.c_str(), mode);
	if (i == -1)
		throw errno;
}

void InterixVFSNode::RmDir(const string& name)
{
	RAIILock locked;
	setup(name);

	int i = rmdir(name.c_str());
	if (i == -1)
		throw errno;
}

int InterixVFSNode::Access(const string& name, int mode)
{
	RAIILock locked;
	setup(name);

	int i = access(name.c_str(), mode);
	if (i == -1)
		throw errno;
	return i;
}

void InterixVFSNode::Rename(const string& from, VFSNode* other, const string& to)
{
	InterixVFSNode* othernode = dynamic_cast<InterixVFSNode*>(other);
	assert(othernode);

	if ((from == ".") || (from == "..") || from.empty() ||
		(to == ".") || (to == "..") || to.empty())
		throw EINVAL;

	RAIILock locked;

	string toabs = othernode->GetRealPath() + "/" + to;

	setup();
	int i = rename(from.c_str(), toabs.c_str());
	if (i == -1)
		throw errno;
}

void InterixVFSNode::Chmod(const string& name, int mode)
{
	RAIILock locked;
	setup(name);

	int i = chmod(name.c_str(), mode);
	if (i == -1)
		throw errno;
}

void InterixVFSNode::Link(const string& from, VFSNode* other, const string& to)
{
	InterixVFSNode* othernode = dynamic_cast<InterixVFSNode*>(other);
	assert(othernode);

	if ((from == ".") || (from == "..") || from.empty() ||
		(to == ".") || (to == "..") || to.empty())
		throw EINVAL;

	RAIILock locked;

	string toabs = othernode->GetRealPath() + "/" + to;

	setup();
	int i = link(from.c_str(), toabs.c_str());
	if (i == -1)
		throw errno;
}

void InterixVFSNode::Unlink(const string& name)
{
	RAIILock locked;
	setup(name);

	int i = unlink(name.c_str());
	if (i == -1)
		throw errno;
}

void InterixVFSNode::Symlink(const string& name, const string& target)
{
	RAIILock locked;
	setup(name);

	int i = symlink(name.c_str(), target.c_str());
	if (i == -1)
		throw errno;
}


void InterixVFSNode::Utimes(const string& name, const struct timeval times[2])
{
	RAIILock locked;
	setup(name);

	/* Interix doesn't support times(), even though the docs say it does! */

	struct utimbuf ub;
	if (!times)
	{
		time(&ub.actime);
		ub.modtime = ub.actime;
	}
	else
	{
		ub.actime = times[0].tv_sec;
		ub.modtime = times[1].tv_sec;
	}

	int i = utime(name.c_str(), &ub);
	if (i == -1)
		throw errno;
}
