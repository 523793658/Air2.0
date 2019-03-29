#pragma once
#include "GenericPlatform/GenericPlatformAffinity.h"

namespace Air
{
	class WindowsPlatformAffinity : public GenericPlatformAffinity
	{
	public:
		static const CORE_API uint64 getRenderingThreadMask()
		{
			return 0xffffffffffffffff;
		}

		static const CORE_API uint64 getRTHeartBeatMask()
		{
			return 0xffffffffffffffff;
		}


		static EThreadPriority getRenderingThreadPriority()
		{
			return TPri_Normal;
		}

	};

	typedef WindowsPlatformAffinity PlatformAffinity;
}