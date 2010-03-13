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

#if 0
typedef struct
{
  //DWORD start_brk, brk, start_code, end_code, end_data, start_stack;

  u_int32_t arg_start, arg_end, env_start, env_end;
  u_int32_t start_brk;
  u_int32_t brk;
  u_int32_t start_code;
  u_int32_t end_code;
  u_int32_t start_stack;
  u_int32_t end_stack;
  u_int32_t end_data;


} procinfo_t;

static procinfo_t currento;
static procinfo_t* current = &currento;

#define __VERBOSE__

#define INTERPRETER_NONE 0
#define INTERPRETER_AOUT 1
#define INTERPRETER_ELF 2

#define ELF_PAGESTART(_v) ((_v) & ~(unsigned long)(ELF_EXEC_PAGESIZE-1))
#define ELF_PAGEOFFSET(_v) ((_v) & (ELF_EXEC_PAGESIZE-1))
#define ELF_PAGEALIGN(_v) (((_v) + ELF_EXEC_PAGESIZE - 1) & ~(ELF_EXEC_PAGESIZE - 1))

#define DLINFO_ITEMS 13
typedef u_int32_t elf_addr_t;
typedef char* elf_caddr_t;

#define __put_user(val, addr) ( *(unsigned long*)(addr) = (unsigned long)(val))

unsigned long copy_strings(int argc, char ** argv, unsigned long *page,
		unsigned long p)
{
	char *str;

	if ((long) p <= 0)
		return p; /* bullet-proofing */

	while (argc-- > 0)
	{
		unsigned int len;
		unsigned long pos;

		str = argv[argc];
		if (!str)
		{
			return -EFAULT;
		}

		len = strlen(str) + 1; /* includes the '\0' */
		if (!len || len > p)
		{ /* EFAULT or E2BIG */
			return len ? -E2BIG : -EFAULT;
		}

		p -= len;
		pos = p;
		while (len > 0)
		{
			char *pag;
			int offset, bytes_to_copy;

			offset = pos % PAGE_SIZE;
			pag = (char *) page[pos / PAGE_SIZE];
			if (!pag)
			{
				pag = (char*) malloc(PAGE_SIZE);
				if (!pag)
					return -EFAULT;
				page[pos / PAGE_SIZE] = (u_int32_t) pag;
			}

			bytes_to_copy = PAGE_SIZE - offset;
			if (bytes_to_copy > len)
				bytes_to_copy = len;

			memcpy(pag + offset, str, bytes_to_copy);
			pos += bytes_to_copy;
			str += bytes_to_copy;
			len -= bytes_to_copy;
		}
	}
	return p;
}

