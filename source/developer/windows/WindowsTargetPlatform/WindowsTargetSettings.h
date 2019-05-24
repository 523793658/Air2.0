#pragma once
#include "CoreMinimal.h"
#ifdef WindowsTargetPlatform_EXPORTS
#define WINDOWS_TARGET_PLATFORM_API	DLLEXPORT
#else
#define WINDOWS_TARGET_PLATFORM_API	DLLIMPORT
#endif


namespace Air
{
	enum class EMinmumSupportedOS : uint8
	{
		MSOS_Win7 = 0
	};


	class WINDOWS_TARGET_PLATFORM_API WindowsTargetSettings
	{
	public:
		TArray<wstring> mTargetedRHIs;

		EMinmumSupportedOS mMinimumOSVersion;

		virtual ~WindowsTargetSettings() {}
	};
}