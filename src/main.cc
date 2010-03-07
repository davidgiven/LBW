#include "globals.h"
#include "Thread.h"
#include "filesystem/VFS.h"
#include "filesystem/RawFD.h"
#include <stdarg.h>
#include <pthread.h>
#include <semaphore.h>

typeof(Options) Options;

static const char* const environment[] =
{
	NULL
};

void log(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);

	writef(2, "lbw(%lu): ", getpid());
	vwritef(2, format, ap);
	writef(2, "\n");

	va_end(ap);
}

void Warning(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);

	writef(2, "lbw(%lu) warning: ", getpid());
	vwritef(2, format, ap);
	writef(2, "\n");

	va_end(ap);
}

void error(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);

	writef(2, "lbw(%lu) error: ", getpid());
	vwritef(2, format, ap);
	writef(2, "\n");

	va_end(ap);
	_exit(-1);
}

void Error(const string& message)
{
	writef(2, "lbw error: %s\n", message.c_str());
	_exit(-1);
}

#if 0
static int init_fifo(void)
{
	const char* homedir = getenv("HOME");
	char buffer[PATH_MAX];

	snprintf(buffer, sizeof(buffer),
			"%s/.lbw", homedir);
	mkdir(buffer, S_IRWXU); /* ignore errors */

	snprintf(buffer, sizeof(buffer),
			"%s/.lbw/%d", homedir, slavepid);
	int e = mknod(buffer, S_IFIFO | S_IRWXU, 0);
	if (e)
		return e;

	slavefd = open(buffer, O_RDWR);
	if (slavefd == -1)
		return -1;
	return 0;
}
#endif

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
};

static void add_std_fd(int fd)
{
	Ref<RawFD> fdo = new RawFD();
	fdo->Reference(); // never allow these objects to be destroyed
	fdo->Open(fd);
	assert(FD::New(fdo) == fd);
}

int main(int argc, const char* argv[], const char* environ[])
{
	{
		ArgumentParser* ap = new ArgumentParser();
		ap->Parse(argc, argv);

		Options.FakeRoot = ap->FakeRoot;
		VFS::SetRoot(ap->Chroot);
		VFS::SetCWD(ap->CWD);

		delete ap;
	}

	if (argc == 0)
		Error("lbw: you must specify a binary to run. Try --help.");

	add_std_fd(0);
	add_std_fd(1);
	add_std_fd(2);

	StartMonitor();
	Exec(argc, argv, environ);
	return 0;
}

