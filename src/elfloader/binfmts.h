/* based on linux-2.2.16/include/linux/binfmts.h */
#ifndef _LINUX_BINFMTS_H
#define _LINUX_BINFMTS_H


struct pt_regs {
  long eax;
  long ebx;
  long ecx;
  long edx;
  long esi;
  long edi;
  long ebp;
  long esp;
  long eip;
  
#if 0  
  int  xds;
  int  xes;
  long orig_eax;
  int  xcs;
  long eflags;
  int  xss;
#endif  
};

/*
 * MAX_ARG_PAGES defines the number of pages allocated for arguments
 * and envelope for the new program. 32 should suffice, this gives
 * a maximum env+arg of 128kB w/4KB pages!
 */
#define MAX_ARG_PAGES 32


/*
 * This structure is used to hold the arguments that are used when 
 * loading binaries.  It's based on the structure from the Linux 2.2.x kernel
 * with the same name.
 */
struct linux_binprm{
	char buf[128];
	unsigned long page[MAX_ARG_PAGES];
	int argc, envc;
	
	int fd;

	unsigned long p;
	int sh_bang;
	int e_uid, e_gid;
	char *filename;	/* Name of binary */
	unsigned long loader, exec;
};


/*
 * This structure defines the functions that are used to load the binary formats that
 * linux accepts.
 */
struct linux_binfmt {
	struct linux_binfmt * next;
	int (*load_binary)(struct linux_binprm *);
	int (*load_shlib)(int fd);
};

int register_binfmt(struct linux_binfmt *);
int read_exec(int fd, unsigned long offset, 
                     char *addr, unsigned long count, int to_kmem);  
unsigned long copy_strings(int argc,char ** argv,unsigned long *page,
                                  unsigned long p);
void remove_arg_zero(struct linux_binprm *bprm);
unsigned long setup_arg_pages(unsigned long p, struct linux_binprm * bprm);
int search_binary_handler(struct linux_binprm *bprm);


extern int init_exe_binfmt(void);
extern int init_elf_binfmt(void);
extern int init_script_binfmt(void);


#endif /* _LINUX_BINFMTS_H */
