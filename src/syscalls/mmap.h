#ifndef SYSCALLS_MMAP_H
#define SYSCALLS_MMAP_H

#define LINUX_PROT_READ	0x1		/* page can be read */
#define LINUX_PROT_WRITE	0x2		/* page can be written */
#define LINUX_PROT_EXEC	0x4		/* page can be executed */
#define LINUX_PROT_SEM	0x8		/* page may be used for atomic ops */
#define LINUX_PROT_NONE	0x0		/* page can not be accessed */

#define LINUX_MAP_SHARED	0x01		/* Share changes */
#define LINUX_MAP_PRIVATE	0x02		/* Changes are private */
#define LINUX_MAP_TYPE	0x0f		/* Mask for type of mapping */
#define LINUX_MAP_FIXED	0x10		/* Interpret addr exactly */
#define LINUX_MAP_ANONYMOUS	0x20		/* don't use a file */

extern u32 do_mmap(u8* addr, u32 len, u32 prot, u32 flags, Ref<FD>& fd, u32 offset);
extern void do_munmap(u8* addr, u32 len);

#endif
