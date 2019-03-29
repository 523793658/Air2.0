#pragma once
#include "CoreType.h"
#include "HAL/PlatformAtomics.h"

namespace Air
{
	class ThreadSafeCounter
	{
	public:
		typedef int32 IntegerType;
		ThreadSafeCounter()
		{}
		ThreadSafeCounter(const ThreadSafeCounter& other)
		{
			mCounter = other.getValue();
		}

		ThreadSafeCounter(int32 value)
			:mCounter(value)
		{
		}

		int32 increment()
		{
			return PlatformAtomics::interlockedIncrement(&mCounter);
		}

		int32 add(int32 amount)
		{
			return PlatformAtomics::interLockedAdd(&mCounter, amount);
		}

		int32 decrement()
		{
			return PlatformAtomics::interLockedDecrement(&mCounter);
		}

		int32 subtract(int32 amount)
		{
			return PlatformAtomics::interLockedAdd(&mCounter, -amount);
		}

		int32 set(int32 value)
		{
			return PlatformAtomics::interlockedExchange(&mCounter, value);
		}


		int32 reset()
		{
			return PlatformAtomics::interlockedExchange(&mCounter, 0);
		}

		int32 getValue() const
		{
			return mCounter;
		}
	private:
		void operator = (const ThreadSafeCounter& other) {}
		volatile int32 mCounter{ 0 };
	};
}