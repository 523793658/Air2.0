#pragma once
#include "CoreType.h"
#include "HAL/MemoryBase.h"
namespace Air
{
	struct GenericPlatformMemoryConstants
	{
		SIZE_T mTotalPhysical;
		SIZE_T mTotalVirtual;
		SIZE_T mPageSize;
		SIZE_T mOsAllocationGranularity;
		uint64 mAddressLimit;
		uint32 mTotalPhysicalGB;

		GenericPlatformMemoryConstants()
			:mTotalPhysical(0)
			,mTotalVirtual(0)
			,mPageSize(0)
			,mOsAllocationGranularity(0)
			,mAddressLimit((uint64)0xffffffff + 1)
			,mTotalPhysicalGB(1)
		{}

		GenericPlatformMemoryConstants(const GenericPlatformMemoryConstants& other)
			:mTotalPhysical(other.mTotalPhysical)
			, mTotalVirtual(other.mTotalVirtual)
			, mPageSize(other.mPageSize)
			, mOsAllocationGranularity(other.mOsAllocationGranularity)
			, mAddressLimit(other.mAddressLimit)
			, mTotalPhysicalGB(other.mTotalPhysicalGB)
		{}
	};

	typedef GenericPlatformMemoryConstants PlatformMemoryConstants;


	struct CORE_API GenericPlatformMemory
	{
	public:
		static Malloc* baseAllocator();

		enum EMemoryAllocatorToUse
		{
			Ansi		//Default C allocator
		};


		static FORCEINLINE void* Memmove(void* dest, const void* src, SIZE_T count)
		{
			return memmove(dest, src, count);
		}

		static FORCEINLINE void* Memzero(void* dest, SIZE_T count)
		{
			return memset(dest, 0, count);
		}

		static FORCEINLINE void memswap(void* ptr1, void* ptr2, SIZE_T count)
		{
			switch (count)
			{
			case 0:
				break;
			case 1:
				valswap(*(uint8*)ptr1, *(uint8*)ptr2);
				break;
			case 2:
				valswap(*(uint16*)ptr1, *(uint16*)ptr2);
				break;
			case 3:
				valswap(*((uint16*&)ptr1)++, *((uint16*&)ptr2)++);
				valswap(*(uint8*)ptr1, *(uint8*)ptr2);
				break;
			case 4:
				valswap(*(uint32*)ptr1, *(uint32*)ptr2);
				break;
			case 5:
				valswap(*((uint32*&)ptr1)++, *((uint32*&)ptr2)++);
				valswap(*(uint8*)ptr1, *(uint8*)ptr2);
				break;
			case 6:
				valswap(*((uint32*&)ptr1)++, *((uint32*&)ptr2)++);
				valswap(*(uint16*)ptr1, *(uint16*)ptr2);
				break;
			case 7:
				valswap(*((uint32*&)ptr1)++, *((uint32*&)ptr2)++);
				valswap(*((uint16*&)ptr1)++, *((uint16*&)ptr2)++);
				valswap(*(uint8*)ptr1, *(uint8*)ptr2);
				break;
			case 8:
				valswap(*((uint32*&)ptr1)++, *((uint32*&)ptr2)++);
				valswap(*(uint32*)ptr1, *(uint32*)ptr2);

				break;
			default:
				memswapGreaterThan8(ptr1, ptr2, count);
				break;
			}
		}

		static FORCEINLINE void* Memset(void* dest, uint8 val, SIZE_T count)
		{
			return memset(dest, val, count);
		}

		static FORCEINLINE int32 Memcmp(const void* buf1, const void* buf2, SIZE_T count)
		{
			return memcmp(buf1, buf2, count);
		}

		static FORCEINLINE void* Memcpy(void* dst, const void* src, SIZE_T count)
		{
			return memcpy(dst, src, count);
		}

		static const PlatformMemoryConstants& getConstants();

		static FORCEINLINE bool supportsFastVRAMemory()
		{
			return false;
		}

		static EMemoryAllocatorToUse mAllocatorToUse;
	private:
		template<typename T>
		static FORCEINLINE void valswap(T& A, T& B)
		{
			T tmp = A;
			A = B;
			B = tmp;
		}

		static void memswapGreaterThan8(void* ptr1, void* ptr2, SIZE_T size);
	};
} 