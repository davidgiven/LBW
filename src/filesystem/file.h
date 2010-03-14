/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef LINUX_FILE_H
#define LINUX_FILE_H

#define LINUX_O_ACCMODE	    00000003
#define LINUX_O_RDONLY	    00000000
#define LINUX_O_WRONLY	    00000001
#define LINUX_O_RDWR		00000002
#define LINUX_O_CREAT		00000100	/* not fcntl */
#define LINUX_O_EXCL		00000200	/* not fcntl */
#define LINUX_O_NOCTTY	    00000400	/* not fcntl */
#define LINUX_O_TRUNC		00001000	/* not fcntl */
#define LINUX_O_APPEND	    00002000
#define LINUX_O_NONBLOCK	00004000
#define LINUX_O_SYNC		00010000
#define LINUX_O_DIRECT	    00040000	/* direct disk access hint */
#define LINUX_O_LARGEFILE	00100000
#define LINUX_O_DIRECTORY	00200000	/* must be a directory */
#define LINUX_O_NOFOLLOW	00400000	/* don't follow links */
#define LINUX_O_NOATIME	    01000000
#define LINUX_O_CLOEXEC	    02000000	/* set close_on_exec */

extern u_int32_t FileFlagsI2L(u_int32_t flags);
extern u_int32_t FileFlagsL2I(u_int32_t flags);

#define LINUX_F_DUPFD         0       /* dup */
#define LINUX_F_GETFD         1       /* get close_on_exec */
#define LINUX_F_SETFD         2       /* set/clear close_on_exec */
#define LINUX_F_GETFL         3       /* get file->f_flags */
#define LINUX_F_SETFL         4       /* set file->f_flags */
#define LINUX_F_GETLK         5
#define LINUX_F_SETLK         6
#define LINUX_F_SETLKW        7

#define LINUX_F_SETOWN        8       /*  for sockets. */
#define LINUX_F_GETOWN        9       /*  for sockets. */
#define LINUX_F_SETSIG        10      /*  for sockets. */
#define LINUX_F_GETSIG        11      /*  for sockets. */

#define LINUX_F_GETLK64       12      /*  using 'struct flock64' */
#define LINUX_F_SETLK64       13
#define LINUX_F_SETLKW64      14

#define LINUX_FD_CLOEXEC      1       /* actually anything with low bit set goes */

#pragma pack(push, 1)
struct linux_compat_stat
{
	u16	st_dev;
	u16 __pad1;
	u32	st_ino;
	u16	st_mode;
	u16	st_nlink;
	u16	st_uid;
	u16	st_gid;
	u16	st_rdev;
	u16	__pad2;
	u32	st_size;
	u32	st_blksize;
	u32	st_blocks;
	u32	st_atime;
	u32	st_atime_nsec;
	u32	st_mtime;
	u32	st_mtime_nsec;
	u32	st_ctime;
	u32	st_ctime_nsec;
	u32	__unused4;
	u32	__unused5;
};

struct linux_stat64  {
	u64 st_dev;	    /* Device.  */
	u64 __pad0;
	u32 st_mode;	/* File mode.  */
	u32 st_nlink;	/* Link count.  */
	u32 st_uid;		/* User ID of the file's owner.  */
	u32 st_gid;		/* Group ID of the file's group. */
	u64 st_rdev;	/* Device number, if device.  */
	u32 __pad1;
	s64 st_size;	/* Size of file, in bytes.  */
	u32 st_blksize;	/* Optimal block size for I/O.  */
	u64 st_blocks;	/* Number 512-byte blocks allocated. */
	u32 st_atime;	/* Time of last access.  */
	u32 st_atime_nsec;
	u32 st_mtime;	/* Time of last modification.  */
	u32 st_mtime_nsec;
	u32 st_ctime;	/* Time of last status change.  */
	u32 st_ctime_nsec;
	u64 st_ino;
};
#pragma pack(pop)

extern void Convert(struct stat& is, struct linux_stat64& ls);

#define LINUX_AT_FDCWD -100
#define LINUX_AT_SYMLINK_NOFOLLOW    0x100
#define LINUX_AT_REMOVEDIR           0x200
#define LINUX_AT_SYMLINK_FOLLOW      0x400
#define LINUX_AT_EACCESS             0x200

#endif
