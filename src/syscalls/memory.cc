/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "syscalls.h"
#include <sys/mman.h>
#include <pthread.h>

#define BRK_SIZE (1024*1024)

static u_int8_t* brkbuf = NULL;
static u_int8_t* pos;

void ClearBrk()
{
	pos = brkbuf;
}

SYSCALL(sys_brk)
{
	RAIILock locked;
	u_int32_t addr = arg.a0.u;

	if (!brkbuf)
	{
		void* result = mmap(NULL, BRK_SIZE,
				PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS,
				-1, 0);
		//log("brk block mapped to %08x+%08x", result, BRK_SIZE);
		if (brkbuf == MAP_FAILED)
			return -LINUX_ENOMEM;

		brkbuf = pos = (u_int8_t*) result;
	}

	if (addr)
	{
		//log("brk was %08x, app asked for %08x", pos, addr);
		pos = (u_int8_t*) addr;

		u_int8_t* top = brkbuf + BRK_SIZE;
		if (pos > top)
			pos = top;
		if (pos < brkbuf)
			pos = brkbuf;
		//log("brk now %08x", pos);
	}
	return (u_int32_t) pos;
}
