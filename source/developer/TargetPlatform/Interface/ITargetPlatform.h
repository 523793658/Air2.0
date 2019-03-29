#pragma once
#include "CoreMinimal.h"

namespace Air
{
	class ITargetPlatform
	{
	public:
		virtual bool isRunningPlatform() const = 0;
	};
}