static elf_addr_t* create_elf_tables(char *p, int argc, int envc,
		struct elfhdr * exec, unsigned long load_addr, unsigned long load_bias,
		unsigned long interp_load_addr, int ibcs) {
	elf_caddr_t *argv;
	elf_caddr_t *envp;
	elf_addr_t *sp, *csp;
	const char* k_platform;
	char* u_platform;
	long hwcap;
	size_t platform_len = 0;

	/*
	 * Get hold of platform and hardware capabilities masks for
	 * the machine we are running on.  In some cases (Sparc),
	 * this info is impossible to get, in others (i386) it is
	 * merely difficult.
	 */

	hwcap = ELF_HWCAP;
	k_platform = ELF_PLATFORM;

	if (k_platform) {
		platform_len = strlen(k_platform) + 1;
		u_platform = p - platform_len;
		memcpy(u_platform, k_platform, platform_len);
	} else
		u_platform = p;

	/*
	 * Force 16 byte _final_ alignment here for generality.
	 * Leave an extra 16 bytes free so that on the PowerPC we
	 * can move the aux table up to start on a 16-byte boundary.
	 */
	sp = (elf_addr_t *) ((~15UL & (unsigned long) (u_platform)) - 16UL);
	csp = sp;
	csp -= ((exec ? DLINFO_ITEMS * 2 : 4) + (k_platform ? 2 : 0));
	csp -= envc + 1;
	csp -= argc + 1;
	csp -= (!ibcs ? 3 : 1); /* argc itself */
	if ((unsigned long) csp & 15UL)
		sp -= ((unsigned long) csp & 15UL) / sizeof(*sp);

	/*
	 * Put the ELF interpreter info on the stack
	 */
#define NEW_AUX_ENT(nr, id, val) \
  __put_user ((id), sp+(nr*2)); \
  __put_user ((val), sp+(nr*2+1));

	sp -= 2;
	NEW_AUX_ENT(0, AT_NULL, 0);
	if (k_platform) {
		sp -= 2;
		NEW_AUX_ENT(0, AT_PLATFORM, (elf_addr_t)(unsigned long) u_platform);
	}
	sp -= 2;
	NEW_AUX_ENT(0, AT_HWCAP, hwcap);

	if (exec) {
		sp -= 11 * 2;

		NEW_AUX_ENT(0, AT_PHDR, load_addr + exec->e_phoff);
		NEW_AUX_ENT(1, AT_PHENT, sizeof (struct elf_phdr));
		NEW_AUX_ENT(2, AT_PHNUM, exec->e_phnum);
		NEW_AUX_ENT(3, AT_PAGESZ, ELF_EXEC_PAGESIZE);
		NEW_AUX_ENT(4, AT_BASE, interp_load_addr);
		NEW_AUX_ENT(5, AT_FLAGS, 0);
		NEW_AUX_ENT(6, AT_ENTRY, load_bias + exec->e_entry);
		NEW_AUX_ENT(7, AT_UID, (elf_addr_t) getuid());
		NEW_AUX_ENT(8, AT_EUID, (elf_addr_t) geteuid());
		NEW_AUX_ENT(9, AT_GID, (elf_addr_t) getgid());
		NEW_AUX_ENT(10, AT_EGID, (elf_addr_t) getegid());
	}
#undef NEW_AUX_ENT

	sp -= envc + 1;
	envp = (elf_caddr_t *) sp;
	sp -= argc + 1;
	argv = (elf_caddr_t *) sp;
	if (!ibcs) {
		__put_user((elf_addr_t) (unsigned long) envp, --sp);
		__put_user((elf_addr_t) (unsigned long) argv, --sp);
	}

	__put_user((elf_addr_t) argc, --sp);
	current->arg_start = (unsigned long) p;
	while (argc-- > 0) {
		__put_user((elf_caddr_t) (unsigned long) p, argv++);
		p += strlen(p) + 1;
	}
	__put_user(NULL, argv);
	current->arg_end = current->env_start = (unsigned long) p;
	while (envc-- > 0) {
		__put_user((elf_caddr_t) (unsigned long) p, envp++);
		p += strlen(p) + 1;
	}
	__put_user(NULL, envp);
	current->env_end = (unsigned long) p;
	return sp;
}

static void padzero(unsigned long elf_bss)
{
  unsigned long nbyte;

  nbyte = ELF_PAGEOFFSET(elf_bss);
  if (nbyte) {
    nbyte = ELF_EXEC_PAGESIZE - nbyte;
    bzero((void *) elf_bss, nbyte);
  }
}

