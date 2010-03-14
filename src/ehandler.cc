/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "MemOp.h"

#define EXCEPTION_MAXIMUM_PARAMETERS 15
#define MAXIMUM_SUPPORTED_EXTENSION 512

#define EXCEPTION_CONTINUE_EXECUTION (-1)
#define EXCEPTION_CONTINUE_SEARCH 0

#define STATUS_WAIT_0 0
#define STATUS_ABANDONED_WAIT_0 0x80
#define STATUS_USER_APC 0xC0
#define STATUS_TIMEOUT 0x102
#define STATUS_PENDING 0x103
#define STATUS_SEGMENT_NOTIFICATION 0x40000005
#define STATUS_GUARD_PAGE_VIOLATION 0x80000001
#define STATUS_DATATYPE_MISALIGNMENT 0x80000002
#define STATUS_BREAKPOINT 0x80000003
#define STATUS_SINGLE_STEP 0x80000004
#define STATUS_ACCESS_VIOLATION 0xC0000005
#define STATUS_IN_PAGE_ERROR 0xC0000006
#define STATUS_INVALID_HANDLE 0xC0000008L
#define STATUS_NO_MEMORY 0xC0000017
#define STATUS_ILLEGAL_INSTRUCTION 0xC000001D
#define STATUS_NONCONTINUABLE_EXCEPTION 0xC0000025
#define STATUS_INVALID_DISPOSITION 0xC0000026
#define STATUS_ARRAY_BOUNDS_EXCEEDED 0xC000008C
#define STATUS_FLOAT_DENORMAL_OPERAND 0xC000008D
#define STATUS_FLOAT_DIVIDE_BY_ZERO 0xC000008E
#define STATUS_FLOAT_INEXACT_RESULT 0xC000008F
#define STATUS_FLOAT_INVALID_OPERATION 0xC0000090
#define STATUS_FLOAT_OVERFLOW 0xC0000091
#define STATUS_FLOAT_STACK_CHECK 0xC0000092
#define STATUS_FLOAT_UNDERFLOW 0xC0000093
#define STATUS_INTEGER_DIVIDE_BY_ZERO 0xC0000094
#define STATUS_INTEGER_OVERFLOW 0xC0000095
#define STATUS_PRIVILEGED_INSTRUCTION 0xC0000096
#define STATUS_STACK_OVERFLOW 0xC00000FD
#define STATUS_CONTROL_C_EXIT 0xC000013A
#define STATUS_DLL_INIT_FAILED 0xC0000142
#define STATUS_DLL_INIT_FAILED_LOGOFF 0xC000026B
#define EXCEPTION_ACCESS_VIOLATION      STATUS_ACCESS_VIOLATION
#define EXCEPTION_DATATYPE_MISALIGNMENT STATUS_DATATYPE_MISALIGNMENT
#define EXCEPTION_BREAKPOINT    STATUS_BREAKPOINT
#define EXCEPTION_SINGLE_STEP   STATUS_SINGLE_STEP
#define EXCEPTION_ARRAY_BOUNDS_EXCEEDED STATUS_ARRAY_BOUNDS_EXCEEDED
#define EXCEPTION_FLT_DENORMAL_OPERAND  STATUS_FLOAT_DENORMAL_OPERAND
#define EXCEPTION_FLT_DIVIDE_BY_ZERO    STATUS_FLOAT_DIVIDE_BY_ZERO
#define EXCEPTION_FLT_INEXACT_RESULT    STATUS_FLOAT_INEXACT_RESULT
#define EXCEPTION_FLT_INVALID_OPERATION STATUS_FLOAT_INVALID_OPERATION
#define EXCEPTION_FLT_OVERFLOW  STATUS_FLOAT_OVERFLOW
#define EXCEPTION_FLT_STACK_CHECK       STATUS_FLOAT_STACK_CHECK
#define EXCEPTION_FLT_UNDERFLOW STATUS_FLOAT_UNDERFLOW
#define EXCEPTION_INT_DIVIDE_BY_ZERO    STATUS_INTEGER_DIVIDE_BY_ZERO
#define EXCEPTION_INT_OVERFLOW  STATUS_INTEGER_OVERFLOW
#define EXCEPTION_PRIV_INSTRUCTION      STATUS_PRIVILEGED_INSTRUCTION
#define EXCEPTION_IN_PAGE_ERROR STATUS_IN_PAGE_ERROR
#define EXCEPTION_ILLEGAL_INSTRUCTION   STATUS_ILLEGAL_INSTRUCTION
#define EXCEPTION_NONCONTINUABLE_EXCEPTION      STATUS_NONCONTINUABLE_EXCEPTION
#define EXCEPTION_STACK_OVERFLOW        STATUS_STACK_OVERFLOW
#define EXCEPTION_INVALID_DISPOSITION   STATUS_INVALID_DISPOSITION
#define EXCEPTION_GUARD_PAGE    STATUS_GUARD_PAGE_VIOLATION
#define EXCEPTION_INVALID_HANDLE        STATUS_INVALID_HANDLE

