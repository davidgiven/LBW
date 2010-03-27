/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"
#include "syscalls.h"
#include "filesystem/RealFD.h"
#include "syscalls/mmap.h"
#include "MemOp.h"
#include <sys/mman.h>
#include <map>
#include <bitset>
#include <algorithm>

//#define VERBOSE

using std::map;
using std::bitset;
using std::min;

/* Linux wants to be able to map files to 4kB page boundaries when loading
 * executables. Alas, Interix can't do that --- it will only let us map stuff
 * to 64kB boundaries. So what follows is a hideous emulation layer where we
 * try to emulate 4kB blocks on top of 64kB blocks by mmap()ing where we can
 * and just loading code where we can't.
 *
 * For clarity, I'll refer to a 64kB block as a block and a 4kB page as a
 * page.
 */

#undef PAGE_SIZE
static const u32 BLOCK_SIZE   = 0x00010000;
static const u32 PAGE_SIZE    = 0x00001000;
static const u32 RANGE_BOTTOM = 0x08000000;
static const u32 RANGE_TOP    = 0x80000000;

static const u32 BLOCK_COUNT = (RANGE_TOP - RANGE_BOTTOM) / BLOCK_SIZE;

class Block
{
public:
	Block(u8* address):
		_address(address)
	{
	}

	virtual ~Block()
	{
	}

	u8* Address() const
	{
		return _address;
	}

	virtual void Msync(size_t length, int flags)
	{
		int i = msync(_address, length, flags);
		if (i == -1)
			throw errno;
	}

	virtual u32 GetLength() const
	{
		return 0x10000;
	}

private:
	u8* _address;
};

class FragmentedBlock : public Block
{
public:
	FragmentedBlock(u8* address):
		Block(address)
	{
		void* result = mmap(address, BLOCK_SIZE,
				PROT_READ | PROT_WRITE | PROT_EXEC,
				MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS,
				-1, 0);
		if (result != address)
		{
			throw errno;
		}

#if defined VERBOSE
		log("FragmentedBlock(%08x)", address);
#endif
		_refcount = 0;
	}

	~FragmentedBlock()
	{
		munmap(Address(), BLOCK_SIZE);
	}

	bitset<16>& Pages()
	{
		return _usage;
	}

private:
	int _refcount;
	bitset<16> _usage;
};

class MappedBlock : public Block
{
public:
	MappedBlock(u8* address, int realfd, u32 offset, u32 length,
			bool shared, bool w, bool x):
		Block(address),
		_length(length),
		_writeable(false)
	{
		int flags = MAP_FIXED;
		if (shared)
			flags |= MAP_SHARED;
		else
			flags |= MAP_PRIVATE;

		int prot = PROT_READ;
		if (w)
		{
			prot |= PROT_WRITE;
			_writeable = true;
		}
		if (x)
			prot |= PROT_EXEC;

		void* result = mmap(address, _length, prot, flags, realfd, offset);
		if (result != address)
			throw errno;
#if defined VERBOSE
		log("MappedBlock(%08x)", address);
#endif
	}

	~MappedBlock()
	{
		munmap(Address(), BLOCK_SIZE);
	}

	u32 GetLength() const
	{
		return _length;
	}

	void Msync(size_t length, int flags)
	{
		if (!_writeable)
			return;
		Block::Msync(GetLength(), flags);
	}

private:
	size_t _length;
	bool _writeable : 1;
};

class BlockStore
{
public:
	BlockStore()
	{
		memset(_blocks, 0, sizeof(_blocks));
	}

	~BlockStore()
	{
		Reset();
	}

	void Reset()
	{
		for (u32 block = 0; block < BLOCK_COUNT; block++)
		{
			delete _blocks[block];
			_blocks[block] = NULL;
		}
	}

