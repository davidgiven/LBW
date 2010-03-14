/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "syscalls.h"

#define LINUX_FUTEX_WAIT              0
#define LINUX_FUTEX_WAKE              1
#define LINUX_FUTEX_FD                2
#define LINUX_FUTEX_REQUEUE           3
#define LINUX_FUTEX_CMP_REQUEUE       4
#define LINUX_FUTEX_WAKE_OP           5
#define LINUX_FUTEX_LOCK_PI           6
#define LINUX_FUTEX_UNLOCK_PI         7
#define LINUX_FUTEX_TRYLOCK_PI        8
#define LINUX_FUTEX_WAIT_BITSET       9
#define LINUX_FUTEX_WAKE_BITSET       10

#define LINUX_FUTEX_PRIVATE_FLAG      128
#define LINUX_FUTEX_CLOCK_REALTIME	256
#define LINUX_FUTEX_CMD_MASK		~(LINUX_FUTEX_PRIVATE_FLAG | LINUX_FUTEX_CLOCK_REALTIME)

#define LINUX_FUTEX_WAIT_PRIVATE      (LINUX_FUTEX_WAIT | LINUX_FUTEX_PRIVATE_FLAG)
#define LINUX_FUTEX_WAKE_PRIVATE      (LINUX_FUTEX_WAKE | LINUX_FUTEX_PRIVATE_FLAG)
#define LINUX_FUTEX_REQUEUE_PRIVATE   (LINUX_FUTEX_REQUEUE | LINUX_FUTEX_PRIVATE_FLAG)
#define LINUX_FUTEX_CMP_REQUEUE_PRIVATE (LINUX_FUTEX_CMP_REQUEUE | LINUX_FUTEX_PRIVATE_FLAG)
#define LINUX_FUTEX_WAKE_OP_PRIVATE   (LINUX_FUTEX_WAKE_OP | LINUX_FUTEX_PRIVATE_FLAG)
#define LINUX_FUTEX_LOCK_PI_PRIVATE   (LINUX_FUTEX_LOCK_PI | LINUX_FUTEX_PRIVATE_FLAG)
#define LINUX_FUTEX_UNLOCK_PI_PRIVATE (LINUX_FUTEX_UNLOCK_PI | LINUX_FUTEX_PRIVATE_FLAG)
#define LINUX_FUTEX_TRYLOCK_PI_PRIVATE (LINUX_FUTEX_TRYLOCK_PI | LINUX_FUTEX_PRIVATE_FLAG)
#define LINUX_FUTEX_WAIT_BITSET_PRIVATE       (LINUX_FUTEX_WAIT_BITS | LINUX_FUTEX_PRIVATE_FLAG)
#define LINUX_FUTEX_WAKE_BITSET_PRIVATE       (LINUX_FUTEX_WAKE_BITS | LINUX_FUTEX_PRIVATE_FLAG)

struct linux_user_desc {
	int  entry_number;
	unsigned int  base_addr;
	unsigned int  limit;
	unsigned int  seg_32bit:1;
	unsigned int  contents:2;
	unsigned int  read_exec_only:1;
	unsigned int  limit_in_pages:1;
	unsigned int  seg_not_present:1;
	unsigned int  useable:1;
};

int32_t sys_set_thread_area(Registers& regs)
{
	Arguments& arg = regs.arg;
	struct linux_user_desc& u = *(struct linux_user_desc*) arg.a0.p;

	assert(u.entry_number == -1);
	u.entry_number = 10;

	u_int16_t gs = (u.entry_number << 3) | 7;
	//log("GS %04x now pointing at linear %08x", gs, u.base_addr);
	SetGS(gs, (void*) u.base_addr);

	return 0;
}

/* Not implemented --- nop */
SYSCALL(sys_set_tid_address)
{
	Warning("sys_set_tid_address() currently unimplemented");
	return getpid();
}

/* Not implemented */
SYSCALL(compat_sys_set_robust_list)
{
	Warning("compat_sys_set_robust_list() currently unimplemented");
	return 0; //-LINUX_ENOSYS;
}

SYSCALL(compat_sys_futex)
{
	int* uaddr = (int*) arg.a0.p;
	int op = arg.a1.s;
	int val = arg.a2.s;
	const void* timespec = (const void*) arg.a3.p;
	int* uaddr2 = (int*) arg.a4.p;
	int val3 = arg.a5.s;

	int cmd = op & LINUX_FUTEX_CMD_MASK;
	switch (cmd)
	{
		case LINUX_FUTEX_WAIT:
			*uaddr = 0;
			log("FUTEX_WAIT faked");
			return 0;

		case LINUX_FUTEX_WAKE:
			return 0;

		case LINUX_FUTEX_WAIT_BITSET:
			throw EINVAL;

		default:
			log("op=%02x uaddr=%08x, *uaddr=%08x, val=%08x, timespec=%p, uaddr2=%08x, val3=%08x", op, uaddr, *uaddr, val,
					timespec, uaddr2, val3);

			error("compat_sys_futex(%x) currently unimplemented", op);
	}

	//sleep(-1);
	throw ENOSYS;
}

SYSCALL(sys_gettid)
{
	return (u32) pthread_self();
}
