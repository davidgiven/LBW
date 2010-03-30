/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "Thread.h"
#include "filesystem/VFS.h"
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
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
		FakeRoot(false),
		Warnings(false)
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
		{
			static const char message[] =
				"lbw: [options...] <executable> [arguments...]\n"
				"LBW " VERSION " (C) 2010 David Given\n"
				"\n"
				"Options:\n"
				"  --fakeroot       Enable a crude fakeroot mode\n"
				"  --warnings       Show warnings for emulation problems\n"
				"  --chroot <path>  Set up a fake chroot for path\n"
				"  --forceload      Don't mmap() code, load it instead\n"
				"\n"
				"In order to run dynamic binaries, you must set up a chroot.\n"
				"\n"
				"This program is a technical proof of concept, and is not suitable for\n"
				"any purpose whatsoever. If you try to use it for real work you will\n"
				"regret it. Horribly. YOU HAVE BEEN WARNED.\n"
				;

			write(1, message, sizeof(message));
			exit(0);
		}
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
		else if (option == "--forceload")
		{
			ForceLoad = true;
			return 1;
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
	bool ForceLoad : 1;
};

int main(int argc, const char* argv[], const char* environ[])
{
	string linuxfile;

	/* Disable core files (as LBW has a tendency to crash and make them). */

	{
		struct rlimit rlimit = {0, 0};
		setrlimit(RLIMIT_CORE, &rlimit);
	}

#if 0
	for (int i=0; i<100; i++)
	{
		if (fcntl(i, F_GETFD, 0) != -1)
			log("fd %d is live", i);
	}
#endif

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

		Options.Warnings = !!getenv("LBW_FORCELOAD");
		unsetenv("LBW_FORCELOAD");

		const char* s = getenv("LBW_CHROOT");
		if (s)
		{
			Options.Chroot = s;
			VFS::SetRoot(Options.Chroot);
			VFS::SetCWD(NULL, "/");
		}
		unsetenv("LBW_CHROOT");

		s = getenv("LBW_CWD");
		if (s)
			VFS::SetCWD(NULL, s);
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
		if (access(Options.LBW.c_str(), X_OK) != 0)
		{
			string lbwexe = Options.LBW + ".exe";
			Options.LBW += ".exe";
			if (access(lbwexe.c_str(), X_OK) != 0)
				error("lbw: neither %s nor %s are executable",
						Options.LBW.c_str(), lbwexe.c_str());
			Options.LBW = lbwexe;
		}

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
		Options.ForceLoad = ap.ForceLoad;
		VFS::SetRoot(Options.Chroot);
		VFS::SetCWD(NULL, ap.CWD);

		if (argc == 0)
			Error("lbw: you must specify a binary to run. Try --help.");

		linuxfile = argv[0];
	}

	InstallExceptionHandler();

	//log("running elf file <%s>", linuxfile.c_str());
	RunElf(linuxfile, argv, environ);
	return 0;
}

