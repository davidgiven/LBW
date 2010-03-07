#ifndef SYSCALLS_H
#define SYSCALLS_H

union Argument
{
	int32_t s;
	u_int32_t u;
	void* p;
};

struct Arguments
{
	int32_t syscall;
	Argument a0;
	Argument a1;
	Argument a2;
	Argument a3;
	Argument a4;
	Argument a5;
};

struct Registers
{
	union
	{
		Arguments arg;
		struct
		{
			Argument eax;
			Argument ebx;
			Argument ecx;
			Argument edx;
			Argument esi;
			Argument edi;
			Argument ebp;
		};
	};
	u_int32_t flags;
	u_int32_t gs;
	u_int32_t eip;
};

#define SYSCALL(__name) \
	int32_t __name(const Arguments& arg)
typedef int32_t Syscall(const Arguments& arg);

#define NUM_SYSCALLS 337

extern const char* const SyscallNames[NUM_SYSCALLS];

#endif
