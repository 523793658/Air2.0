#pragma once
#include "CoreType.h"
#include "GenericPlatform/GenericPlatformTLS.h"
#include "Windows/WindowsSystemIncludes.h"
namespace Air
{
	struct CORE_API WindowsPlatformTLS : public GenericPlatformTLS
	{
		static FORCEINLINE uint32 getCurrentThreadId()
		{
			return Windows::GetCurrentThreadId();
		}

		static FORCEINLINE uint32 allocTLSSlot()
		{
			return Windows::TlsAlloc();
		}

		static FORCEINLINE void setTlsValue(uint32 slotIndex, void * value)
		{
			Windows::TlsSetValue(slotIndex, value);
		}

		static FORCEINLINE void * getTLSValue(uint32 slotIndex)
		{
			return Windows::TlsGetValue(slotIndex);
		}

		static FORCEINLINE void freeTlsSlot(uint32 slotIndex)
		{
			Windows::TlsFree(slotIndex);
		}
	};

	typedef WindowsPlatformTLS PlatformTLS;
}