struct FLOATING_SAVE_AREA
{
	u32 ControlWord;
	u32 StatusWord;
	u32 TagWord;
	u32 ErrorOffset;
	u32 ErrorSelector;
	u32 DataOffset;
	u32 DataSelector;
	u8 RegisterArea[80];
	u32 Cr0NpxState;
};

struct CONTEXT
{
	u32 ContextFlags;
	u32 Dr0;
	u32 Dr1;
	u32 Dr2;
	u32 Dr3;
	u32 Dr6;
	u32 Dr7;
	FLOATING_SAVE_AREA FloatSave;
	u32 SegGs;
	u32 SegFs;
	u32 SegEs;
	u32 SegDs;
	u32 Edi;
	u32 Esi;
	u32 Ebx;
	u32 Edx;
	u32 Ecx;
	u32 Eax;
	u32 Ebp;
	u32 Eip;
	u32 SegCs;
	u32 EFlags;
	u32 Esp;
	u32 SegSs;
	u8 ExtendedRegisters[MAXIMUM_SUPPORTED_EXTENSION];
};

struct EXCEPTION_RECORD
{
	u32 ExceptionCode;
	u32 ExceptionFlags;
	EXCEPTION_RECORD* ExceptionRecord;
    void* ExceptionAddress;
    u32 NumberParameters;
    u32 ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
};

struct EXCEPTION_POINTERS
{
	EXCEPTION_RECORD* ExceptionRecord;
	CONTEXT* ContextRecord;
};

typedef s32 __stdcall VectoredExceptionHandler(EXCEPTION_POINTERS* ep);

extern "C" void* __stdcall RtlAddVectoredExceptionHandler(u32 first,
		VectoredExceptionHandler* handler);
asm ("_RtlAddVectoredExceptionHandler: jmp _RtlAddVectoredExceptionHandler@8");

static pthread_key_t linear_key = 0;

void InitGSStore()
{
	/* If linear_key is not 0, then we must have forked and
	 * we're looking at the keys for the old process. In which case, we
	 * don't need to create new ones.
	 */

	if (linear_key == 0)
		pthread_key_create(&linear_key, NULL);
}

void SetGS(u_int16_t gs, void* linear)
{
	//pthread_setspecific(gs_key, (void*) gs);
	pthread_setspecific(linear_key, linear);
}

void printregs(CONTEXT& regs)
{
	log("eax = %08x", regs.Eax);
	log("ebx = %08x", regs.Ebx);
	log("ecx = %08x", regs.Ecx);
	log("edx = %08x", regs.Edx);
	log("esi = %08x", regs.Esi);
	log("edi = %08x", regs.Edi);
	log("ebp = %08x", regs.Ebp);
	log("eip = %08x", regs.Eip);
	log("esp = %08x", regs.Esp);
	log(" cs = %08x", regs.SegCs);
	log(" ds = %08x", regs.SegDs);
	log(" ss = %08x", regs.SegSs);
	log(" es = %08x", regs.SegEs);
	log(" fs = %08x", regs.SegFs);
	log(" gs = %08x", regs.SegGs);

	u32* stack = (u32*) regs.Esp;
	log("stack: %08x %08x %08x %08x",
			stack[0], stack[1], stack[2], stack[3]);
	log("       %08x %08x %08x %08x",
			stack[4], stack[5], stack[6], stack[7]);

	u8* code = (u8*) regs.Eip;
	log("code: %02x %02x %02x %02x %02x %02x %02x %02x",
			code[0], code[1], code[2], code[3],
			code[4], code[5], code[6], code[7]);
}

