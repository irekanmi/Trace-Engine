#include "pch.h"

#include "memory.h"
#include "core/Enums.h"

namespace trace {



	uintptr_t AlignAddress(uintptr_t addr, uint32_t align)
	{

		uint64_t mask = align - 1;
		TRC_ASSERT((align & mask) == 0, "alignment most be a power of 2");
		
		return ( addr + mask ) & ~mask;
	}

	void* AllocAligned(uint32_t size_in_bytes, uint32_t align)
	{
		uint32_t actual_bytes = size_in_bytes + align;

		unsigned char* pRawMem = new unsigned char[actual_bytes];

		uintptr_t res = (uintptr_t)pRawMem;

		unsigned char* aligned_addr = (unsigned char*)AlignAddress(res, align);
		if (aligned_addr == pRawMem)
			aligned_addr += align;

		ptrdiff_t shift = aligned_addr - pRawMem;
		TRC_ASSERT((shift > 0) && (shift <= 256), "Alignment most not be greater than 256");
		aligned_addr[-1] = static_cast<unsigned char>(shift & 0xFF);

		return aligned_addr;
	}

	void FreeAligned(void* ptr)
	{

		if (ptr)
		{
			unsigned char* aligned_addr = reinterpret_cast<unsigned char*>(ptr);

			ptrdiff_t shift = aligned_addr[-1];
			if (shift == 0)
				shift = 256;

			unsigned char* pRawMem = aligned_addr - shift;

			delete[] pRawMem;
		}

	}

}