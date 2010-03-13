/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "filesystem/FD.h"
#include "filesystem/VFS.h"
#include "syscalls/mmap.h"
#include "ElfLoader.h"
#include "MemOp.h"
#include "binfmts.h"
#include "a.out.h"
#include <sys/mman.h>

//#define VERBOSE

static const char* elftype_to_str(int type)
{
	switch (type)
	{
		case PT_NULL:         return "PT_NULL        ";
		case PT_LOAD:         return "PT_LOAD        ";
		case PT_DYNAMIC:      return "PT_DYNAMIC     ";
		case PT_INTERP:       return "PT_INTERP      ";
		case PT_NOTE:         return "PT_NOTE        ";
		case PT_SHLIB:        return "PT_SHLIB       ";
		case PT_PHDR:         return "PT_PHDR        ";
		case PT_LOPROC:       return "PT_LOPROC      ";
		case PT_HIPROC:       return "PT_HIPROC      ";
		case PT_TLS:          return "PT_TLS         ";
		case PT_GNU_EH_FRAME: return "PT_GNU_EH_FRAME";
		case PT_GNU_STACK:    return "PT_GNU_STACK   ";
		case PT_RELRO:        return "PT_RELRO       ";

		default:
		{
			static char buffer[11];
			sprintf(buffer, "PT_%08x    ", type);
			return buffer;
		}
	}
}

ElfLoader::ElfLoader():
		_phdr(NULL)
{
}

ElfLoader::~ElfLoader()
{
	Close();
}

void ElfLoader::Open(const string& filename)
{
	int i;

	_fd = VFS::OpenFile(filename);
	int fd = _fd->GetRealFD();

	/* Load the header. */

	i = pread(fd, &_elfhdr, sizeof(_elfhdr), 0);
	if (i != sizeof(_elfhdr))
		throw ENOEXEC;

	/* Is this a loadable ELF file? */

	if ((_elfhdr.e_ident[0] != 0x7f) ||
		strncmp((char*) &_elfhdr.e_ident[1], "ELF", 3) != 0)
		throw ENOEXEC;

	if ((_elfhdr.e_type != ET_EXEC) && (_elfhdr.e_type != ET_DYN))
		throw ENOEXEC;
	if (!elf_check_arch(_elfhdr.e_machine))
		throw ENOEXEC;

	/* Read in the entire header. */

	if ((GetProgramHeaderSize() != sizeof(struct elf_phdr)) ||
		(GetNumProgramHeaders() < 1) ||
		(GetNumProgramHeaders() > (65536 / sizeof(struct elf_phdr))))
		throw ENOEXEC;

	delete [] _phdr;
	_phdr = NULL;

	int phdrsize = GetNumProgramHeaders() * sizeof(struct elf_phdr);
	_phdr = new struct elf_phdr[GetNumProgramHeaders()];
	i = pread(fd, _phdr, phdrsize, _elfhdr.e_phoff);
	if (i != phdrsize)
		throw ENOEXEC;

#if defined VERBOSE
	for (size_t i=0; i<GetNumProgramHeaders(); i++)
	{
		const struct elf_phdr& ph = GetProgramHeader(i);
		log("%03d %s %08x+%08x (%08x) @%08x %c%c%c %08x",
				i, elftype_to_str(ph.p_type),
				ph.p_vaddr, ph.p_filesz, ph.p_memsz, ph.p_offset,
				(ph.p_flags & PF_R) ? 'R' : ' ',
				(ph.p_flags & PF_W) ? 'W' : ' ',
				(ph.p_flags & PF_X) ? 'X' : ' ',
				ph.p_align);
	}
#endif

	/* Look for an interpreter here. */

	_interpreter = "";
	for (size_t i=0; i<GetNumProgramHeaders(); i++)
	{
		const struct elf_phdr& ph = GetProgramHeader(i);
		if (ph.p_type == PT_INTERP)
		{
			char buffer[ph.p_filesz+1];
			i = pread(fd, buffer, ph.p_filesz, ph.p_offset);
			if (i != ph.p_filesz)
				throw ENOEXEC;
			buffer[ph.p_filesz] = 0;
			_interpreter = buffer;
#if defined VERBOSE
			log("using interpreter %s", _interpreter.c_str());
#endif
		}
	}

	_entrypoint = _elfhdr.e_entry;
	_loadaddress = 0;
}

void ElfLoader::Close()
{
	delete [] _phdr;
	_entrypoint = 0;
}

