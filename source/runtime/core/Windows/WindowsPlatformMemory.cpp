#include "Windows/WindowsPlatformMemory.h"
#include "HAL/MallocAnsi.h"
namespace Air
{


	Malloc* WindowsPlatformMemory::baseAllocator()
	{
		mAllocatorToUse = Ansi;

		switch (mAllocatorToUse)
		{
		case Air::GenericPlatformMemory::Ansi:
			return new MallocAnsi();
		default:
			break;
		}
	}

	const PlatformMemoryConstants& WindowsPlatformMemory::getConstants()
	{
		static PlatformMemoryConstants memoryConstants;
		if (memoryConstants.mTotalPhysical == 0)
		{
			MEMORYSTATUSEX memoryStatusEx;
			PlatformMemory::Memzero(&memoryStatusEx, sizeof(memoryStatusEx));
			memoryStatusEx.dwLength = sizeof(memoryStatusEx);
			::GlobalMemoryStatusEx(&memoryStatusEx);
			SYSTEM_INFO systemInfo;
			PlatformMemory::Memzero(&systemInfo, sizeof(systemInfo));
			::GetSystemInfo(&systemInfo);

			memoryConstants.mTotalPhysical = memoryStatusEx.ullTotalPhys;
			memoryConstants.mTotalVirtual = memoryStatusEx.ullTotalVirtual;
			memoryConstants.mPageSize = systemInfo.dwPageSize;
			memoryConstants.mTotalPhysicalGB = (memoryConstants.mTotalPhysical + 1024 * 1024 * 1024 - 1) / 1024 / 1024 / 1024;
		}
		return memoryConstants;
	}
}