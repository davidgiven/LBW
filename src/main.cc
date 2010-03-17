/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "Thread.h"
#include "filesystem/VFS.h"
#include "filesystem/RawFD.h"
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

typeof(Options) Options;

static const char* const environment[] =
{
	NULL
};

void log(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);

	string s = cprintf("lbw(%lu): ", getpid());
	s += vcprintf(format, ap);
	s += "\n";

	write(2, s.data(), s.size());

	va_end(ap);
}

void Warning(const char* format, ...)
{
	if (Options.Warnings)
	{
		va_list ap;
		va_start(ap, format);

		string s = cprintf("lbw(%lu) warning: ", getpid());
		s += vcprintf(format, ap);
		s += "\n";

		write(2, s.data(), s.size());

		va_end(ap);
	}
}

void error(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);

	string s = cprintf("lbw(%lu) error: ", getpid());
	s += vcprintf(format, ap);
	s += "\n";

	write(2, s.data(), s.size());

	va_end(ap);
	_exit(-1);
}

void Error(const string& message)
{
	writef(2, "lbw error: %s\n", message.c_str());
	_exit(-1);
}

class ArgumentParser
{
public:
	ArgumentParser():
		Chroot("/"),
		FakeRoot(false)
	{
		char buffer[PATH_MAX];
		getcwd(buffer, sizeof(buffer));
		CWD = buffer;
	}

	void Parse(int& argc, const char**& argv)
	{
		argc--;
		argv++;

		while (*argv)
		{
			string option = *argv;
			if (option.empty())
				BadOption();

			if (option[0] != '-')
				break;

			string argument;
			if (argv[1])
				argument = argv[1];

			int delta = Option(option, argument);

			argc -= delta;
			argv += delta;
		}
	}

	void BadOption()
	{
		Error("lbw: invalid syntax. Try --help");
	}

	int Option(const string& option, const string& argument)
	{
		if (option == "--help")
			Error("lbw: no help yet. Sorry.");
		else if (option == "--fakeroot")
		{
			FakeRoot = true;
			return 1;
		}
		else if (option == "--warnings")
		{
			Warnings = true;
			return 1;
		}
		else if (option == "--chroot")
		{
			Chroot = argument;
			CWD = "/";
			return 2;
		}
		else
			BadOption();
		return 1;
	}


public:
	string Chroot;
	string CWD;
	bool FakeRoot : 1;
	bool Warnings : 1;
};

static void add_std_fd(int fd)
{
	/* This is a very crude way of determining whether the file descriptor is
	 * open. */

#if 0
	{
		fd_set set;
		FD_ZERO(&set);
		FD_SET(fd, &set);

		struct timeval tv = {0, 0};

		if (select(fd+1, &set, NULL, NULL, &tv) == -1)
			return;
	}
#else
	if (fcntl(fd, F_GETFL, 0) == -1)
		return;
#endif

	//log("registering fd %d", fd);
	Ref<RawFD> fdo = new RawFD();
	fdo->Reference(); // never allow these objects to be destroyed
	fdo->Open(fd);
	FD::New(fdo, fd);
}

int main(int argc, const char* argv[], const char* environ[])
{
	string linuxfile;

	/* Register file descriptors. We have to do this first to avoid
	 * problems caused by our own file descriptors.
	 */

	for (int i = 0; i < 10; i++)
		add_std_fd(i);

	if (getenv("LBW_CHILD"))
	{
		unsetenv("LBW_CHILD");

		/* This is a child LBW invoked via LBW's own exec, so use the magic
		 * environment-based options parsing.
		 */

		Options.FakeRoot = !!getenv("LBW_FAKEROOT");
		unsetenv("LBW_FAKEROOT");

		Options.Warnings = !!getenv("LBW_WARNINGS");
		unsetenv("LBW_WARNINGS");

		const char* s = getenv("LBW_CHROOT");
		if (s)
		{
			Options.Chroot = s;
			VFS::SetRoot(Options.Chroot);
		}
		unsetenv("LBW_CHROOT");

		s = getenv("LBW_CWD");
		if (s)
			VFS::SetCWD(s);
		unsetenv("LBW_CWD");

		s = getenv("LBW_LBWEXE");
		if (!s)
			error("lbw: unable to locate LBW executable");
		Options.LBW = s;
		unsetenv("LBW_LBWEXE");

		s = getenv("LBW_LINUXEXE");
		if (!s)
			error("lbw: unable to locate Linux executable");
		linuxfile = s;
		unsetenv("LBW_LINUXEXE");
	}
	else
	{
		/* Find the real location of LBW. */

		char buffer[PATH_MAX];
		char* s = realpath(argv[0], buffer);
		if (!s)
			error("lbw: unable to locate LBW executable");
		Options.LBW = buffer;

		/* Parse human arguments. */

		ArgumentParser ap;
		ap.Parse(argc, argv);

		/* Find the real location of the chroot. */

		s = realpath(ap.Chroot.c_str(), buffer);
		if (!s)
			error("lbw: unable to locate chroot");

		Options.Chroot = buffer;
		Options.FakeRoot = ap.FakeRoot;
		Options.Warnings = ap.Warnings;
		VFS::SetRoot(Options.Chroot);
		VFS::SetCWD(ap.CWD);

		if (argc == 0)
			Error("lbw: you must specify a binary to run. Try --help.");

		linuxfile = argv[0];
	}

	InstallExceptionHandler();

	log("running elf file <%s>", linuxfile.c_str());
	RunElf(linuxfile, argv, environ);
	return 0;
}

