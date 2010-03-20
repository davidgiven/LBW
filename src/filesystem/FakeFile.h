/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef FAKEFILE_H
#define FAKEFILE_H

#include "VFSNode.h"
#include <map>

using std::map;

class FakeFile : public HasRefCount
{
public:
	virtual ~FakeFile();

public:
	virtual string GetName();
	virtual Ref<VFSNode> OpenDirectory(VFSNode* parent);
	virtual Ref<FD> OpenFile(int flags, int mode);
	virtual void Stat(struct stat& st);
	virtual int Access(int mode);
};

class TunnelledFakeFileOrDirectory : public FakeFile
{
public:
	TunnelledFakeFileOrDirectory(const string& localname, const string& destname);

public:
	string GetName();
	void Stat(struct stat& st);
	int Access(int mode);

protected:
	string _localname;
	string _destname;
};

class TunnelledFakeFile : public TunnelledFakeFileOrDirectory
{
public:
	TunnelledFakeFile(const string& localname, const string& destname);

public:
	Ref<FD> OpenFile(int flags, int mode);
};

class TunnelledFakeDirectory : public TunnelledFakeFileOrDirectory
{
public:
	TunnelledFakeDirectory(const string& localname, const string& destname);

public:
	Ref<VFSNode> OpenDirectory(VFSNode* parent);
};

#endif
