#pragma once 
#include "CoreType.h"
#include "GenericPlatform/GenericPlatformMisc.h"

namespace Air
{
	struct GenericPlatformProperties
	{
		static FORCEINLINE bool supportsFastVRAMMemory()
		{
			return false;
		}
		static FORCEINLINE bool allowsFramerateSmoothing()
		{
			return true;
		}
	};


}