	Block*& GetBlock(u8* address)
	{
		if ((address < (u8*)RANGE_BOTTOM) || (address >= (u8*)RANGE_TOP))
		{
			log("address %08x out of range of refcounted area", address);
			throw EINVAL;
		}

		return _blocks[((u32)address - RANGE_BOTTOM) / BLOCK_SIZE];
	}

	/* address, offset must be 64kB-aligned */
	void Map(u8* address, u32 length, int realfd, u32 offset,
					bool shared, bool w, bool x)
	{
		assert(MemOp::Aligned<BLOCK_SIZE>(address));
		//assert(MemOp::Aligned<BLOCK_SIZE>(length));
		assert(MemOp::Aligned<BLOCK_SIZE>(offset));

		try
		{
			for (u32 i = 0; i < length; i += BLOCK_SIZE)
			{
#if defined VERBOSE
				log("create mapped block %08x, offset %08x, shared=%c w=%c x=%c",
						address + i, offset + i,
						shared ? 'y' : 'n',
						w ? 'y' : 'n',
						x ? 'y' : 'n');
#endif
				Block*& block = GetBlock(address + i);
				if (block)
					delete block;
				u32 bl = length - i;
				if (bl > 0x10000)
					bl = 0x10000;
				block = new MappedBlock(address + i, realfd, offset + i, bl,
						shared, w, x);
			}
		}
		catch (int e)
		{
			log("Map() I/O error %d, trying to clean up", e);
			Unmap(address, length);
			throw e;
		}
	}

	/* address must be 64kB-aligned. */
	bitset<16>& GetPageMap(u8* address)
	{
		assert(MemOp::Aligned<BLOCK_SIZE>(address));

		Block*& block = GetBlock(address);
		if (!block)
		{
			/* No block here --- create one! */
			block = new FragmentedBlock(address);
		}

		MappedBlock* mb = dynamic_cast<MappedBlock*>(block);
		if (mb)
		{
			/* Need to convert this MappedBlock into a FragmentedBlock.
			 * This involves copying the data out, unmapping it, mapping
			 * a new one in the same place, and copying the data back in
			 * again.
			 */

			u32 length = mb->GetLength();
#if defined VERBOSE
			log("converting block at %08x+%08x from mapped to fragmented", address, length);
#endif
			u8 copybuffer[length];
			memcpy(copybuffer, address, length);

			delete block;
			FragmentedBlock* fb = new FragmentedBlock(address);
			block = fb;

			memcpy(address, copybuffer, length);
			fb->Pages().set();
		}

		FragmentedBlock* fb = dynamic_cast<FragmentedBlock*>(block);
		assert(fb);
		return fb->Pages();
	}

	/* address must be 64kB-aligned */
	void Unmap(u8* address)
	{
		assert(MemOp::Aligned<BLOCK_SIZE>(address));

		Block*& block = GetBlock(address);
		delete block;
		block = NULL;
	}

	/* address, length must be 64kB-aligned */
	void Unmap(u8* address, u32 length)
	{
		assert(MemOp::Aligned<BLOCK_SIZE>(address));
		assert(MemOp::Aligned<BLOCK_SIZE>(length));

		for (u32 i = 0; i < length; i += BLOCK_SIZE)
			Unmap(address + i);
	}

	/* address must be 4kB-aligned */
	void UsePage(u8* address)
	{
		//log("ref %08x", address);
		assert(MemOp::Aligned<PAGE_SIZE>(address));
		u8* alignedaddress = MemOp::Align<BLOCK_SIZE>(address);
		u32 page = MemOp::Offset<BLOCK_SIZE>(address) / PAGE_SIZE;

		bitset<16>& pagemap = GetPageMap(alignedaddress);
		//log("(refcount is %d)", pagemap.count());
		pagemap[page] = true;
	}

