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
#define LINUX_FUTEX_CMD_MASK          ~LINUX_FUTEX_PRIVATE_FLAG

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

static pthread_mutex_t tlsmutex = PTHREAD_MUTEX_INITIALIZER;

typedef u_int32_t DWORD;
typedef u_int32_t ULONG;
typedef u_int16_t WORD;
typedef u_int8_t BYTE;

#pragma pack(push)
struct LDT_ENTRY {
		WORD LimitLow;
		WORD BaseLow;
		union {
				struct {
						BYTE BaseMid;
						BYTE Flags1;
						BYTE Flags2;
						BYTE BaseHi;
				} Bytes;
				struct {
						DWORD BaseMid:8;
						DWORD Type:5;
						DWORD Dpl:2;
						DWORD Pres:1;
						DWORD LimitHi:4;
						DWORD Sys:1;
						DWORD Reserved_0:1;
						DWORD Default_Big:1;
						DWORD Granularity:1;
						DWORD BaseHi:8;
				} Bits;
		} HighWord;
};
#pragma pack(pop)

extern "C" u_int32_t __stdcall NtSetLdtEntries(
	/*IN*/ ULONG  Selector1,
	/*IN*/ LDT_ENTRY  LdtEntry1,
	/*IN*/ ULONG  Selector2,
	/*IN*/ LDT_ENTRY  LdtEntry2);
asm ("_NtSetLdtEntries: jmp _NtSetLdtEntries@24");

extern "C" u_int32_t __stdcall NtQueryInformationProcess(
  /*IN*/ int32_t ProcessHandle,
  /*IN*/ int32_t ProcessInformationClass,
  /*OUT*/ void* ProcessInformation,
  /*IN*/ u_int32_t ProcessInformationLength,
  /*OUT*/ u_int32_t* ReturnLength  /*OPTIONAL*/);
asm ("_NtQueryInformationProcess: jmp _NtQueryInformationProcess@20");

extern "C" u_int32_t __stdcall NtSetInformationProcess(
  /*IN*/ int32_t ProcessHandle,
  /*IN*/ int32_t ProcessInformationClass,
  /*IN*/ void* ProcessInformation,
  /*IN*/ u_int32_t ProcessInformationLength);
asm ("_NtSetInformationProcess: jmp _NtSetInformationProcess@16");

static u_int32_t read_ldt(u_int32_t selector, LDT_ENTRY& ldt)
{
	u_int32_t buffer[4];
	buffer[0] = selector << 3;
	buffer[1] = sizeof(ldt);

	u_int32_t result = NtQueryInformationProcess(-1, 10, buffer, 16, NULL);
	if (buffer[1] == 0)
		ldt.HighWord.Bits.Pres = 0;
	else
		memcpy(&ldt, &buffer[2], sizeof(ldt));
	return result;
}

static u_int32_t write_ldt(u_int32_t selector, const LDT_ENTRY& ldt)
{
	u_int32_t buffer[4];
	buffer[0] = selector << 3;
	buffer[1] = sizeof(ldt);
	memcpy(&buffer[2], &ldt, sizeof(ldt));

	return NtSetInformationProcess(-1, 10, buffer, 16);
}

static int
display_selector (LDT_ENTRY& info)
{
      int base, limit;
      if (!info.HighWord.Bits.Pres)
	{
	  log ("Segment not present\n");
	  return 0;
	}
      base = (info.HighWord.Bits.BaseHi << 24) +
	     (info.HighWord.Bits.BaseMid << 16)
	     + info.BaseLow;
      limit = (info.HighWord.Bits.LimitHi << 16) + info.LimitLow;
      if (info.HighWord.Bits.Granularity)
	limit = (limit << 12) | 0xfff;
      log ("base=0x%08x limit=0x%08x", base, limit);
      if (info.HighWord.Bits.Default_Big)
	log(" 32-bit ");
      else
	log(" 16-bit ");
      switch ((info.HighWord.Bits.Type & 0xf) >> 1)
	{
	case 0:
	  log ("Data (Read-Only, Exp-up");
	  break;
	case 1:
	  log ("Data (Read/Write, Exp-up");
	  break;
	case 2:
	  log ("Unused segment (");
	  break;
	case 3:
	  log ("Data (Read/Write, Exp-down");
	  break;
	case 4:
	  log ("Code (Exec-Only, N.Conf");
	  break;
	case 5:
	  log ("Code (Exec/Read, N.Conf");
	  break;
	case 6:
	  log ("Code (Exec-Only, Conf");
	  break;
	case 7:
	  log ("Code (Exec/Read, Conf");
	  break;
	default:
	  log ("Unknown type 0x%x",info.HighWord.Bits.Type);
	}
      if ((info.HighWord.Bits.Type & 0x1) == 0)
	log(", N.Acc");
      log (")\n");
      if ((info.HighWord.Bits.Type & 0x10) == 0)
	log("System selector ");
      log ("Priviledge level = %d. ", info.HighWord.Bits.Dpl);
      if (info.HighWord.Bits.Granularity)
	log ("Page granular.\n");
      else
	log ("Byte granular.\n");
      return 1;
}

