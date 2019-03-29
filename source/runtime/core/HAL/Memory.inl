#define  MEMORY_INLINE_GMalloc	GMalloc

#ifndef FMEMORY_INLINE_FUNCTION_DECORATOR
#define FMEMORY_INLINE_FUNCTION_DECORATOR
#endif

namespace Air
{
	struct Memory;
	void* Memory::malloc(SIZE_T Count, uint32 alignment /* = DEFAULT_ALIGNMENT */)
	{
		if (!MEMORY_INLINE_GMalloc)
		{
			mallocExternal(Count, alignment);
		}
		doGameThreadHook(0);
		return MEMORY_INLINE_GMalloc->malloc(Count, alignment);
	}


	void* Memory::realloc(void* original, SIZE_T count, uint32 alignment)
	{
		if (!MEMORY_INLINE_GMalloc)
		{
			return reallocExternal(original, count, alignment);
		}
		return MEMORY_INLINE_GMalloc->realloc(original, count, alignment);
	}

	void Memory::free(void* original)
	{
		if (!MEMORY_INLINE_GMalloc)
		{
			freeExternal(original);
			return;
		}
		MEMORY_INLINE_GMalloc->free(original);
	}

	FMEMORY_INLINE_FUNCTION_DECORATOR SIZE_T Memory::quantizeSize(SIZE_T count, uint32 alignment /* = DEFAULT_ALIGNMENT */)
	{
		if (!MEMORY_INLINE_GMalloc)
		{
			return count;
		}
		return MEMORY_INLINE_GMalloc->quantizeSize(count, alignment);
	}

}