	/* address must be 4kB-aligned */
	void UnusePage(u8* address)
	{
		//log("unref %08x", address);
		assert(MemOp::Aligned<PAGE_SIZE>(address));
		u8* alignedaddress = MemOp::Align<BLOCK_SIZE>(address);
		u32 page = MemOp::Offset<BLOCK_SIZE>(address) / PAGE_SIZE;

		bitset<16>& pagemap = GetPageMap(alignedaddress);
		pagemap[page] = false;
		//log("(refcount is %d)", pagemap.count());

		if (pagemap.none())
		{
			/* This block is no longer in use, so we can nuke it. */

#if defined VERBOSE
			log("nuking block %08x", alignedaddress);
#endif
			Unmap(alignedaddress, BLOCK_SIZE);
		}
	}

private:
	Block* _blocks[BLOCK_COUNT];
};
static BlockStore blockstore;

void UnmapAll()
{
	blockstore.Reset();
}

struct linux_mmap_arg_struct
{
	u32 addr;
	u32 len;
	u32 prot;
	u32 flags;
	u32 fd;
	u32 offset;
};

u32 do_mmap(u8* addr, u32 len, u32 prot, u32 flags, int fd, u32 offset)
{
	RAIILock locked;

#if defined VERBOSE
	log("mmap(%p, %p, %p, %p, %d, %p)",
			addr, len, prot, flags, fd, offset);
#endif

	if (!MemOp::Aligned<PAGE_SIZE>(addr))
		throw EINVAL;
	if (!MemOp::Aligned<PAGE_SIZE>(offset))
		throw EINVAL;

	if (prot & LINUX_PROT_SEM)
	{
		log("mmap(): no support for PROT_SEM, ignoring");
	}

	bool w = prot & LINUX_PROT_WRITE;
	bool x = prot & LINUX_PROT_EXEC;
	bool shared = flags & LINUX_MAP_SHARED;

	if (!(flags & LINUX_MAP_FIXED))
	{
		/* This is a very nasty hack to find an unused memory area to
		 * map the block into. Rather than keep our own range allocator,
		 * we abuse Interix'. This also has the advantage that we
		 * interoperate nicely with Interix' mappings.
		 */

		void* result = mmap(addr, len,
				PROT_READ | PROT_WRITE | PROT_EXEC,
				MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS,
				-1, 0);
		if (result == MAP_FAILED)
			throw ENOMEM;

#if defined VERBOSE
		log("nonfixed map going to %08x+%08x (app preferred %08x)", result, len, addr);
#endif
		addr = (u8*) result;
		int i = munmap(result, len);
		assert(i == 0);
	}

	if (flags & LINUX_MAP_ANONYMOUS)
	{
#if defined VERBOSE
		log("anonymous");
#endif
		/* Anonymous areas are all fragmented. */

		for (u32 i = 0; i < len; i += PAGE_SIZE)
		{
			blockstore.UsePage(addr + i);
			memset(addr + i, 0, PAGE_SIZE);
		}
	}
	else
	{
#if defined VERBOSE
		log("not anonymous");
#endif
		/* If the offset and the address are 64kB-aligned, we can do a proper
		 * mmap(), which we prefer. Otherwise we have to create fragmented
		 * blocks and load the data into RAM (ick, phht).
		 */

		u8* loadaddr = addr;
		if (!Options.ForceLoad && MemOp::Aligned<BLOCK_SIZE>(addr)
				&& MemOp::Aligned<BLOCK_SIZE>(offset))
		{
#if defined VERBOSE
			log("aligned!");
#endif
			/* Only map what data the file has (or odd stuff happens). */

			struct stat st;
			fstat(fd, &st);
			int reallen = min(len, (u32) st.st_size - offset);
#if defined VERBOSE
			log("file length is %p, actually mapping %p", st.st_size, reallen);
#endif

			if (reallen > 0)
				blockstore.Map(addr, reallen, fd, offset, shared, w, x);
		}
		else
		{
#if defined VERBOSE
			log("loading %08x+%08x @%08x", loadaddr, len, offset);
#endif
			for (u32 i = 0; i < len; i += PAGE_SIZE)
			{
				blockstore.UsePage(loadaddr + i);

				int r = pread(fd, loadaddr+i, PAGE_SIZE, offset+i);
				if (r == -1)
					throw errno;
			}
		}
	}

	return (u32) addr;
}