static bool try_perversion(Registers& regs, int offset)
{
	u_int16_t& insn = *(u_int16_t*)(regs.eip+offset);
	if (insn == 0xe88e)
	{
		insn = 0x9090;
		return true;
	}
	return false;
}

int32_t sys_set_thread_area(Registers& regs)
{
	Arguments& arg = regs.arg;
	struct linux_user_desc& u = *(struct linux_user_desc*) arg.a0.p;

#if 0
	log("  entry_number = %d", u.entry_number);
	log("  base_addr = %08x", u.base_addr);
	log("  limit = %08x", u.limit);
	log("  seg_32bit = %d", u.seg_32bit);
	log("  contents = %d", u.contents);
	log("  read_exec_only = %d", u.read_exec_only);
	log("  limit_in_pages = %d", u.limit_in_pages);
	log("  seg_not_present = %d", u.seg_not_present);
	log("  useable = %d", u.useable);
#endif

	/* Sanitise the limit; NT enforces a certain upper limit. */

	u_int64_t maximum = u.limit;
	if (u.limit_in_pages)
		maximum <<= 12;
	maximum += u.base_addr;
	if (maximum > 0x7ff00000)
		maximum = 0x7ff00000;

	LDT_ENTRY ldt;

	if (u.entry_number == -1)
	{
		/* Search for an unused segment. */

		int selector = 512;
		while (selector < 8192)
		{
			//log("examining selector %d", selector);
			u_int32_t r = read_ldt(selector, ldt);
			if (r != 0)
				break;
			if (!ldt.HighWord.Bits.Pres)
			{
				/* This selector is unused. */
				u.entry_number = selector;
				break;
			}
			selector++;
		}
		//log("found unused selector %d", u.entry_number);
	}

	if (u.entry_number == -1)
		return -ENOMEM;


	memset(&ldt, 0, sizeof(ldt));

	u_int32_t limit = maximum - u.base_addr;
	//log("maximum=%08llx, limit=%08x", maximum, limit);
	ldt.BaseLow                    = (u.base_addr & 0xFFFF);
	ldt.HighWord.Bits.BaseMid      = (u.base_addr >> 16) & 0xFF;
	ldt.HighWord.Bits.BaseHi       = (u.base_addr >> 24) & 0xFF;
	ldt.HighWord.Bits.Type         = 0x13; // RW data segment
	ldt.HighWord.Bits.Dpl          = 3;    // user segment
	ldt.HighWord.Bits.Pres         = 1;    // present
	ldt.HighWord.Bits.Sys          = 0;
	ldt.HighWord.Bits.Default_Big  = 1;    // 386 segment

	if (limit > 0x100000)
	{
		ldt.HighWord.Bits.Granularity = 1;
		limit >>= 12;
	}
	else
		ldt.HighWord.Bits.Granularity = 0;

	ldt.LimitLow                   = (limit & 0xFFFF);
	ldt.HighWord.Bits.LimitHi      = (limit >> 16) & 0xF;

	//display_selector(ldt);
	u_int32_t r = write_ldt(u.entry_number, ldt);
	if (r)
		return -EINVAL;

	if (try_perversion(regs, 48) ||
		try_perversion(regs, 18) ||
		try_perversion(regs, 28))
	{
		u_int16_t fs;
		asm volatile ("mov %%fs, %w0" : "=r" (fs));

		u_int16_t gs = (u.entry_number << 3) | 7;
		log("GS %04x now pointing at linear %08x", gs, u.base_addr);
		SetGS(gs, (void*) u.base_addr);
		//DumpMemory(u.base_addr, 32);
	}
	else
	{
		DumpMemory(regs.eip, 128);
		sleep(-1);
		error("unable to pervert GDT selector load; check code at %08x", regs.eip);
	}

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
	return -LINUX_ENOSYS;
}

SYSCALL(compat_sys_futex)
{
	int* uaddr = (int*) arg.a0.p;
	int op = arg.a1.s;
	int val = arg.a2.s;
	const void* timespec = (const void*) arg.a3.p;
	int* uaddr2 = (int*) arg.a4.p;
	int val3 = arg.a5.s;

	log("uaddr=%08x, *uaddr=%08x, val=%08x", uaddr, *uaddr, val);
	//sleep(-1);
	log("compat_sys_futex(%x) currently unimplemented", op);
	return -LINUX_ENOSYS;
}

SYSCALL(sys_gettid)
{
	return (u32) pthread_self();
}
