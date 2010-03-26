/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "exec/ElfLoader.h"
#include "exec/exec.h"
#include "filesystem/FD.h"
#include "filesystem/VFS.h"
#include "filesystem/InterixVFSNode.h"

enum
{
	UNKNOWN_EXECUTABLE,
	ELF_EXECUTABLE,
	INTERIX_EXECUTABLE,
	SHELL_EXECUTABLE
};

static int probe_executable(const string& filename)
{
	Ref<FD> ref = VFS::OpenFile(NULL, filename);
	int fd = ref->GetFD();

	char buffer[4];
	if (pread(fd, buffer, sizeof(buffer), 0) == -1)
		return UNKNOWN_EXECUTABLE;

	if (memcmp(buffer, "\x7F" "ELF", 4) == 0)
		return ELF_EXECUTABLE;
	if (memcmp(buffer, "MZ", 2) == 0)
		return INTERIX_EXECUTABLE;
	if (memcmp(buffer, "#!", 2) == 0)
		return SHELL_EXECUTABLE;
	return UNKNOWN_EXECUTABLE;
}

static void linux_exec(const string& pathname, const char* argv[], const char* environ[])
{
	/* Count options. */

	int envc = 1;
	while (environ[envc])
		envc++;

	const char* newenviron[envc + 8];
	memset(newenviron, 0, sizeof(newenviron));

	int index = 0;

	/* Set LBW-specific options. */

	newenviron[index++] = "LBW_CHILD=1";
	if (Options.FakeRoot)
		newenviron[index++] = "LBW_FAKEROOT=1";
	if (Options.Warnings)
		newenviron[index++] = "LBW_WARNINGS=1";
	if (Options.ForceLoad)
		newenviron[index++] = "LBW_FORCELOAD=1";

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

static bool isterm(int c)
{
	return (c == '\0') || (c == '\n') || (c == '\r');
}

static bool issep(int c)
{
	return isterm(c) || (c == ' ') || (c == '\t');
}

static void shell_exec(const string& pathname, const char* argv[], const char* environ[])
{
	Ref<FD> ref = VFS::OpenFile(NULL, pathname);
	int fd = ref->GetFD();

	/* We know this will work, because we've just tried it when probing the
	 * file.
	 */

	char buffer[128];
	if (pread(fd, buffer, sizeof(buffer), 0) == -1)
		throw EIO;
	buffer[sizeof(buffer)-1] = '\0';

	/* Messy code to parse the #! line. */

	char* interpreter_start = buffer + 2;
	while (issep(*interpreter_start))
		interpreter_start++;
	if (!*interpreter_start)
		throw EINVAL;

	char* interpreter_end = interpreter_start + 1;
	while (!issep(*interpreter_end))
		interpreter_end++;

	char* arg_start = NULL;
	if (!isterm(*interpreter_end))
	{
		arg_start = interpreter_end + 1;

		while (issep(*arg_start) && !isterm(*arg_start))
			arg_start++;

		if (isterm(*arg_start))
			arg_start = NULL;
		else
		{
			char* arg_end = arg_start + 1;

			while (!issep(*arg_end))
				arg_end++;
			*arg_end = '\0';
		}
	}

	*interpreter_end = '\0';

	/* Create a new argument array with this new arguments on the front. */

	int argc = 0;
	while (argv[argc])
		argc++;

	const char* newargv[argc+3];
	int index = 0;
	newargv[index++] = interpreter_start;
	if (arg_start)
		newargv[index++] = arg_start;
	memcpy(newargv+index, argv, (argc+1) * sizeof(char*));
	newargv[index] = pathname.c_str();

	/* Retry the exec. */

	Exec(newargv[0], newargv, environ);
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
			chdir(inode->GetRealPath().c_str());
		else
			chdir("/");

		/* Now actually perform the exec. */

		//log("exec <%s>", pathname.c_str());
		switch (type)
		{
			case ELF_EXECUTABLE:
				linux_exec(pathname, argv, environ);
				break;

			case INTERIX_EXECUTABLE:
				execve(pathname.c_str(), (char* const*) argv, (char* const*) environ);
				break;

			case SHELL_EXECUTABLE:
				shell_exec(pathname, argv, environ);
		}

		throw errno;
	}
	catch (int e)
	{
		error("Failed to exec with errno %d!", e);
	}
}
