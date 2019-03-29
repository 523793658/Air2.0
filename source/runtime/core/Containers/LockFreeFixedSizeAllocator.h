#pragma once
#include "CoreType.h"
#include "Misc/NoopCounter.h"
#include "Containers/LockFreeListImpl.h"
namespace Air
{
	template<int32 SIZE, int TPaddingForCacheTontention, typename TTRackingCounter = NoopCounter>
	class TLockFreeFixedSizeAllocator_TLSCache : public TLockFreeFixedSizeAllocator_TLSCacheBase<SIZE, TLockFreePointerListUnordered<void*, TPaddingForCacheTontention>, TTRackingCounter>
	{

	};

	template<class T, int TPaddingForCacheContention>
	class TLockFreeClassAllocator_TLSCache : private TLockFreeFixedSizeAllocator_TLSCache<sizeof(T), TPaddingForCacheContention>
	{
	public:
		void* allocate()
		{
			return TLockFreeFixedSizeAllocator_TLSCache<sizeof(T), TPaddingForCacheContention>::allocate();
		}

		T* New()
		{
			return new (allocate())T();
		}

		void free(T* item)
		{
			item->~T();
			TLockFreeFixedSizeAllocator_TLSCache<sizeof(T), TPaddingForCacheContention>::free(item);
		}
	};

	template<int32 SIZE, int TPaddingForCacheTontention, typename TTrackingCounter = NoopCounter>
	class TLockFreeFixedSizeAllocator
	{
	public:

		~TLockFreeFixedSizeAllocator()
		{
			while (void* mem = mFreeList.pop())
			{
				Memory::free(mem);
				mNumFree.decrement();
			}
		}

		void* allocate()
		{
			mNumUsed.increment();
			void* memory = mFreeList.pop();
			if (memory)
			{
				mNumFree.decrement();
			}
			else
			{
				memory = Memory::malloc(SIZE);
			}
			return memory;
		}

		void free(void* item)
		{
			mNumUsed.decrement();
			mFreeList.push(item);
			mNumFree.increment();
		}

		const TTrackingCounter& getNumUsed() const
		{
			return mNumUsed;
		}

		const TTrackingCounter& getNumFree() const
		{
			return mNumFree;
		}

	private:
		TLockFreePointerListUnordered<void, TPaddingForCacheTontention> mFreeList;

		TTrackingCounter	mNumFree;
		TTrackingCounter	mNumUsed;
	};
}