/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "exec/ElfLoader.h"
#include "syscalls/mmap.h"
#include "syscalls/memory.h"
#include "filesystem/FD.h"
#include "filesystem/InterixVFSNode.h"
#include "filesystem/VFS.h"
#include "MemOp.h"
#include <pthread.h>
#include <vector>

using std::vector;

static pthread_mutex_t lock;
static ElfLoader* executable = NULL;
static ElfLoader* interpreter = NULL;

/* Used when a new process is started; initialises everything that needs to
 * be done anew for any process. (Such as pthread mutexes and GS.)
 */
void InitProcess()
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

	int i = pthread_mutex_init(&lock, &attr);
	if (i)
		error("Failed to init process lock: %d!", i);

	pthread_mutexattr_destroy(&attr);
	InitGSStore();
}

/* Used once we've committed to loading a new executable. Deinits all
 * resources that aren't going to be carried over to the new
 * executable. In particular: memory mappings, file descriptors.
 */
void FlushExecutable()
{
	UnmapAll();
	ClearBrk();
	FD::Flush();
}

void Lock()
{
	pthread_mutex_lock(&lock);
}

void Unlock()
{
	pthread_mutex_unlock(&lock);
}

void RunElf(const string& pathname, const char* argv[], const char* environ[])
{
	int argvsize;
	int envsize;

	void* entrypoint;
	executable = new ElfLoader();
	interpreter = NULL;

	executable->Open(pathname);

	if (executable->HasInterpreter())
	{
		interpreter = new ElfLoader();
		interpreter->Open(executable->GetInterpreter());
	}

	/* Count the size of the argument and environment lists. */

	argvsize = 0;
	while (argv[argvsize])
		argvsize++;

	envsize = 0;
	while (environ[envsize])
		envsize++;

	/* Load the executable and interpreter. */

	if (interpreter)
	{
		interpreter->Load();
		executable->Load();
		entrypoint = interpreter->GetEntrypoint();
	}
	else
	{
		executable->Load();
		entrypoint = executable->GetEntrypoint();
	}

	InitProcess();

	int auxsize = 7*2 + 1;

	/* Initialise the stack. */

	int arraysize = argvsize + 1 + envsize + 1 + auxsize;
	const char* calldata[arraysize];

	int index = 0;
	for (int i = 0; i < argvsize; i++)
		calldata[index++] = argv[i];
	calldata[index++] = NULL;
	for (int i = 0; i < envsize; i++)
		calldata[index++] = environ[i];
	calldata[index++] = NULL;

	calldata[index++] = (const char*) AT_PHDR;
	calldata[index++] = (const char*) &executable->GetProgramHeader(0);
	calldata[index++] = (const char*) AT_PHENT;
	calldata[index++] = (const char*) executable->GetProgramHeaderSize();
	calldata[index++] = (const char*) AT_PHNUM;
	calldata[index++] = (const char*) executable->GetNumProgramHeaders();
	calldata[index++] = (const char*) AT_ENTRY;
	calldata[index++] = (const char*) executable->GetEntrypoint();
	calldata[index++] = (const char*) AT_PAGESZ;
	calldata[index++] = (const char*) 0x1000;
	calldata[index++] = (const char*) AT_BASE;
	calldata[index++] = (const char*) (interpreter ? interpreter->GetLoadAddress() : NULL);
	calldata[index++] = (const char*) AT_FLAGS;
	calldata[index++] = (const char*) 0;
	calldata[index++] = (const char*) AT_NULL;

	asm volatile (
		"mov %1, %%esp; " // set stack to startup data
		"push %0; " // argc
		"push %2; " //routine to call
		"xor %%eax, %%eax; "
		"mov %%eax, %%ebx; "
		"mov %%eax, %%ecx; "
		"mov %%eax, %%edx; "
		"mov %%eax, %%esi; "
		"mov %%eax, %%edi; "
		"mov %%eax, %%ebp; "
		"ret"
		:
		: "r" (argvsize),
		  "r" (calldata),
		  "r" (entrypoint)
		);
	_exit(0);
}

