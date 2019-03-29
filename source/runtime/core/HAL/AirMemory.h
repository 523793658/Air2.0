#pragma once
#include "CoreType.h"

#include "HAL/MemoryBase.h"
#include "PlatformMemory.h"
#include <type_traits>
namespace Air
{
	struct CORE_API Memory
	{
	public:
		static FORCEINLINE void * systemMalloc(SIZE_T size)
		{
			return ::malloc(size);
		}

		static FORCEINLINE void systemFree(void* ptr)
		{
			::free(ptr);
		}

		static void* malloc(SIZE_T Count, uint32 alignment = DEFAULT_ALIGNMENT);

		static void* realloc(void* original, SIZE_T count, uint32 alignment = DEFAULT_ALIGNMENT);

		static void free(void* original);

		static FORCEINLINE void* memmove(void* dest, const void* src, SIZE_T count)
		{
			return PlatformMemory::Memmove(dest, src, count);
		}

		static FORCEINLINE void* memzero(void* dest, SIZE_T count)
		{
			return PlatformMemory::Memzero(dest, count);
		}

		static FORCEINLINE void memswap(void* src, void* dest, SIZE_T count)
		{
			return PlatformMemory::memswap(src, dest, count);
		}

		template<class T>
		static FORCEINLINE void memzero(T& src)
		{
			static_assert(!std::_Is_pointer<T>::value, "For pointers use the two parameters function");
			memzero(&src, sizeof(T));
		}

		static FORCEINLINE void* Memset(void* dest, uint8 val, SIZE_T count)
		{
			return PlatformMemory::Memset(dest, val, count);
		}

		static FORCEINLINE int32 memcmp(const void* buf1, const void* buf2, SIZE_T count)
		{
			return PlatformMemory::Memcmp(buf1, buf2, count);
		}

		static FORCEINLINE void* memcpy(void *dst, const void* src, SIZE_T count)
		{
			return PlatformMemory::Memcpy(dst, src, count);
		}

		template<typename T>
		static FORCEINLINE void memcpy(T& dest, const T& src)
		{
			static_assert(!std::_Is_pointer<T>::value, "For pointers use the three parameters function");
			memcpy(&dest, &src, sizeof(T));

		}

		static void setupTLSCachesOnCurrentThread();

		static void clearAndDisableTLSCachesOnCurrentThread();

		static SIZE_T quantizeSize(SIZE_T count, uint32 alignment = DEFAULT_ALIGNMENT);

	private:

		static void GCreateMalloc();

		static void* mallocExternal(SIZE_T count, uint32 alignment = DEFAULT_ALIGNMENT);

		static void* reallocExternal(void* original, SIZE_T count, uint32 alignment = DEFAULT_ALIGNMENT);

		static void freeExternal(void* original);

		static SIZE_T getAllocSizeExternal(void* original);

		static SIZE_T quantizeSizeExternal(SIZE_T count, uint32 alignment = DEFAULT_ALIGNMENT);
	};
}