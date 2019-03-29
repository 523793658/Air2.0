#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

namespace Air
{

	class ITargetPlatform;

	class ITargetPlatformModule : public IModuleInterface
	{
	public:
		virtual ITargetPlatform* getTargetPlatform() = 0;

	public:
		virtual ~ITargetPlatformModule() {}
	};
}