const struct elf_phdr& ElfLoader::GetProgramHeader(int n) const
{
	const struct elf_phdr* phdr;
	if (_loadaddress)
		phdr = (const struct elf_phdr*) (_loadaddress + GetElfHeader().e_phoff);
	else
		phdr = _phdr;

	return phdr[n];
}

void ElfLoader::Load()
{
	/* We can only load ET_DYN and ET_EXEC executables. */

	if ((_elfhdr.e_type != ET_DYN) && (_elfhdr.e_type != ET_EXEC))
		error("not a supported ELF type! %d", _elfhdr.e_type);

	/* First, determine the memory range that's going to be in use. */

	u_int32_t minaddr = 0xffffffff;
	u_int32_t maxaddr = 0x00000000;

	for (size_t i=0; i<GetNumProgramHeaders(); i++)
	{
		const struct elf_phdr& ph = GetProgramHeader(i);
		if (ph.p_type == PT_LOAD)
		{
			if (ph.p_vaddr < minaddr)
				minaddr = ph.p_vaddr;

			u_int32_t end = ph.p_vaddr + ph.p_memsz;
			if (end > maxaddr)
				maxaddr = end;
		}
	}

	log("min addr = %08lx, max addr = %08lx", minaddr, maxaddr);

	/* If this is a dynamic executable, allocate an address range for it. */

	u32 loadoffset = 0;
	loadoffset = 0;
	if (_elfhdr.e_type == ET_DYN)
	{
		assert(minaddr == 0);
		loadoffset = do_mmap(NULL, maxaddr,
				LINUX_PROT_READ | LINUX_PROT_WRITE | LINUX_PROT_EXEC,
				LINUX_MAP_PRIVATE | LINUX_MAP_ANONYMOUS,
				_fd, 0);
		do_munmap((u8*) loadoffset, maxaddr);
#if defined VERBOSE
		log("actual load offset is %08x", loadoffset);
#endif
	}

	/* Load the executable into memory. */

	for (size_t i=0; i<GetNumProgramHeaders(); i++)
	{
		const struct elf_phdr& ph = GetProgramHeader(i);
		if (ph.p_type == PT_LOAD)
		{
			int prot = 0;
			if (ph.p_flags & PF_R)
				prot |= LINUX_PROT_READ;
			if (ph.p_flags & PF_W)
				prot |= LINUX_PROT_WRITE;
			if (ph.p_flags & PF_X)
				prot |= LINUX_PROT_EXEC;

#if defined VERBOSE
			log("raw area %08x-%08x", loadoffset + ph.p_vaddr, loadoffset + ph.p_vaddr + ph.p_memsz);
#endif
			u32 addr = MemOp::Align<0x1000>(loadoffset + ph.p_vaddr);
			u32 mlen = ph.p_memsz + MemOp::Offset<0x1000>(ph.p_vaddr);
			u32 flen = ph.p_filesz + MemOp::Offset<0x1000>(ph.p_vaddr);
			u32 off = ph.p_offset - MemOp::Offset<0x1000>(ph.p_vaddr);

			/* Ensure the entire area contains writeable memory. */

			if (prot & LINUX_PROT_WRITE)
			{
#if defined VERBOSE
				log("writeable area %08x-%08x", addr, addr+mlen);
#endif
				do_mmap((u8*) addr, mlen, prot,
						LINUX_MAP_PRIVATE | LINUX_MAP_FIXED | LINUX_MAP_ANONYMOUS,
						_fd, off);
			}

			/* Load the actual data. */

#if defined VERBOSE
			log("readable area %08x-%08x", addr, addr+flen);
#endif
			do_mmap((u8*) addr, flen, prot,
					LINUX_MAP_PRIVATE | LINUX_MAP_FIXED,
					_fd, off);

			/* Wipe any remaining data, if necessary --- we may have loaded a
			 * page too many.
			 */

			if (prot & LINUX_PROT_WRITE)
			{
#if defined VERBOSE
				log("wiping from %08x-%08x", addr+flen, addr+mlen);
#endif
				memset((u8*) (addr+flen), 0, MemOp::AlignUp<0x1000>(mlen) - flen);
			}
		}
	}

	_entrypoint = _elfhdr.e_entry + loadoffset;
	_loadaddress = loadoffset + minaddr;
#if defined VERBOSE
	log("entrypoint is %08x", _entrypoint);
#endif
}
