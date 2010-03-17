/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "exec/ElfLoader.h"
#include "filesystem/FD.h"
#include "filesystem/VFS.h"
#include "filesystem/InterixVFSNode.h"

enum
{
	UNKNOWN_EXECUTABLE,
	ELF_EXECUTABLE,
	INTERIX_EXECUTABLE
};

static int probe_executable(const string& filename)
{
	Ref<FD> ref = VFS::OpenFile(NULL, filename);
	int fd = ref->GetRealFD();

	char buffer[4];
	if (pread(fd, buffer, sizeof(buffer), 0) == -1)
		return UNKNOWN_EXECUTABLE;

	if (memcmp(buffer, "\x7F" "ELF", 4) == 0)
		return ELF_EXECUTABLE;
	if (memcmp(buffer, "MZ", 2) == 0)
		return INTERIX_EXECUTABLE;
	return UNKNOWN_EXECUTABLE;
}

static void linux_exec(const string& pathname, const char* argv[], const char* environ[])
{
	/* Count options. */

	int envc = 1;
	while (environ[envc])
		envc++;

	const char* newenviron[envc + 7];
	memset(newenviron, 0, sizeof(newenviron));

	int index = 0;

	/* Set LBW-specific options. */

	newenviron[index++] = "LBW_CHILD=1";
	if (Options.FakeRoot)
		newenviron[index++] = "LBW_FAKEROOT=1";
	if (Options.Warnings)
		newenviron[index++] = "LBW_WARNINGS=1";

	if (!Options.Chroot.empty())
	{
		string s = cprintf("LBW_CHROOT=%s", Options.Chroot.c_str());
		newenviron[index++] = strdup(s.c_str());
	}

	{
		string s = cprintf("LBW_CWD=%s", VFS::GetCWD().c_str());
		newenviron[index++] = strdup(s.c_str());
	}

	{
		string s = cprintf("LBW_LBWEXE=%s", Options.LBW.c_str());
		newenviron[index++] = strdup(s.c_str());
	}

	{
		string s = cprintf("LBW_LINUXEXE=%s", pathname.c_str());
		newenviron[index++] = strdup(s.c_str());
	}

	memcpy(newenviron + index, environ, envc * sizeof(char*));

	/* Invoke child lwb instance. */

	//log("exec <%s> into pid %d", pathname.c_str(), getpid());
	execve(Options.LBW.c_str(), (char* const*) argv, (char* const*) newenviron);
}

void Exec(const string& pathname, const char* argv[], const char* environ[])
{
	//log("opening %s", argv[0]);

	int type = probe_executable(pathname);
	if (type == UNKNOWN_EXECUTABLE)
		throw ENOEXEC;

	/* Beyond this point we cannot recover. */

	try
	{
		/* Change the Interix directory. */

		Ref<VFSNode> node = VFS::GetCWDNode();
		InterixVFSNode* inode = dynamic_cast<InterixVFSNode*>((VFSNode*) node);
		if (inode)
			fchdir(inode->GetRealFD());
		else
			chdir("/");

		/* Adjust the fd map so that Interix sees the same mappings that Linux
		 * is.
		 */

		FD::Flush(); // free cloexec file descriptors
		map<int, int> fdmap = FD::GetFDMap();

		map<int, int>::const_iterator i = fdmap.begin();
		while (i != fdmap.end())
		{
			//log("linuxfd %d maps to realfd %d", i->first, i->second);
			if (i->first != i->second)
			{
				if (fdmap.find(i->second) != fdmap.end())
				{
					int newfd = dup(i->first);
					//log("copying realfd %d -> %d", i->first, newfd);
					fdmap[i->second] = newfd;
				}

				//log("copying realfd %d -> %d", i->second, i->first);
				int fd = dup2(i->second, i->first);
				assert(fd != -1);
			}
			i++;
		}

		/* Now actually perform the exec. */

		switch (type)
		{
			case ELF_EXECUTABLE:
				linux_exec(pathname, argv, environ);
				break;

			case INTERIX_EXECUTABLE:
				execve(pathname.c_str(), (char* const*) argv, (char* const*) environ);
				break;
		}

		throw errno;
	}
	catch (int e)
	{
		error("Failed to exec with errno %d!", e);
	}
}
