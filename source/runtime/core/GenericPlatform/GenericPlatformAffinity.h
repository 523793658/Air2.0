#pragma once

#include "CoreType.h"

namespace Air
{
	enum EThreadPriority
	{
		TPri_Normal,
		TPri_AboveNormal,
		TPri_BelowNormal,
		TPri_Highest,
		TPri_Lowest,
		TPri_SlightlyBelowNormal,
	};

	class GenericPlatformAffinity
	{
	public:
		static const CORE_API uint64 getMainGameMask()
		{
			return 0xffffffffffffffff;
		}

		static const CORE_API uint64 getNoAffinityMask()
		{
			return 0xFFFFFFFFFFFFFFFF;
		}

		static const CORE_API uint64 getTaskGraphThreadMask()
		{
			return 0xFFFFFFFFFFFFFFFF;
		}
	};
}