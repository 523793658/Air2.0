#include "GenericPlatform/GenericPlatformMemory.h"
#include "HAL/MallocAnsi.h"
#include "Math/Math.h"
namespace Air
{
	GenericPlatformMemory::EMemoryAllocatorToUse GenericPlatformMemory::mAllocatorToUse = GenericPlatformMemory::EMemoryAllocatorToUse::Ansi;


	Malloc* GenericPlatformMemory::baseAllocator()
	{
		return new MallocAnsi();
	}

	const PlatformMemoryConstants& GenericPlatformMemory::getConstants()
	{
		static PlatformMemoryConstants MemoryConstants;
		return MemoryConstants;
	}

	void GenericPlatformMemory::memswapGreaterThan8(void* RESTRICT ptr1, void* RESTRICT ptr2, SIZE_T size)
	{
		union PtrUnion
		{
			void* ptrVoid;
			uint8* ptr8;
			uint16* ptr16;
			uint32* ptr32;
			uint64* ptr64;
			UPTRINT PtrUnit;
		};

		PtrUnion union1 = { ptr1 };
		PtrUnion union2 = { ptr2 };

		BOOST_ASSERT(union1.ptrVoid &&  union2.ptrVoid);

		BOOST_ASSERT(size > 8);
		if (union1.PtrUnit & 1)
		{
			valswap(*union1.ptr8++, *union2.ptr8++);
			size -= 1;
		}
		if (union1.PtrUnit & 2)
		{
			valswap(*union1.ptr16++, *union2.ptr16++);
			size -= 2;
		}
		if (union1.PtrUnit & 4)
		{
			valswap(*union1.ptr32++, *union2.ptr32++);
			size -= 4;
		}
		uint32 commonAlignment = Math::min(Math::countTrainlingZeros(union1.PtrUnit - union2.PtrUnit), 3u);
		switch (commonAlignment)
		{
		default:
			for (; size >= 8; size -= 8)
			{
				valswap(*union1.ptr64++, *union2.ptr64++);
			}
		case 2:
			for (; size >= 4; size -= 4)
			{
				valswap(*union1.ptr32++, *union2.ptr32++);
			}
		case 1:
			for (; size >= 2; size -= 2)
			{
				valswap(*union1.ptr16++, *union2.ptr16++);
			}
		case 0:
			for (; size >= 1; size -= 1)
			{
				valswap(*union1.ptr8++, *union2.ptr8++);
			}
		}
	}
}