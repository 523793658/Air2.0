#pragma once
#include "CoreType.h"
#include "HAL/TIsAutoCleanup.h"
#include <functional>
namespace Air
{
	class ThreadSingletonInitializer
	{
	public:
		static CORE_API TLSAutoCleanup* get(std::function<TLSAutoCleanup*()> createInstance, uint32& tlsSlot);
	};


	template<class T>
	class TThreadSingleton : public  TLSAutoCleanup
	{
		static uint32& getTLSSlot()
		{
			static uint32 TLSSlot = 0xffffffff;
			return TLSSlot;
		}
	protected:
		TThreadSingleton()
			:mThreadId(PlatformTLS::getCurrentThreadId())
		{}

		static TLSAutoCleanup* createInstance()
		{
			return new T();
		}

		const uint32 mThreadId;

	public:
		FORCEINLINE static T& get()
		{
			return *(T*)ThreadSingletonInitializer::get([]() {return (TLSAutoCleanup*)new T(); }, T::getTLSSlot());
		}
	};
}