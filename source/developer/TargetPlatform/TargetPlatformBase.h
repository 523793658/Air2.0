#pragma once
#include "CoreMinimal.h"
#include "Interface/ITargetPlatform.h"
namespace Air
{

	class TargetPlatformBase
		: public ITargetPlatform
	{

	};


	template<typename PlatformProperties>
	class TTargetPlatformBase
		: public TargetPlatformBase
	{

	};
}