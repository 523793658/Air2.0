#pragma once
#include "CoreType.h"
#include "Modules/ModuleInterface.h"
#include "Containers/String.h"
namespace Air
{
	class ITargetPlatformManagerModule : public IModuleInterface
	{
	public:
		virtual uint16 shaderFormatVersion(wstring name) = 0;

		virtual const class IShaderFormat* findShaderFormat(wstring name) = 0;

		virtual const TArray<const class IShaderFormat*>& getShaderFormats() = 0;

		virtual ITargetPlatform* getRunningTargetPlatform() = 0;

		virtual const TArray<ITargetPlatform*>& getTargetPlatforms() = 0;
	};
}