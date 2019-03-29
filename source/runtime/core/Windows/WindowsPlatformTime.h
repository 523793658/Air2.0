#pragma once

#include "GenericPlatform/GenericPlatformTime.h"
#include "Windows/WindowsSystemIncludes.h"
namespace Air
{
	struct CORE_API WindowsPlatformTime : public GenericPlatformTime
	{
	public:

		static double initTiming();



		//获取当前CPU时间
		static FORCEINLINE double seconds()
		{
			Windows::LARGE_INTEGER cycles;
			Windows::QueryPerformanceCounter(&cycles);
			return cycles.QuadPart * getSecondsPerCycle() + 16777216.0;
		}

		static void systemTime(int32& year, int32& month, int32& dayOfWeek, int32& day, int32& hour, int32& min, int32 & sec, int32& msec);

		static FORCEINLINE uint32 cycles()
		{
			Windows::LARGE_INTEGER cycles;
			Windows::QueryPerformanceCounter(&cycles);
			return cycles.QuadPart;
		}
	};


	typedef WindowsPlatformTime PlatformTime;
}