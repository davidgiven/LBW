#include "globals.h"
#include "syscalls.h"
#include <unistd.h>

SYSCALL(sys32_execve)
{
	const char* path = (const char*) arg.a0.p;
	const char** argv = (const char**) arg.a1.p;
	const char** envp = (const char**) arg.a2.p;

	Exec(path, argv, envp);
}
