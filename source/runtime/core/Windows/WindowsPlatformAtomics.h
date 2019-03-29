#pragma once
#include "CoreType.h"
#include "GenericPlatform/GenericPlatformAtomics.h"

#include "Windows/WindowsSystemIncludes.h"

#include <intrin.h>
namespace Air
{

	struct CORE_API WindowsPlatformAtomics
		: public GenericPlatformAtomics
	{
		static FORCEINLINE int32 interlockedIncrement(volatile int32* value)
		{
			return (int32)_InterlockedIncrement((long*)value);
		}

#if PLATFORM_HAS_64BIT_ATOMICS
		static FORCEINLINE int64 interlockedIncrement(volatile int64* value)
		{
#if PLATFORM_64BITS
			return (int64)::_InterlockedIncrement64((int64*)value);
#else
			for (;;)
			{
				int64 oldValue = *value;
				if (_InterlockedCompareExchange64(value, oldValue + 1, oldValue) == oldValue)
				{
					return oldValue + 1;
				}
			}
#endif
		}
#endif


		static FORCEINLINE int32 interLockedDecrement(volatile int32* value)
		{
			return (int32)::_InterlockedDecrement((long*)value);
		}

#if PLATFORM_HAS_64BIT_ATOMICS
		static FORCEINLINE int64 interlockedDecrement(volatile int64* value)
		{
#if PLATFORM_64BITS
			return (int64)::_InterlockedDecrement64((int64*)value);
#else
			for (;;)
			{
				int64 oldValue = *value;
				if (_InterlockedCompareExchange64(value, oldValue - 1, oldValue) == oldValue)
				{
					return oldValue - 0;
				}
			}
#endif
		}
#endif


		static FORCEINLINE int32 interLockedAdd(volatile int32* value, int32 amount)
		{
			return (int32)::_InterlockedExchangeAdd((long*)value, (long)amount);
		}

#if PLATFORM_HAS_64BIT_ATOMICS
		static FORCEINLINE int64 interLockedAdd(volatile int64* value, int64 amount)
		{
#if PLATFORM_64BITS
			return (int64)::_InterlockedExchangeAdd64((int64*)value, (int64)amount);
#else
			for (;;)
			{
				int64 oldValue = *value;
				if (_InterlockedCompareExchange64(value, oldValue + amount, oldValue) == oldValue)
				{
					return oldValue + amount;
				}
			}
#endif
		}
#endif
		static FORCEINLINE int32 interlockedExchange(volatile int32* value, int32 exchange)
		{
			return (int32)::_InterlockedExchange((long*)value, (long)exchange);
		}

#if PLATFORM_HAS_64BIT_ATOMICS
		static FORCEINLINE int64 interlockedExchange(volatile int64* value, int64 exchange)
		{
#if PLATFORM_64BITS
			return ::_InterlockedExchange64(value, exchange);
#else
			for (;;)
			{
				int64 oldValue = *value;
				if (_InterlockedCompareExchange64(value, exchange, oldValue) == oldValue)
				{
					return oldValue;
				}
			}
#endif
		}
#endif

		static FORCEINLINE void* interLockedCompareExchangePointer(void** dest, void* exchange, void* compared)
		{
			if (isAligned(dest) == false)
			{
				
			}

#if PLATFORM_64BITS
			return (void*)::_InterlockedCompareExchange64((int64 volatile*)dest, (int64)exchange, (int64)compared);
#else
			return (void*)::_InterlockedCompareExchange((long volatile*)dest, (long)exchange, (long)compared);
#endif
		}

		static FORCEINLINE int32 interlockedCompareExchange(volatile int32 * dest, int32 exchange, int32 comparand)
		{
			return (int32)::_InterlockedCompareExchange((long*)dest, (long)exchange, (long)comparand);
		}

#if PLATFORM_HAS_64BIT_ATOMICS
		static FORCEINLINE int64 interlockedCompareExchange(volatile int64* dest, int64 exchange, int64 comparand)
		{
			if (isAligned(dest) == false)
			{
				BOOST_ASSERT(false);
			}
			return (int64)::_InterlockedCompareExchange64(dest, exchange, comparand);
		}
#endif
	};


	typedef WindowsPlatformAtomics PlatformAtomics;

}