#include "globals.h"
#include "syscalls.h"
#include "filesystem/RawFD.h"
#include "syscalls/mmap.h"
#include <sys/mman.h>
#include <map>
#include <bitset>

//#define VERBOSE

using std::map;
using std::bitset;

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

template <int SIZE, typename T> static T align(T address)
{
	return (T)((u32)address & ~(SIZE-1));
}

template <int SIZE, typename T> static u32 offset(T address)
{
	return (u32)address & (SIZE-1);
}

template <int SIZE, typename T> static bool aligned(T address)
{
	return offset<SIZE>(address) == 0;
}

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
			throw errno;

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
	MappedBlock(u8* address, int realfd, u32 offset,
			bool shared, bool w, bool x):
		Block(address)
	{
		int flags = MAP_FIXED;
		if (shared)
			flags |= MAP_SHARED;
		else
			flags |= MAP_PRIVATE;

		int prot = PROT_READ;
		if (w)
			prot |= PROT_WRITE;
		if (x)
			prot |= PROT_EXEC;

		void* result = mmap(address, BLOCK_SIZE, prot, flags, realfd, offset);
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
			_blocks[block] = 0;
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

	/* address, length, offset must all be 64kB-aligned */
	void Map(u8* address, u32 length, int realfd, u32 offset,
					bool shared, bool w, bool x)
	{
		assert(aligned<BLOCK_SIZE>(address));
		assert(aligned<BLOCK_SIZE>(length));
		assert(aligned<BLOCK_SIZE>(offset));

		try
		{
			for (u32 i = 0; i < length; i += BLOCK_SIZE)
			{
#if defined VERBOSE
				log("create mapped block %08x", address + i);
#endif
				Block*& block = GetBlock(address + i);
				if (block)
					delete block;
				block = new MappedBlock(address + i, realfd, offset + i,
						shared, w, x);
			}
		}
		catch (int e)
		{
			log("Map() I/O error %d, trying to clean up");
			Unmap(address, length);
			throw e;
		}
	}

	/* address must be 64kB-aligned. */
	bitset<16>& GetPageMap(u8* address)
	{
		assert(aligned<BLOCK_SIZE>(address));

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

			log("converting block at %08x from mapped to fragmented", address);
			u8 copybuffer[BLOCK_SIZE];
			memcpy(copybuffer, address, BLOCK_SIZE);

			delete block;
			FragmentedBlock* fb = new FragmentedBlock(address);
			block = fb;

			memcpy(address, copybuffer, BLOCK_SIZE);
			fb->Pages().set();
		}

		FragmentedBlock* fb = dynamic_cast<FragmentedBlock*>(block);
		assert(fb);
		return fb->Pages();
	}

	/* address must be 64kB-aligned */
	void Unmap(u8* address)
	{
		assert(aligned<BLOCK_SIZE>(address));

		Block*& block = GetBlock(address);
		delete block;
		block = NULL;
	}

	/* address, length must be 64kB-aligned */
	void Unmap(u8* address, u32 length)
	{
		assert(aligned<BLOCK_SIZE>(address));
		assert(aligned<BLOCK_SIZE>(length));

		for (u32 i = 0; i < length; i += BLOCK_SIZE)
			Unmap(address + i);
	}

	/* address must be 4kB-aligned */
	void UsePage(u8* address)
	{
		//log("ref %08x", address);
		assert(aligned<PAGE_SIZE>(address));
		u8* alignedaddress = align<BLOCK_SIZE>(address);
		u32 page = offset<BLOCK_SIZE>(address) / PAGE_SIZE;

		bitset<16>& pagemap = GetPageMap(alignedaddress);
		//log("(refcount is %d)", pagemap.count());
		pagemap[page] = true;
	}

	/* address must be 4kB-aligned */
	void UnusePage(u8* address)
	{
		//log("unref %08x", address);
		assert(aligned<PAGE_SIZE>(address));
		u8* alignedaddress = align<BLOCK_SIZE>(address);
		u32 page = offset<BLOCK_SIZE>(address) / PAGE_SIZE;

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

struct linux_mmap_arg_struct
{
	u32 addr;
	u32 len;
	u32 prot;
	u32 flags;
	u32 fd;
	u32 offset;
};

u32 do_mmap(u8* addr, u32 len, u32 prot, u32 flags, Ref<FD>& ref, u32 offset)
{
	RAIILock locked;

	if (!aligned<PAGE_SIZE>(addr))
		throw EINVAL;
	if (!aligned<PAGE_SIZE>(offset))
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
		int realfd = ref->GetRealFD();

		/* If the offset and the address are 64kB-aligned, we can do a proper
		 * mmap(), which we prefer. Otherwise we have to create fragmented
		 * blocks and load the data into RAM (ick, phht).
		 */

		u8* loadaddr = addr;
		if (aligned<BLOCK_SIZE>(addr) && aligned<BLOCK_SIZE>(offset))
		{
#if defined VERBOSE
			log("aligned!");
#endif
			/* ...although remember we can only mmap() whole blocks. */

			u32 alignedlen = align<BLOCK_SIZE>(len);

			blockstore.Map(addr, alignedlen, realfd, offset, shared, w, x);

			/* The rest gets loaded into a fragmented block. */

			len -= alignedlen;
			loadaddr += alignedlen;
			offset += alignedlen;
		}

#if defined VERBOSE
		log("loading %08x+%08x @%08x", loadaddr, len, offset);
#endif
		for (u32 i = 0; i < len; i += PAGE_SIZE)
		{
			blockstore.UsePage(loadaddr + i);

			int r = pread(realfd, loadaddr+i, PAGE_SIZE, offset+i);
			if (r == -1)
				throw errno;
		}
	}

	return (u32) addr;
}

u32 do_mmap(u8* addr, u32 len, u32 prot, u32 flags, u32 fd, u32 offset)
{
	Ref<FD> ref;
	if (!(flags & LINUX_MAP_ANONYMOUS))
		ref = FD::Get(fd);
	return do_mmap(addr, len, prot, flags, ref, offset);
}

void do_munmap(u8* addr, u32 len)
{
	if (aligned<BLOCK_SIZE>(addr))
	{
#if defined VERBOSE
		log("unmapping aligned area %08x+%08x", addr, len);
#endif

		u32 alignedlen = align<BLOCK_SIZE>(len);

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
	log("mprotect(%08x, %08x, %08x)", addr, len, prot);
	//return SysError(result);
	return 0;
}
