#pragma once
#include "CoreMinimal.h"
#include "TargetPlatform/TargetPlatformBase.h"
#include "windows/WindowsPlatformProperties.h"
namespace Air
{
#ifdef WINDOWS_TARGET_PLATFORM_RESOURCE
#define WINDOWS_TARGET_PLATFORM_API	DLLEXPORT
#else
#define WINDOWS_TARGET_PLATFORM_API	DLLIMPORT
#endif

	template<bool HAS_EDITOR_DATA, bool IS_DEDICATED_SERVER, bool IS_CLIENT_ONLY>
	class TGenericWindowsTargetPlatform
		: public TTargetPlatformBase<WindowsPlatformProperties<HAS_EDITOR_DATA, IS_DEDICATED_SERVER, IS_CLIENT_ONLY>>
	{

	};


}