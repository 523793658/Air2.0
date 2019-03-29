#pragma once
#include "CoreType.h"
#include "GenericPlatform/GenericPlatformMemory.h"
#include "Windows/WindowsSystemIncludes.h"
#include "Windows/WindowsHWrapper.h"
class Malloc;
namespace Air
{
	struct CORE_API WindowsPlatformMemory
		: public GenericPlatformMemory
	{
	public:
		static Malloc* baseAllocator();

		static const PlatformMemoryConstants& getConstants();
	};

	typedef WindowsPlatformMemory PlatformMemory;
}