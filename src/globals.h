#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include <string>

#include "linux_errno.h"
#include "Ref.h"

using std::string;

class FD;

#include "Exception.h"

#define PACKED  __attribute__((packed)) __attribute__((aligned(1)))

typedef u_int16_t __u16;
typedef int16_t __s16;
typedef u_int32_t __u32;
typedef int32_t __s32;
typedef u_int64_t __u64;
typedef int64_t __s64;

typedef u_int8_t u8;
typedef int8_t s8;
typedef u_int16_t u16;
typedef int16_t s16;
typedef u_int32_t u32;
typedef int32_t s32;
typedef u_int64_t u64;
typedef int64_t s64;

struct Options_s
{
	bool FakeRoot : 1;
};

extern Options_s Options;

static inline u_int32_t MemoryRoundDown(u_int32_t addr)
{
	return addr & ~0xffff;
}

static inline u_int32_t MemoryRoundUp(u_int32_t addr)
{
	return (addr + 0xffff) & ~0xffff;
}

extern void vwritef(int fd, const char* format, va_list ap);
extern void writef(int fd, const char* format, ...);
extern string cprintf(const char* format, ...);

extern void log(const char* format, ...);
extern void Warning(const char* format, ...);
extern void error(const char* format, ...)
	__attribute__ ((noreturn));
extern void Error(const string& message)
	__attribute__ ((noreturn));

struct ElfInfo
{
	void* entrypoint;
	void* phdr;
	u32 phent;
	u32 phnum;
};

extern int LoadElf(int fd, ElfInfo& info);

extern int SysError(int e);
extern int ErrnoI2L(int e);

extern void StartMonitor(void);

extern void DumpMemory(u_int32_t address, u_int32_t len);
extern void DumpMemory(const void* address, u_int32_t len);
extern void DumpMap(pid_t pid);

extern string StringF(const char* format, ...);

/* User space */

extern void InitProcess();
extern void Lock();
extern void Unlock();
extern int Exec(int argc, const char* argv[], const char* environ[]);

class RAIILock
{
public:
	RAIILock() { Lock(); }
	~RAIILock() { Unlock(); }
};

/* GS handling */

extern void InitGSStore();
extern void SetGS(u_int16_t gs, void* linear);

/* Machine code entry points */

typedef void MCE(void);

extern "C" void Linux_MCE(void);
extern "C" void InterpretGS_MCE(void);

#endif
