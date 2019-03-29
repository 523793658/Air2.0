#pragma once
#include "CoreType.h"

namespace Air
{
	struct GenericPlatformTLS
	{
		static FORCEINLINE bool isValidTLSSlot(uint32 slotIndex)
		{
			return slotIndex != 0xFFFFFFFF;
		}
	};
}