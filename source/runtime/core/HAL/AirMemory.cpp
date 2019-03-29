#include "AirMemory.h"
#include "CoreGlobals.h"
#include "PlatformMemory.h"
#include <functional>
namespace Air
{
	CORE_API std::function<void(int32)>* GGameThreadMallocHook = nullptr;

	void doGameThreadHook(int32 index)
	{
		if (GIsRunning && GGameThreadMallocHook && isInGameThread())
		{
			(*GGameThreadMallocHook)(index);
		}
	}

	void* UseSystemMallockForNew::operator new(size_t size)
	{
		return Memory::systemMalloc(size);
	}

	void UseSystemMallockForNew::operator delete(void* ptr)
	{
		Memory::systemFree(ptr);
	}

	void* UseSystemMallockForNew::operator new[](size_t size)
	{
		return Memory::systemMalloc(size);
	}

	void UseSystemMallockForNew::operator delete[](void*ptr)
	{
		Memory::systemFree(ptr);
	}



	void Memory::GCreateMalloc()
	{
		GMalloc = PlatformMemory::baseAllocator();
	}

	void* Memory::mallocExternal(SIZE_T count, uint32 alignment)
	{
		if (!GMalloc)
		{
			GCreateMalloc();
		}
		return GMalloc->malloc(count, alignment);
	}

	void* Memory::reallocExternal(void* original, SIZE_T count, uint32 alignment)
	{
		if (!GMalloc)
		{
			GCreateMalloc();
		}
		return GMalloc->realloc(original, count, alignment);
	}

	void Memory::freeExternal(void* original)
	{
		if (!GMalloc)
		{
			GCreateMalloc();
		}
		if (original)
		{
			GMalloc->free(original);
		}
	}

	SIZE_T Memory::getAllocSizeExternal(void* original)
	{
		if (!GMalloc)
		{
			GCreateMalloc();
		}
		SIZE_T size = 0;
		return GMalloc->getAllocationSize(original, size) ? size : 0;
	}


	SIZE_T Memory::quantizeSizeExternal(SIZE_T count, uint32 alignment)
	{
		if (!GMalloc)
		{
			GCreateMalloc();
		}
		return GMalloc->quantizeSize(count, alignment);
	}

	void Memory::setupTLSCachesOnCurrentThread()
	{
		if (!GMalloc)
		{
			GCreateMalloc();
		}
		return GMalloc->setupTLSCachesOnCurrentThread();
	}


	void Memory::clearAndDisableTLSCachesOnCurrentThread()
	{
		if (GMalloc)
		{
			GMalloc->clearAndDisableTLSCachesOnCurrentThread();
		}
	}


}

#include "HAL/Memory.inl"