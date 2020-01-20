#pragma once
#include "CoreType.h"
#include "Template/TypeCompatibleBytes.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Delegates/IDelegateInstance.h"
#include "Math/Math.h"
namespace Air
{

#if !defined(_WIN32) || defined(_WIN64)
	typedef TAlignedBytes<16, 16> AlignedInlinedelegateType;
#if USE_SMALL_DELEGATES
	typedef HeapAllocator DelegateAllocatorType;
#else
	typedef TInlineAllocator<2> DelegateAllocatorType;
#endif
#endif

	template<typename ObjectPtrType>
	class MulticastDelegateBase;



	class DelegateBase
	{
		friend class MulticastDelegateBase<std::weak_ptr<void>>;
	public:
		explicit DelegateBase()
			:mDelegateSize(0)
		{
		}

		~DelegateBase()
		{
			unbind();
		}

		DelegateBase(DelegateBase&& other)
		{
			mDelegateAllocator.moveToEmpty(other.mDelegateAllocator);
			mDelegateSize = other.mDelegateSize;
			other.mDelegateSize = 0;
		}

		DelegateBase& operator = (DelegateBase&& other)
		{
			unbind();
			mDelegateAllocator.moveToEmpty(other.mDelegateAllocator);
			mDelegateSize = other.mDelegateSize;
			other.mDelegateSize = 0;
			return *this;
		}

		uint64 getBoundProgramCounterForTimerManager() const
		{
			if (IDelegateInstance* ptr = getDelegateInstanceProtected())
			{
				return ptr->getBoundProgramCounterForTimerManager();
			}
			return 0;
		}

		FORCEINLINE IDelegateInstance* getDelegateInstance() const
		{
			return getDelegateInstanceProtected();
		}

		FORCEINLINE DelegateHandle getHandle() const
		{
			DelegateHandle result;
			if (IDelegateInstance* ptr = getDelegateInstanceProtected())
			{
				result = ptr->getHandle();
			}
			return result;
		}

		FORCEINLINE bool isBound() const
		{
			IDelegateInstance* ptr = getDelegateInstanceProtected();
			return ptr && ptr->isSafeToExecute();
		}

		FORCEINLINE void unbind()
		{
			if (IDelegateInstance* ptr = getDelegateInstanceProtected())
			{
				ptr->~IDelegateInstance();
				mDelegateAllocator.resizeAllocation(0, 0, sizeof(AlignedInlinedelegateType));
				mDelegateSize = 0;
			}
		}
	protected:
		FORCEINLINE IDelegateInstance* getDelegateInstanceProtected() const
		{
			return mDelegateSize ? (IDelegateInstance*)mDelegateAllocator.getAllocation() : nullptr;
		}

	private:


		inline void* operator new (size_t size, Air::DelegateBase& base)
		{
			return base.allocate(size);
		}

		void* allocate(int32 size)
		{
			if (IDelegateInstance* currentInstance = getDelegateInstanceProtected())
			{
				currentInstance->~IDelegateInstance();
			}

			int32 newDelegateSize = Math::divideAndRoundUp(size, (int32)sizeof(AlignedInlinedelegateType));
			if (mDelegateSize != newDelegateSize)
			{
				mDelegateAllocator.resizeAllocation(0, newDelegateSize, sizeof(AlignedInlinedelegateType));
				mDelegateSize = newDelegateSize;
			}

			return mDelegateAllocator.getAllocation();
		}
	private:
		DelegateAllocatorType::ForElementType<AlignedInlinedelegateType> mDelegateAllocator;
		int32 mDelegateSize;
	};
}


