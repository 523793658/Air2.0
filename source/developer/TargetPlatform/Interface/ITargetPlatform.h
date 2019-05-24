#pragma once
#include "CoreMinimal.h"
#include "DesktopPlatform/PlatformInfo.h"
namespace Air
{
	class ITargetPlatform
	{
	public:
		virtual bool isRunningPlatform() const = 0;

		virtual const class StaticMeshLODSettings& getStaticMeshLODSettings() const = 0;

		virtual wstring platformName() const = 0;
	
		virtual const Name_PlatformInfo::PlatformInfo& getPlatformInfo() const = 0;
	};
}