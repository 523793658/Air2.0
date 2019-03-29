#pragma once
#include "CoreType.h"

#include "MemoryBase.h"
#if defined(_MSC_VER)
#define USE_ALIGNED_MALLOC 1
#else
#define USE_ALIGNED_MALLOC 0
#endif



namespace Air
{
	class MallocAnsi : public Malloc
	{
	public:
		MallocAnsi();

		virtual void* malloc(SIZE_T size, uint32 alignment = DEFAULT_ALIGNMENT) override
		{
			IncrementTotalMallocCalls();

			alignment = std::max(size >= 16 ? (uint32)16 : (uint32)8, alignment);

#if USE_ALIGNED_MALLOC
			void *result = _aligned_malloc(size, alignment);
#else
			void* ptr = malloc(size + alignment + sizeof(void*) + sizeof(SIZE_T));
			void* result = align((uint8*)ptr + sizeof(void*) + sizeof(SIZE_T), alignment);

			*((void**)((uint8*)result - sizeof(void*))) = ptr;
			*((SIZE_T*)((uint8*)result - sizeof(void*) - sizeof(SIZE_T))) = size;
#endif
			BOOST_ASSERT(result != nullptr, "OutOfMemory");
			return result;
		}

		virtual void* realloc(void* ptr, SIZE_T count, uint32 alignment = DEFAULT_ALIGNMENT) override
		{
			incrementTotalReallocCalls();
			void* result;
			alignment = std::max(count >= 16 ? (uint32)16 : (uint32)8, alignment);

#if USE_ALIGNED_MALLOC
			if (ptr && count)
			{
				result = _aligned_realloc(ptr, count, alignment);
			}
			else if(ptr == nullptr)
			{
				result = _aligned_malloc(count, alignment);
			}
			else
			{
				_aligned_free(ptr);
				result = nullptr;
			}
#else
			if (ptr && count)
			{
				result = malloc(count, alignment);
				SIZE_T ptrSize = 0;
				getAllocationSize(ptr, ptrSize);
				memcpy(result, ptr, std::min(count, ptrSize));
				free(ptr);
			}
			else if (ptr == nullptr)
			{
				result = malloc(count, alignment);
			}
			else
			{
				free(*((void**)((uint8*)ptr - sizeof(void*))));
				result = nullptr;
			}
#endif
			if(result == nullptr && count != 0)
			{
				BOOST_ASSERT(false, "out of memory");
			}
			return result;
		}

		virtual void free(void* ptr) override
		{
			incrementTotalFreeCalls();
#if USE_ALIGNED_MALLOC
			_aligned_free(ptr);
#else
			if (ptr)
			{
				free(*((void**)((uint8*)ptr - sizeof(void*))));
			}
#endif
		}

		virtual bool getAllocationSize(void* original, SIZE_T & sizeOut) override
		{
			if (!original)
			{
				return false;
			}
#if USE_ALIGNED_MALLOC
			sizeOut = _aligned_msize(original, 16, 0);
#else
			sizeOut = *((SIZE_T*)((uint8*)original - sizeof(void*) - sizeof(SIZE_T)));
#endif
			return true;
		}
	};
}