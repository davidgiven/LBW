/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "syscalls/syscalls.h"

//#define VERBOSE

static pthread_key_t gs_key = 0;
static pthread_key_t linear_key = 0;

extern "C" void InterpretGS_MCE_Handler(Registers& regs)
	__attribute__ ((regparm (1)));

extern "C" u8 InterpretGS_MCE_ReturnBlock[16];

__asm (
	".data; "
	".globl _InterpretGS_MCE; "
	".extern _InterpretGS_MCE; "
	"_InterpretGS_MCE: "
		"pushf; "
		"pushl %gs; "
		"pushl %ebp; "
        "pushl %edi; "
        "pushl %esi; "
        "pushl %edx; "
        "pushl %ecx; "
        "pushl %ebx; "
        "pushl %eax; "
		"mov %esp, %eax; "
		"call _InterpretGS_MCE_Handler; "
		"popl %eax; "
		"popl %ebx; "
		"popl %ecx; "
		"popl %edx; "
		"popl %esi; "
		"popl %edi; "
		"popl %ebp; "
		"popl %gs; "
		"popf; "
	"_InterpretGS_MCE_ReturnBlock: "
		"ret; ret; ret; ret; "
		"ret; ret; ret; ret; "
		"ret; ret; ret; ret; "
		"ret; ret; ret; ret; "
	);

void InitGSStore()
{
	/* If gs_key and linear_key are not 0, then we must have forked and
	 * we're looking at the keys for the old process. In which case, we
	 * don't need to create new ones.
	 */

	if (gs_key == 0)
	{
		pthread_key_create(&gs_key, NULL);
		pthread_key_create(&linear_key, NULL);
	}

	log("gs_key=%p, linear_key=%p", gs_key, linear_key);
}

#if 0
u_int16_t ReloadGS_MCE_Handler()
{
	u_int16_t gs = (u_int16_t)(u_int32_t) pthread_getspecific(gs_key);
	if (gs == 0)
		error("could not find a GS for this thread");

	return gs;
}
#endif

void SetGS(u_int16_t gs, void* linear)
{
	pthread_setspecific(gs_key, (void*) gs);
	pthread_setspecific(linear_key, linear);
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

static void patchedinstruction(Registers& regs, u8* out, u32 insnlength, u32 patchoffset)
{
	u32 linear = (u32) pthread_getspecific(linear_key);

#if defined VERBOSE
	DumpMemory(regs.eip, insnlength);
	log("patch offset %d, linear is %08x", patchoffset, linear);
#endif

	memcpy(out, (void*) regs.eip, insnlength);
	u32 const32 = load<u32>(out + patchoffset);
	store<u32>(const32 + linear, out + patchoffset);
	store<u32>(0xc3, out + insnlength);

	dumpbuffer(out, insnlength);
	regs.eip += insnlength;
}

static void mungedinstruction(Registers& regs, u8* out, u32 insnlength, u32 insertat)
{
	u32 linear = (u32) pthread_getspecific(linear_key);

#if defined VERBOSE
	DumpMemory(regs.eip, insnlength);
	log("munging instruction, linear is %08x", linear);
#endif

	u8 opcode = load<u8>(regs.eip);
	u8 modrm = load<u8>(regs.eip+1);

	assert((modrm & 0xc0) == 0);
	assert((modrm & 0x07) != 5);
	modrm |= 0x80;

	store<u8> (opcode, out + 0);
	store<u8> (modrm,  out + 1);
	store<u32>(linear, out + 2);

	if (insertat < insnlength)
		memcpy((void*)(out + 6), (void*)(regs.eip + insertat),
				insnlength - insertat);

	store<u8> (0xc3,   out + insnlength + 4); /* ret */

	dumpbuffer(out, 7);
	regs.eip += insnlength;

	//assert(insertat == insnlength);
}

static void nopinstruction(Registers& regs, u8* out, u32 insnlength)
{
	out[0] = 0xc3; // ret

	dumpbuffer(out, 1);
	regs.eip += insnlength;
}

void InterpretGS_MCE_Handler(Registers& regs)
{
	u8* out = InterpretGS_MCE_ReturnBlock;

	u8 b = read<u8>(regs.eip);
	if (b == 0xf0)
	{
		*out++ = 0xf0;
		b = read<u8>(regs.eip);
	}
	assert(b == 0x65);

	u32 ip = regs.eip;
	switch (read<u8>(ip))
	{
		case 0x0f: // hideous two-byte instructions
		{
			switch (read<u8>(ip))
			{
				case 0xb1: // cmpxchg Eb, Gb
				{
					u8 modrm = read<u8>(ip);
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
			u8 modrm = read<u8>(ip);
			switch (getmodrmsize(modrm))
			{
				case 0:  return mungedinstruction(regs, out, 3, 2);
				case 4:  return patchedinstruction(regs, out, 7, 2);
				default: goto unknown;
			}
		}

		case 0x8a: // mov Gb, Eb
		{
			u8 modrm = read<u8>(ip);
			switch (getmodrmsize(modrm))
			{
				case 4:	 return patchedinstruction(regs, out, 6, 2);
				default: goto unknown;
			}
		}

		case 0x03: // add Gv, Ev
		case 0x33: // xor Gv, Ev
		case 0x87: // xchg Ev, Gv
		case 0x89: // mov Ev, Gv
		case 0x8b: // mov Gv, Ev
		{
			u8 modrm = read<u8>(ip);
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
			u8 modrm = read<u8>(ip);
			switch (getmodrmsize(modrm))
			{
				case 4:  return patchedinstruction(regs, out, 7, 2);
				default: goto unknown;
			}
		}

		case 0xc7: // <group 12> Ev, Iz
		{
			u8 modrm = read<u8>(ip);
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
				DumpMemory(regs.eip-1, 128);
				error("unable to interpret above instruction sequence");
			}
	}
}
