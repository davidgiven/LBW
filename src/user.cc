#include "globals.h"
#include "elfloader/ElfLoader.h"
#include <pthread.h>

static pthread_mutex_t lock;
static ElfLoader* executable = NULL;
static ElfLoader* interpreter = NULL;

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

void Lock()
{
	pthread_mutex_lock(&lock);
}

void Unlock()
{
	pthread_mutex_unlock(&lock);
}

int Exec(int argc, const char* argv[], const char* environ[])
{
	log("opening %s", argv[0]);

	void* entrypoint;
	ElfLoader* newExecutable = new ElfLoader();
	ElfLoader* newInterpreter = NULL;
	try
	{
		newExecutable->Open(argv[0]);

		if (newExecutable->HasInterpreter())
		{
			newInterpreter = new ElfLoader();
			newInterpreter->Open(newExecutable->GetInterpreter());
		}

		/* If the open succeeded, flush the old executable and commit to
		 * using the new one.
		 */

		if (executable)
		{
			executable->Close();
			delete executable;
			interpreter->Close();
			delete interpreter;
		}
		executable = newExecutable;
		newExecutable = NULL;
		interpreter = newInterpreter;
		newInterpreter = NULL;

		if (interpreter)
		{
			interpreter->Load();
			entrypoint = interpreter->GetEntrypoint();
		}
		else
		{
			executable->Load();
			entrypoint = executable->GetEntrypoint();
		}
	}
	catch (int e)
	{
		error("could not load executable: %d", e);
	}
	delete newExecutable;

	InitProcess();

	int auxsize = 5*2 + 1;

	int envsize = 0;
	while (environ[envsize])
		envsize++;

	int argvsize = argc;
	int arraysize = argvsize + 1 + envsize + 1 + auxsize;
	const char* calldata[arraysize];

	int index = 0;
	for (int i = 0; i < argvsize+1; i++)
		calldata[index++] = argv[i];
	for (int i = 0; i < envsize+1; i++)
		calldata[index++] = environ[i];

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
	calldata[index++] = (const char*) AT_NULL;

	log("running code now");
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
		: "r" (argc),
		  "r" (calldata),
		  "r" (entrypoint)
		);
	_exit(0);
}