static u32 getmodrmsize(int modrm)
{
	int mod = modrm >> 6;
	int rm = modrm & 7;

	switch (mod)
	{
		case 0:
			if (rm == 5)
				return 4;
			break;

		case 1:
			return 1;

		case 2:
			return 4;

		case 3:
			error("mod 3 not supported yet");
	}

	return 0;
}

template <typename T, typename S> static T read(S& reg)
{
	T value = *(T*) reg;
	reg += sizeof(T);
	return value;
}

template <typename T, typename S> static void store(T value, S address)
{
	*(T*)address = value;
}

template <typename T, typename S> static T load(S address)
{
	return *(T*)address;
}

static void dumpbuffer(u8* out, u32 insnlength)
{
#if defined VERBOSE
	insnlength += out - InterpretGS_MCE_ReturnBlock;
	DumpMemory(InterpretGS_MCE_ReturnBlock, insnlength + 1);
#endif
}

static void patchedinstruction(CONTEXT& regs, u8* out, u32 insnlength, u32 patchoffset)
{
	u32 linear = (u32) pthread_getspecific(linear_key);

#if defined VERBOSE
	DumpMemory(regs.eip, insnlength);
	log("patch offset %d, linear is %08x", patchoffset, linear);
#endif

	memcpy(out, (void*) regs.Eip, insnlength);
	u32 const32 = load<u32>(out + patchoffset);
	store<u32>(const32 + linear, out + patchoffset);
	store<u32>(0xc3, out + insnlength);

	dumpbuffer(out, insnlength);
	regs.Eip += insnlength;
}

static void mungedinstruction(CONTEXT& regs, u8* out, u32 insnlength, u32 insertat)
{
	u32 linear = (u32) pthread_getspecific(linear_key);

#if defined VERBOSE
	DumpMemory(regs.Eip, insnlength);
	log("munging instruction, linear is %08x", linear);
#endif

	u8 opcode = MemOp::Load<u8>(regs.Eip);
	u8 modrm = MemOp::Load<u8>(regs.Eip+1);

	assert((modrm & 0xc0) == 0);
	assert((modrm & 0x07) != 5);
	modrm |= 0x80;

	MemOp::Store<u8> (opcode, out + 0);
	MemOp::Store<u8> (modrm,  out + 1);
	MemOp::Store<u32>(linear, out + 2);

	if (insertat < insnlength)
		memcpy((void*)(out + 6), (void*)(regs.Eip + insertat),
				insnlength - insertat);

	MemOp::Store<u8> (0xc3,   out + insnlength + 4); /* ret */

	dumpbuffer(out, 7);
	regs.Eip += insnlength;

	//assert(insertat == insnlength);
}

static void nopinstruction(CONTEXT& regs, u8* out, u32 insnlength)
{
	out[0] = 0xc3; // ret

	dumpbuffer(out, 1);
	regs.Eip += insnlength;
}

