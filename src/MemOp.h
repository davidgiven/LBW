/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef MEMOP_H
#define MEMOP_H

class MemOp
{
public:
	template <typename T, typename S> static T LoadAndAdvance(S& reg)
	{
		T value = *(T*) reg;
		reg += sizeof(T);
		return value;
	}

	template <typename T, typename S> static void Store(T value, S address)
	{
		*(T*)address = value;
	}

	template <typename T, typename S> static T Load(S address)
	{
		return *(T*)address;
	}

	template <int SIZE, typename T> static inline T Align(T address)
	{
		return (T)((u32)address & ~(SIZE-1));
	}

	template <int SIZE, typename T> static inline T AlignUp(T address)
	{
		return (T)(((u32)address + (SIZE-1)) & ~(SIZE-1));
	}

	template <int SIZE, typename T> static inline u32 Offset(T address)
	{
		return (u32)address & (SIZE-1);
	}

	template <int SIZE, typename T> static inline bool Aligned(T address)
	{
		return Offset<SIZE>(address) == 0;
	}


};

#endif
