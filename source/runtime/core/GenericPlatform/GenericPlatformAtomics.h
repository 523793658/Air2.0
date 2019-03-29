#pragma once
#include "CoreType.h"

namespace Air
{
	struct MS_ALIGN(16) FInt128
	{
		int64 Low;
		int64 High;
	}GCC_ALIGN(16);


	struct GenericPlatformAtomics
	{
#if 0

#endif

		static FORCEINLINE bool CanUseCompareExchange128()
		{
			return false;
		}


	protected:
		static inline bool isAligned(const volatile void* ptr, const uint32 alignment = sizeof(void*))
		{
			return !(PTRINT(ptr) & (alignment - 1));
		}
	};
}