static void translate_gs_instruction(CONTEXT& regs, u8* buffer)
{
	u8* out = buffer;

	u8 b = MemOp::LoadAndAdvance<u8>(regs.Eip);
	if (b == 0xf0)
	{
		*out++ = 0xf0;
		b = MemOp::LoadAndAdvance<u8>(regs.Eip);
	}
	assert(b == 0x65);

	u32 ip = regs.Eip;
	switch (MemOp::LoadAndAdvance<u8>(ip))
	{
		case 0x0f: // hideous two-byte instructions
		{
			switch (MemOp::LoadAndAdvance<u8>(ip))
			{
				case 0xb1: // cmpxchg Eb, Gb
				{
					u8 modrm = MemOp::LoadAndAdvance<u8>(ip);
					switch (getmodrmsize(modrm))
					{
						case 4:	 return patchedinstruction(regs, out, 7, 3);
						default: goto unknown;
					}
				}

				default:
					goto unknown;
			}
		}

		case 0x83: // <group1> Ev, Ib
		{
			u8 modrm = MemOp::LoadAndAdvance<u8>(ip);
			switch (getmodrmsize(modrm))
			{
				case 0:  return mungedinstruction(regs, out, 3, 2);
				case 4:  return patchedinstruction(regs, out, 7, 2);
				default: goto unknown;
			}
		}

		case 0x8a: // mov Gb, Eb
		{
			u8 modrm = MemOp::LoadAndAdvance<u8>(ip);
			switch (getmodrmsize(modrm))
			{
				case 4:	 return patchedinstruction(regs, out, 6, 2);
				default: goto unknown;
			}
		}

		case 0x0b: // or Gv, Ev
		case 0x03: // add Gv, Ev
		case 0x33: // xor Gv, Ev
		case 0x87: // xchg Ev, Gv
		case 0x89: // mov Ev, Gv
		case 0x8b: // mov Gv, Ev
		{
			u8 modrm = MemOp::LoadAndAdvance<u8>(ip);
			switch (getmodrmsize(modrm))
			{
				case 0:  return mungedinstruction(regs, out, 2, 2);
				case 4:	 return patchedinstruction(regs, out, 6, 2);
				default: goto unknown;
			}
		}

		case 0xa1: // mov [gs+offset], eax
		case 0xa2: // mov al, [gs+offset]
		case 0xa3: // mov eax, [gs+offset]
			return patchedinstruction(regs, out, 5, 1);

		case 0xc6: // <group 12> Eb, Ib
		{
			u8 modrm = MemOp::LoadAndAdvance<u8>(ip);
			switch (getmodrmsize(modrm))
			{
				case 4:  return patchedinstruction(regs, out, 7, 2);
				default: goto unknown;
			}
		}

		case 0xc7: // <group 12> Ev, Iz
		{
			u8 modrm = MemOp::LoadAndAdvance<u8>(ip);
			switch (getmodrmsize(modrm))
			{
				case 0:  return mungedinstruction(regs, out, 6, 2);
				case 4:  return patchedinstruction(regs, out, 10, 2);
				default: goto unknown;
			}
		}

		default:
			unknown:
			{
				DumpMemory(regs.Eip-1, 128);
				error("unable to interpret above instruction sequence");
			}
	}
}

static s32 __stdcall handler_cb(EXCEPTION_POINTERS* ep)
{
	static u8 trampoline[32];

	if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_PRIV_INSTRUCTION)
		printregs(*ep->ContextRecord);

	/* We're only interested in access violations. */

	if (ep->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION)
		return EXCEPTION_CONTINUE_SEARCH;

	u8* code = (u8*) ep->ContextRecord->Eip;
	if (!code || ((u32)code > 0x80000000U))
		goto fallback;

	switch (code[0])
	{
		case 0xcd: /* INT */
		{
			/* This is a system call. Check that it's
			 * an int 0x80 Linux system call entry.
			 */

			if (code[1] != 0x80)
				goto fallback;

			/* Create the trampoline code. */

			MemOp::Store<u8>(0x68, trampoline+0); // push Iz
			MemOp::Store<u32>(ep->ContextRecord->Eip+2, trampoline+1); // return address
			MemOp::Store<u8>(0x68, trampoline+5); // push Iz
			MemOp::Store<u32>((u32) Linux_MCE, trampoline+6); // syscall handler
			MemOp::Store<u8>(0xc3, trampoline+10); // ret
			ep->ContextRecord->Eip = (u32) trampoline;

			return EXCEPTION_CONTINUE_EXECUTION;
		}

		case 0x65: /* GS: */
		{
			/* This is an instruction that uses %gs in some way. These
			 * have to be translated before execution.
			 */

			translate_gs_instruction(*ep->ContextRecord, trampoline+5);

			MemOp::Store<u8>(0x68, trampoline+0); // push Iz
			MemOp::Store<u32>(ep->ContextRecord->Eip, trampoline+1); // return address

			ep->ContextRecord->Eip = (u32) trampoline;

			return EXCEPTION_CONTINUE_EXECUTION;
		}

		case 0x8e: /* GS load, probably */
		{
			/* If this is an attempt to load %gs with the correct value,
			 * ignore it (as we're entirely emulating %gs support).
			 */

			if (code[1] != 0xe8)
				goto fallback;

			if (ep->ContextRecord->Eax != 0x53)
				error("attempt to load %gs with the wrong value: %08x", ep->ContextRecord->Eax);

			ep->ContextRecord->Eip += 2;
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		default:
			printregs(*ep->ContextRecord);
			exit(0);
			return EXCEPTION_CONTINUE_SEARCH;
	}

fallback:
	printregs(*ep->ContextRecord);
	error("fatal exception %08x!", ep->ExceptionRecord->ExceptionCode);
}

void InstallExceptionHandler()
{
	RtlAddVectoredExceptionHandler(true, handler_cb);
}
