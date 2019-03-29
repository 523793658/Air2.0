#pragma once

#include "CoreType.h"
#include "boost/noncopyable.hpp"
#include "HAL/ThreadSafeCounter.h"
#include "Misc/NoopCounter.h"
#include "HAL/AirMemory.h"
#include "HAL/PlatformTLS.h"
#include "HAL/PlatformMisc.h"
namespace Air
{

#define MONITOR_LINK_ALLOCATION (0)
#if MONITOR_LINK_ALLOCATION == 1
	typedef ThreadSafeCounter	LockFreeListCounter;
#else
	typedef NoopCounter		LockFreeListCounter;
#endif


	class LockFreeVoidPointerListBase : public boost::noncopyable
	{
	public:
		LockFreeVoidPointerListBase()
		{
			LinkAllocator::get();
		}
		~LockFreeVoidPointerListBase()
		{

		}

		void push(void* NewItem)
		{
			Link* newLink = LinkAllocator::get().allocateLink(NewItem);
			newLink->link(&mHead);
			ASSUME(newLink->mNext != LinkAllocator::get().closedLink());
		}


	private:
		struct Link;

		//后续可以优化
		class LinkAllocator
		{
		public:
			Link* allocateLink(void* newItem)
			{
				ASSUME(newItem);
				if (mNumUsedLinks.increment() % 8192 == 1)
				{

				}
				Link* newLink = Link::unlink(&mFreeLinks);
				if (newLink)
				{
					mNumFreeLinks.decrement();
				}
				else
				{
					if (mNumAllocatedLinks.increment() % 10 == 1)
					{

					}
					newLink = new Link();
					newLink->mLockCount.increment();
				}
				BOOST_ASSERT(!newLink->mItem);
				BOOST_ASSERT(!newLink->mNext);
				BOOST_ASSERT(newLink->mLockCount.getValue() >= 1);
				newLink->mItem = newItem;
				PlatformMisc::memoryBarrier();
				return newLink;
			}

			void freeLink(Link* link)
			{
				BOOST_ASSERT(link != closedLink());
				mNumUsedLinks.decrement();
				link->mLockCount.increment();
				PlatformMisc::memoryBarrier();
				link->link(&mFreeLinks);
				mNumFreeLinks.increment();
			}

			Link* closedLink()
			{
				BOOST_ASSERT(nullptr != mSpecialClosedLink);
				return mSpecialClosedLink;
			}

			static CORE_API LinkAllocator& get()
			{
				static LinkAllocator allocator;
				return allocator;
			}
		private:
			LinkAllocator();
			~LinkAllocator();
			Link*	mFreeLinks;
			Link* mSpecialClosedLink;
			
			LockFreeListCounter	mNumUsedLinks;
			LockFreeListCounter	mNumFreeLinks;
			LockFreeListCounter	mNumAllocatedLinks;
		};




		struct Link
		{
			Link*			mNext{ nullptr };
			void*			mItem{ nullptr };
			ThreadSafeCounter	mLockCount;
			ThreadSafeCounter	mMarkedForDeath;

			Link(){}
			inline void unlock(bool bShouldNeverCauseAFree = false)
			{
				ASSUME(mLockCount.getValue() > 0);
				if (mLockCount.decrement() == 0)
				{
					ASSUME(mMarkedForDeath.getValue() < 2);
					if (mMarkedForDeath.reset())
					{
						ASSUME(!bShouldNeverCauseAFree);
						LockFreeVoidPointerListBase::LinkAllocator::get().freeLink(this);
					}
				}
			}

			bool replaceHeadIfHeadEquel(Link** headPointer, Link* specialTestLink)
			{
				checkNotMarkedForDeath();
				while (1)
				{
					if (*headPointer != specialTestLink)
					{
						return false;
					}
					checkNotMarkedForDeath();
					if (PlatformAtomics::interLockedCompareExchangePointer((void**)headPointer, this, specialTestLink) == specialTestLink)
					{
						break;
					}
				}
				return true;
			}

			bool linkIfHeadNotEqual(Link** headPointer, Link* specialClosedLink)
			{
				checkNotMarkedForDeath();
				while (1)
				{
					if (*headPointer == specialClosedLink)
					{
						checkNotMarkedForDeath();
						return false;
					}
					Link* localHeadPointer = lockLink(headPointer, specialClosedLink);
					checkNotMarkedForDeath();
					setNextPointer(localHeadPointer);
					Link* valueWas = (Link*)PlatformAtomics::interLockedCompareExchangePointer((void**)headPointer, this, localHeadPointer);
					if (valueWas == localHeadPointer)
					{
						if (localHeadPointer)
						{
							localHeadPointer->unlock();
						}
						break;
					}
					setNextPointer(nullptr);
					if (localHeadPointer)
					{
						localHeadPointer->unlock();
					}
				}
				return true;
			}

