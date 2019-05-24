#pragma once
#include "CoreMinimal.h"
namespace Name_PlatformInfo
{
	enum class EPlatformType : uint8
	{
		Game,
		Editor,
		Client,
		Server,
	};


	using namespace Air;
	struct PlatformInfo
	{
		wstring mPlatformInfoName;
		wstring mTargetPlatformName;
		wstring mVanillaPlatformName;
		wstring mPlatformFlavor;
		wstring mDisplayName;
		EPlatformType mPlatformType;

		wstring mAutoSDKPath;
	};
}