void do_munmap(u8* addr, u32 len)
{
	if (MemOp::Aligned<BLOCK_SIZE>(addr))
	{
#if defined VERBOSE
		log("unmapping aligned area %08x+%08x", addr, len);
#endif

		u32 alignedlen = MemOp::Align<BLOCK_SIZE>(len);

		blockstore.Unmap(addr, alignedlen);
		addr += alignedlen;
		len -= alignedlen;
	}

	for (u32 i = 0; i < len; i += PAGE_SIZE)
	{
#if defined VERBOSE
		log("unref %08x", addr + i);
#endif
		blockstore.UnusePage(addr + i);
	}
}

SYSCALL(sys32_mmap)
{
	struct linux_mmap_arg_struct& ma = *(struct linux_mmap_arg_struct*) arg.a0.p;
	return do_mmap((u8*) ma.addr, ma.len, ma.prot, ma.flags, ma.fd, ma.offset);
}

SYSCALL(sys32_mmap2)
{
	u_int32_t addr = arg.a0.u;
	u_int32_t len = arg.a1.u;
	u_int32_t prot = arg.a2.u;
	u_int32_t flags = arg.a3.u;
	int32_t fd = arg.a4.s;
	u_int32_t pgoff = arg.a5.u;

	return do_mmap((u8*) addr, len, prot, flags, fd, pgoff * 4096);
}

SYSCALL(sys_unmap)
{
	void* addr = arg.a0.p;
	size_t len = arg.a1.u;

	do_munmap((u8*) addr, len);
	return 0;
}

SYSCALL(sys32_mprotect)
{
	u_int32_t addr = arg.a0.u;
	u_int32_t len = arg.a1.u;
	u_int32_t prot = arg.a2.u;

	//int iprot = convert_prot_l2i(prot);
	//int result = mprotect((void*) addr, len, iprot);
#if defined VERBOSE
	log("mprotect(%08x, %08x, %08x)", addr, len, prot);
#endif
	//return SysError(result);
	return 0;
}

SYSCALL(sys_msync)
{
	u8* addr = (u8*) arg.a0.p;
	size_t length = arg.a1.u;
	int flags = arg.a2.s;

	int iflags = 0;
	if (flags & LINUX_MS_ASYNC)
		iflags |= MS_ASYNC;
	if (flags & LINUX_MS_SYNC)
		iflags |= MS_SYNC;
	if (flags & LINUX_MS_INVALIDATE)
		iflags |= MS_INVALIDATE;

	if (!MemOp::Aligned<0x1000>(addr))
		throw EINVAL;

	length += MemOp::Offset<0x10000>(addr);
	addr = MemOp::Align<0x10000>(addr);

	for (size_t i = 0; i < length; i += 0x10000)
	{
		Block*& b = blockstore.GetBlock(addr + i);

		try
		{
			b->Msync(b->GetLength(), iflags);
		}
		catch (int e)
		{
			if (e == EIO)
				Warning("msync() reported EIO --- possibly spurious, so ignoring");
			else
				throw e;
		}
	}
	return 0;
}

SYSCALL(sys_mlock)
{
	void* addr = arg.a0.p;
	size_t length = arg.a1.u;
	throw ENOSYS;
}

SYSCALL(sys_mremap)
{
	Warning("mremap() not implemented yet");
	throw ENOSYS;
}

void MakeWriteable(u8* addr, u32 length)
{
	u8* topaddr = MemOp::AlignUp<0x10000>(addr + length - 1);
	addr = MemOp::Align<0x10000>(addr);

	blockstore.GetPageMap(addr);
	if (topaddr != addr)
		blockstore.GetPageMap(topaddr);
}