			void link(Link** headPointer)
			{
				checkNotMarkedForDeath();
				while (1)
				{
					Link* localHeadPointer = lockLink(headPointer);
					checkNotMarkedForDeath();
					setNextPointer(localHeadPointer);
					Link* valueWas = (Link*)PlatformAtomics::interLockedCompareExchangePointer((void**)headPointer, this, localHeadPointer);
					if (valueWas == localHeadPointer)
					{
						if (localHeadPointer)
						{
							localHeadPointer->unlock();
						}
						break;
					}
					setNextPointer(nullptr);
					if (localHeadPointer)
					{
						localHeadPointer->unlock();
					}
				}
			}

			void setNextPointer(Link* newNext)
			{
				//Link* localNextPointer = mNext;
				mNext = newNext;
				if (newNext)
				{
					checkNotMarkedForDeath();
				}
			}

			void checkNotMarkedForDeath()
			{
				ASSUME(mMarkedForDeath.getValue() == 0);
			}

			static Link* replaceList(Link** headPointer, Link* newHeadLink = nullptr)
			{
				while (1)
				{
					Link* localHeadPointer = lockLink(headPointer);
					if (!localHeadPointer && !newHeadLink)
					{
						break;
					}
					Link* valueWas = (Link*)PlatformAtomics::interLockedCompareExchangePointer((void**)headPointer, newHeadLink, localHeadPointer);
					if (valueWas == localHeadPointer)
					{
						if (localHeadPointer)
						{
							localHeadPointer->unlock(true);
							localHeadPointer->checkNotMarkedForDeath();
						}
						return localHeadPointer;
					}
					if (localHeadPointer)
					{
						localHeadPointer->unlock();
					}
				}
				return nullptr;
			}

			void dispose()
			{
				checkNotMarkedForDeath();
				mItem = nullptr;
				mNext = nullptr;
				int32 death = mMarkedForDeath.increment();
				ASSUME(death == 1);
				unlock();
			}

			static Link* unlink(Link** headPointer, Link* specialClosedLink = nullptr)
			{
				while (1)
				{
					Link* localHeadPointer = lockLink(headPointer, specialClosedLink);
					if (!localHeadPointer)
					{
						break;
					}
					Link* nextLink = localHeadPointer->mNext;
					Link* valueWas = (Link*)PlatformAtomics::interLockedCompareExchangePointer((void**)headPointer, nextLink, localHeadPointer);
					if (valueWas == localHeadPointer)
					{
						localHeadPointer->setNextPointer(nullptr);
						localHeadPointer->unlock(true);
						localHeadPointer->checkNotMarkedForDeath();
						return localHeadPointer;
					}
					localHeadPointer->unlock();
				}
				return nullptr;
			}

			static Link* lockLink(Link** headPointer, Link* specialClosedLink = nullptr)
			{
				while (1)
				{
					Link* localHeadPointer = *headPointer;
					if (!localHeadPointer || localHeadPointer == specialClosedLink)
					{
						return nullptr;
					}
					localHeadPointer->mLockCount.increment();
					if (*headPointer == localHeadPointer)
					{
						return localHeadPointer;
					}
					localHeadPointer->unlock();
				}
			}
		};

	public:

		static Link* lockLink(Link** headPointer, Link* specialClosedLink = nullptr)
		{
			while (1)
			{
				Link* localHeadPointer = *headPointer;
				if (!localHeadPointer || localHeadPointer == specialClosedLink)
				{
					return nullptr;
				}
				localHeadPointer->mLockCount.increment();
				if (*headPointer == localHeadPointer)
				{
					return localHeadPointer;
				}
				localHeadPointer->unlock();
			}
		}

		bool reopenIfClosedAndPush(void* newItem)
		{
			Link* newLink = LinkAllocator::get().allocateLink(newItem);
			while (1)
			{
				if (newLink->linkIfHeadNotEqual(&mHead, LinkAllocator::get().closedLink()))
				{
					return false;
				}
				if (newLink->replaceHeadIfHeadEquel(&mHead, LinkAllocator::get().closedLink()))
				{
					return true;
				}
			}
		}

		template<class ArrayType, typename ElementType>
		void popAll(ArrayType& output)
		{
			Link* link = Link::replaceList(&mHead);
			while (link)
			{
				output.push_back((ElementType)(link->mItem));
				Link* nextLink = link->mNext;
				link->dispose();
				link = nextLink;
			}
		}

		bool pushIfNotClosed(void* NewItem)
		{
			Link* newLink = LinkAllocator::get().allocateLink(NewItem);
			bool bSuccess = newLink->linkIfHeadNotEqual(&mHead, LinkAllocator::get().closedLink());
			if (!bSuccess)
			{
				newLink->dispose();
			}
			return bSuccess;
		}

		void * pop()
		{
			Link* link = Link::unlink(&mHead);
			if (!link)
			{
				return nullptr;
			}
			void* r = link->mItem;
			link->dispose();
			return r;
		}

		void* popIfNotClosed()
		{
			Link* link = Link::unlink(&mHead, LinkAllocator::get().closedLink());
			if (!link)
			{
				return nullptr;
			}
			void* r = link->mItem;
			link->dispose();
			return r;
		}

