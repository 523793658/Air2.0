#pragma once

#include "CoreType.h"


namespace Air
{
	struct CORE_API GenericPlatformTime
	{
	public:
		static double getSecondsPerCycle()
		{
			return mSecondsPerCycle;
		}

		static float toMilliseconds(const uint32 cycles)
		{
			return (float)double(mSecondsPerCycle * 1000.0 * cycles);
		}

	protected:
		static double mSecondsPerCycle;
		static double mSecondsPerCycle64;
	};
}