static int load_elf_binary(struct linux_binprm *bprm)
{
  struct pt_regs regs;
  int interpreter_fd = -1;
   unsigned long load_addr = 0, load_bias;
  int load_addr_set = 0;
  char * elf_interpreter = NULL;
  unsigned int interpreter_type = INTERPRETER_NONE;
  unsigned long error;
  struct elf_phdr * elf_ppnt, *elf_phdata;
  unsigned long elf_bss, k, elf_brk;
  int elf_exec_fileno;
  int retval, size, i;
  unsigned long elf_entry, interp_load_addr = 0;
  unsigned long start_code, end_code, end_data;
  struct elfhdr elf_ex;
  struct elfhdr interp_elf_ex;
  struct exec interp_ex;
  char passed_fileno[6];

  /* Get the exec-header */
  elf_ex = *((struct elfhdr *) bprm->buf);

  retval = -ENOEXEC;
  /* First of all, some simple consistency checks */
  if (elf_ex.e_ident[0] != 0x7f ||
      strncmp((char*) &elf_ex.e_ident[1], "ELF", 3) != 0)
    goto out;

  if (elf_ex.e_type != ET_EXEC && elf_ex.e_type != ET_DYN)
    goto out;
  if (!elf_check_arch(elf_ex.e_machine))
    goto out;


  /* Now read in all of the header information */

  if (elf_ex.e_phentsize != sizeof(struct elf_phdr) ||
      elf_ex.e_phnum < 1 ||
      elf_ex.e_phnum > 65536 / sizeof(struct elf_phdr))
    goto out;

  retval = -ENOMEM;
  size = elf_ex.e_phentsize * elf_ex.e_phnum;
  elf_phdata = (struct elf_phdr *) malloc(size);
  if (!elf_phdata)
    goto out;

  retval = read_exec(bprm->fd, elf_ex.e_phoff, (char *) elf_phdata, size);
  if (retval < 0)
    goto out_free_ph;


  elf_exec_fileno = dup(bprm->fd);
  lseek(elf_exec_fileno, 0, SEEK_SET);

  elf_ppnt = elf_phdata;
  elf_bss = 0;
  elf_brk = 0;

  start_code = ~0UL;
  end_code = 0;
  end_data = 0;

  /* look for interpreter */
  for (i = 0; i < elf_ex.e_phnum; i++) {
    if (elf_ppnt->p_type == PT_INTERP) {
      retval = -ENOEXEC;
        if (elf_interpreter ||
          elf_ppnt->p_filesz < 2 ||
          elf_ppnt->p_filesz > PAGE_SIZE)
        goto out_free_dentry;

      /* This is the program interpreter used for
       * shared libraries - for now assume that this
       * is an a.out format binary
       */

      retval = -ENOMEM;
      elf_interpreter = (char *)malloc(elf_ppnt->p_filesz);
      if (!elf_interpreter)
        goto out_free_file;

      retval = read_exec(bprm->fd, elf_ppnt->p_offset,
             elf_interpreter, elf_ppnt->p_filesz);
      if (retval < 0)
        goto out_free_interp;
      elf_interpreter[elf_ppnt->p_filesz - 1] = 0;

#if 0
      /* If the program interpreter is one of these two,
       * then assume an iBCS2 image. Otherwise assume
       * a native linux image.
       */
      if (strcmp(elf_interpreter,"/usr/lib/libc.so.1") == 0 ||
          strcmp(elf_interpreter,"/usr/lib/ld.so.1") == 0)
        ibcs2_interpreter = 1;
#endif

      log("Using ELF interpreter: %s", elf_interpreter);

      interpreter_fd = open(elf_interpreter, O_RDONLY);

      if (interpreter_fd < 0) {
        retval = -errno;
        goto out_free_interp;
      }

#if 0
      retval = permission(interpreter_dentry->d_inode, MAY_EXEC);
      if (retval < 0)
        goto out_free_dentry;
#endif

      retval = read_exec(interpreter_fd, 0, bprm->buf, 128);
      if (retval < 0)
        goto out_free_dentry;

      /* Get the exec headers */
      interp_ex = *((struct exec *) bprm->buf);
      interp_elf_ex = *((struct elfhdr *) bprm->buf);
    }
    elf_ppnt++;
  }


  /* Some simple consistency checks for the interpreter */
  if (elf_interpreter) {
    interpreter_type = INTERPRETER_ELF | INTERPRETER_AOUT;

    /* Now figure out which format our binary is */
    if ((N_MAGIC(interp_ex) != OMAGIC) &&
        (N_MAGIC(interp_ex) != ZMAGIC) &&
        (N_MAGIC(interp_ex) != QMAGIC))
      interpreter_type = INTERPRETER_ELF;

    if (interp_elf_ex.e_ident[0] != 0x7f ||
        strncmp((char*) &interp_elf_ex.e_ident[1], "ELF", 3) != 0)
      interpreter_type &= ~INTERPRETER_ELF;

    retval = -ENOEXEC;
    if (!interpreter_type)
      goto out_free_dentry;

    /* Make sure only one type was selected */
    if ((interpreter_type & INTERPRETER_ELF) &&
         interpreter_type != INTERPRETER_ELF) {
      printf("ELF: Ambiguous type, using ELF\n");
      interpreter_type = INTERPRETER_ELF;
    }
  }

  /* OK, we are done with that, now set up the arg stuff,
     and then start this sucker up */

  if (!bprm->sh_bang) {
    char * passed_p;

    if (interpreter_type == INTERPRETER_AOUT) {
      sprintf(passed_fileno, "%d", elf_exec_fileno);
      passed_p = passed_fileno;

      if (elf_interpreter) {
        bprm->p = copy_strings(1,&passed_p,bprm->page,bprm->p);
        bprm->argc++;
      }
    }
    retval = -E2BIG;
    if (!bprm->p)
      goto out_free_dentry;
  }


#if 0
  /* Flush all traces of the currently running executable */
  retval = flush_old_exec(bprm);
  if (retval)
    goto out_free_dentry;
#endif

  /* OK, This is the point of no return */
  current->end_data = 0;
  current->end_code = 0;
#if 0
  current->mm->mmap = NULL;
  current->flags &= ~PF_FORKNOEXEC;

#endif

  elf_entry = (unsigned long) elf_ex.e_entry;


#if 0
  /* Do this immediately, since STACK_TOP as used in setup_arg_pages
     may depend on the personality.  */
  SET_PERSONALITY(elf_ex, ibcs2_interpreter);
#endif

  /* Do this so that we can load the interpreter, if need be.  We will
     change some of these later */
//  current->mm->rss = 0;
  assert(false);
  //bprm->p = setup_arg_pages(bprm->p, bprm);

  current->start_stack = bprm->p;

  /* Try and get dynamic programs out of the way of the default mmap
     base, as well as whatever program they might try to exec.  This
     is because the brk will follow the loader, and is not movable.  */

  load_bias = ELF_PAGESTART(elf_ex.e_type==ET_DYN ? ELF_ET_DYN_BASE : 0);
#ifdef __VERBOSE__
  printf("load_bias: %08lX\n", load_bias);
#endif

  /* Now we do a little grungy work by mmaping the ELF image into
     the correct location in memory.  At this point, we assume that
     the image should be loaded at fixed address, not at a variable
     address. */

  for(i = 0, elf_ppnt = elf_phdata; i < elf_ex.e_phnum; i++, elf_ppnt++) {
    int elf_prot = 0, elf_flags;
    unsigned long vaddr;

    if (elf_ppnt->p_type != PT_LOAD)
      continue;

    if (elf_ppnt->p_flags & PF_R) elf_prot |= PROT_READ;
    if (elf_ppnt->p_flags & PF_W) elf_prot |= PROT_WRITE;
    if (elf_ppnt->p_flags & PF_X) elf_prot |= PROT_EXEC;

    elf_flags = MAP_PRIVATE; // |MAP_DENYWRITE|MAP_EXECUTABLE;

    vaddr = elf_ppnt->p_vaddr;
    if (elf_ex.e_type == ET_EXEC || load_addr_set) {
      elf_flags |= MAP_FIXED;
    }

#ifdef __VERBOSE__
    printf("mapping: %08lX\n", ELF_PAGESTART(load_bias + vaddr));
#endif

    {
    	void* addr = (void*) ELF_PAGESTART(load_bias + vaddr);
    	size_t len = elf_ppnt->p_filesz + ELF_PAGEOFFSET(elf_ppnt->p_vaddr);
    	off_t off = elf_ppnt->p_offset - ELF_PAGEOFFSET(elf_ppnt->p_vaddr);

    	error = (u_int32_t) mmap(addr, len,
                    elf_prot, elf_flags,
                    bprm->fd, off);
    }

#ifdef __VERBOSE__
    printf("error: %08lX\n", error);
#endif

    if (!load_addr_set) {
      load_addr_set = 1;
      load_addr = (elf_ppnt->p_vaddr - elf_ppnt->p_offset);
#ifdef __VERBOSE__
      printf("load_addr: %08lX, vaddr: %08lX\n", load_addr, vaddr);
#endif
      if (elf_ex.e_type == ET_DYN) {
        load_bias += error - ELF_PAGESTART(load_bias + vaddr);
        load_addr += error;

#ifdef __VERBOSE__
        printf("new\nload_bias: %08lX, load_addr: %08lX\n", load_bias, load_addr);
#endif
      }
    }

    k = elf_ppnt->p_vaddr;
    if (k < start_code) start_code = k;
    k = elf_ppnt->p_vaddr + elf_ppnt->p_filesz;
    if (k > elf_bss)
      elf_bss = k;
    if ((elf_ppnt->p_flags & PF_X) && end_code <  k)
      end_code = k;
    if (end_data < k)
      end_data = k;
    k = elf_ppnt->p_vaddr + elf_ppnt->p_memsz;
    if (k > elf_brk)
      elf_brk = k;
  }


  close(bprm->fd);

  elf_entry += load_bias;
  elf_bss += load_bias;
  elf_brk += load_bias;
  start_code += load_bias;
  end_code += load_bias;
  end_data += load_bias;

  if (elf_interpreter) {
	assert(false);
#if 0
    if (interpreter_type == INTERPRETER_AOUT) {
      elf_entry = load_aout_interp(&interp_ex, interpreter_fd);
    } else {
      elf_entry = load_elf_interp(&interp_elf_ex, interpreter_fd,
                                  &interp_load_addr);
    }
#endif

    close(interpreter_fd);

    if (elf_entry == ~0UL) {
      printf("Unable to load interpreter %.128s\n", elf_interpreter);
      free(elf_interpreter);
      free(elf_phdata);

      //send_sig(SIGSEGV, current, 0);
      exit(1);
      return 0;
    }

    free(elf_interpreter);
  }

  free(elf_phdata);

  if (interpreter_type != INTERPRETER_AOUT)
    close(elf_exec_fileno);

#if 0
#ifndef VM_STACK_FLAGS
  current->executable = dget(bprm->dentry);
#endif
#endif

  bprm->p = (unsigned long)create_elf_tables((char *)bprm->p,
                        bprm->argc, bprm->envc,
                        (interpreter_type == INTERPRETER_ELF ? &elf_ex : NULL),
                        load_addr, load_bias, interp_load_addr,
                        (interpreter_type == INTERPRETER_AOUT ? 0 : 1));

#if 0
  /* N.B. passed_fileno might not be initialized? */
  if (interpreter_type == INTERPRETER_AOUT)
    current->arg_start += strlen(passed_fileno) + 1;
#endif

  current->start_brk = current->brk = elf_brk;
  current->end_code = end_code;
  current->start_code = start_code;
  current->end_data = end_data;
  current->start_stack = bprm->p;

  /* Calling set_brk effectively mmaps the pages that we need
   * for the bss and break sections
   */
  //set_brk(elf_bss, elf_brk);
  padzero(elf_bss);

  log("start_brk: %lx" , current->start_brk);
  log("end_code: %lx" , current->end_code);
  log("start_code: %lx" , current->start_code);
  log("end_data: %lx" , current->end_data);
  log("start_stack: %lx" , current->start_stack);
  log("brk: %lx" , current->brk);

  /*
   * The ABI may specify that certain registers be set up in special
   * ways (on i386 %edx is the address of a DT_FINI function, for
   * example.  This macro performs whatever initialization to
   * the regs structure is required.
   */
  ELF_PLAT_INIT((&regs));

  regs.eip = elf_entry;
  regs.esp = bprm->p;

#if 0
  if (current->flags & PF_PTRACED)
    send_sig(SIGTRAP, current, 0);
#endif


#ifndef __DEBUG__


//  dumpMemoryMap();
  log("[transfering control to Linux executable]");
  //getchar();

  assert(false);
  //ASM_EXEC_JUMP(regs);

  printf("You should never see this message!\n");

#else

  printf("execve() finished, but in debug mode. exiting...\n");

#endif

  retval = 0;
out:
  return retval;

  /* error cleanup */
out_free_dentry:
  close(interpreter_fd);

out_free_interp:
  if (elf_interpreter) {
    free(elf_interpreter);
  }

out_free_file:
  close(elf_exec_fileno);

out_free_ph:
  free(elf_phdata);
  goto out;
}
#endif

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
			log("using interpreter %s", _interpreter.c_str());
		}
	}

	_entrypoint = _elfhdr.e_entry;
	_loadaddress = 0;
}

