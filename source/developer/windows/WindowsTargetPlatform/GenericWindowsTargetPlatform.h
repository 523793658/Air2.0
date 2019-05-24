#pragma once
#include "CoreMinimal.h"
#include "TargetPlatform/TargetPlatformBase.h"
#include "windows/WindowsPlatformProperties.h"

#include "StaticMeshResources.h"

#include "TargetPlatform/Interface/ITargetDevice.h"

#include "WindowsTargetPlatform/LocalPcTargetDevice.h"

namespace Air
{


	template<bool HAS_EDITOR_DATA, bool IS_DEDICATED_SERVER, bool IS_CLIENT_ONLY>
	class TGenericWindowsTargetPlatform
		: public TTargetPlatformBase<WindowsPlatformProperties<HAS_EDITOR_DATA, IS_DEDICATED_SERVER, IS_CLIENT_ONLY>>
	{
	public:

		TGenericWindowsTargetPlatform()
		{
#if PLATFORM_WINDOWS
			mLocalDevice = MakeSharedPtr<LocalPcTargetDevice>(*this);
#endif
			ConfigCacheIni::loadLocalIniFile(mEngineSettings, TEXT("Engine"), true, platformName().c_str());
			mStaticMeshLODSettings.initialize(mEngineSettings);
			GConfig->getArray(TEXT("/Script/WindowsTargetPlatform.WindowsTargetSettings"), TEXT("TargetedRHIs"), mTargetedShaderFormats, GEngineIni);

			TArray<wstring> possibleShaderFormats;


		}

		virtual bool isRunningPlatform() const override
		{
			return PLATFORM_WINDOWS;
		}

		virtual const class StaticMeshLODSettings& getStaticMeshLODSettings() const override
		{
			return mStaticMeshLODSettings;
		}

	private:
		StaticMeshLODSettings mStaticMeshLODSettings;

		ITargetDevicePtr mLocalDevice;

		ConfigFile mEngineSettings;

		TArray<wstring> mTargetedShaderFormats;
	};


}