		bool closeIfEmpty()
		{
			if (LinkAllocator::get().closedLink()->replaceHeadIfHeadEquel(&mHead, nullptr))
			{
				return true;
			}
			return false;
		}

		bool replaceListIfEmpty(LockFreeVoidPointerListBase& notTreadSafeTempListToReplaceWith)
		{
			if (notTreadSafeTempListToReplaceWith.mHead->replaceHeadIfHeadEquel(&mHead, nullptr))
			{
				notTreadSafeTempListToReplaceWith.mHead = nullptr;
				return true;
			}
			return false;
		}

		template<class ArrayType, typename ElementType>
		void popAllAndClose(ArrayType& output)
		{
			Link* link = Link::replaceList(&mHead, LinkAllocator::get().closedLink());
			while (link)
			{
				output.push_back((ElementType)(link->mItem));
				Link* nextLink = link->mNext;
				link->dispose();
				link = nextLink;
			}
		}

		bool isClosed() const
		{
			return mHead == LinkAllocator::get().closedLink();
		}

		bool isEmpty() const
		{
			return mHead == nullptr;
		}

	private:
		MS_ALIGN(8) Link* mHead { nullptr };
	};

	typedef LockFreeVoidPointerListBase LockFreeVoidPointerListGeneric;


	template<int32 SIZE, typename TBundleRecycler, typename TTrackingCounter = NoopCounter>
	class TLockFreeFixedSizeAllocator_TLSCacheBase : public boost::noncopyable
	{
		enum
		{
			NUM_PER_BUNDLE = 32,
		};
	public:
		TLockFreeFixedSizeAllocator_TLSCacheBase()
		{
			static_assert(SIZE >= sizeof(void*) && SIZE % sizeof(void*) == 0, "dd");
			BOOST_ASSERT(isInGameThread());
			mTLSSlot = PlatformTLS::allocTLSSlot();
			BOOST_ASSERT(PlatformTLS::isValidTLSSlot(mTLSSlot));
		}

		~TLockFreeFixedSizeAllocator_TLSCacheBase()
		{
			PlatformTLS::freeTlsSlot(mTLSSlot);
			mTLSSlot = 0;
		}


		FORCEINLINE void* allocate()
		{
			ThreadLocalCache& tls = getTLS();
			if (!tls.mPartialBundle)
			{
				if (tls.mFullBundle)
				{
					tls.mPartialBundle = tls.mFullBundle;
					tls.mFullBundle = nullptr;
				}
				else
				{
					tls.mPartialBundle = mGlobalFreeListBondles.pop();
					if (!tls.mPartialBundle)
					{
						tls.mPartialBundle = (void**)Memory::malloc(SIZE * NUM_PER_BUNDLE);
						void** next = tls.mPartialBundle;
						for (int32 index = 0; index < NUM_PER_BUNDLE - 1; index++)
						{
							void* nextNext = (void*)(((uint8*)next) + SIZE);
							*next = nextNext;
							next = (void**)nextNext;
						}
						*next = nullptr;
						mNumFree.add(NUM_PER_BUNDLE);
					}
				}
				tls.mNumPartial = NUM_PER_BUNDLE;
			}
			mNumUsed.increment();
			mNumFree.decrement();
			void* result = (void*)tls.mPartialBundle;
			tls.mPartialBundle = (void**)*tls.mPartialBundle;
			tls.mNumPartial--;
			return result;
		}

		FORCEINLINE void free(void* item)
		{
			mNumFree.increment();
			mNumUsed.decrement();

			ThreadLocalCache& tls = getTLS();
			if (tls.mNumPartial >= NUM_PER_BUNDLE)
			{
				if (tls.mFullBundle)
				{
					mGlobalFreeListBondles.push(tls.mFullBundle);
				}
				tls.mFullBundle = tls.mPartialBundle;
				tls.mPartialBundle = nullptr;
				tls.mNumPartial = 0;
			}
			*(void**)item = (void*)tls.mPartialBundle;
			tls.mPartialBundle = (void**)item;
			tls.mNumPartial++;
		}

		const TTrackingCounter & getNumUsed() const
		{
			return mNumUsed;
		}

		const TTrackingCounter& getNumFree() const
		{
			return mNumFree;
		}

	private:
		struct ThreadLocalCache
		{
			void ** mFullBundle{ nullptr };
			void ** mPartialBundle{ nullptr };
			int32 mNumPartial{ 0 };
			ThreadLocalCache(){}
		};

		ThreadLocalCache& getTLS()
		{
			ThreadLocalCache* tls = (ThreadLocalCache*)PlatformTLS::getTLSValue(mTLSSlot);
			if (!tls)
			{
				tls = new ThreadLocalCache();
				PlatformTLS::setTlsValue(mTLSSlot, tls);
			}
			return *tls;
		}

		uint32 mTLSSlot;

		TBundleRecycler mGlobalFreeListBondles;

		TTrackingCounter	mNumFree;
		TTrackingCounter	mNumUsed;
	};
}