void ElfLoader::Close()
{
	log("_phdr=%p", _phdr);
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
		log("actual load offset is %08x", loadoffset);
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

			log("raw area %08x-%08x", loadoffset + ph.p_vaddr, loadoffset + ph.p_vaddr + ph.p_memsz);
			u32 addr = MemOp::Align<0x1000>(loadoffset + ph.p_vaddr);
			u32 mlen = ph.p_memsz + MemOp::Offset<0x1000>(ph.p_vaddr);
			u32 flen = ph.p_filesz + MemOp::Offset<0x1000>(ph.p_vaddr);
			u32 off = ph.p_offset - MemOp::Offset<0x1000>(ph.p_vaddr);

			/* Ensure the entire area contains writeable memory. */

			if (prot & LINUX_PROT_WRITE)
			{
				log("writeable area %08x-%08x", addr, addr+mlen);
				do_mmap((u8*) addr, mlen, prot,
						LINUX_MAP_PRIVATE | LINUX_MAP_FIXED | LINUX_MAP_ANONYMOUS,
						_fd, off);
			}

			/* Load the actual data. */

			log("readable area %08x-%08x", addr, addr+flen);
			do_mmap((u8*) addr, flen, prot,
					LINUX_MAP_PRIVATE | LINUX_MAP_FIXED,
					_fd, off);

			/* Wipe any remaining data, if necessary --- we may have loaded a
			 * page too many.
			 */

			if (prot & LINUX_PROT_WRITE)
			{
				log("wiping from %08x-%08x", addr+flen, addr+mlen);
				memset((u8*) (addr+flen), 0, MemOp::AlignUp<0x1000>(mlen) - flen);
			}
			DumpMemory(addr+flen-64, 128);
		}
	}

	_entrypoint = _elfhdr.e_entry + loadoffset;
	_loadaddress = loadoffset + minaddr;
	log("entrypoint is %08x", _entrypoint);
}
