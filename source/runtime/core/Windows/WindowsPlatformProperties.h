#pragma once
#include "GenericPlatform/GenericPlatformProperties.h"
namespace Air
{
	template<bool HAS_EDITOR_DATA, bool IS_DEDICATED_SERVER, bool IS_CLIENTT_ONLY>
	struct WindowsPlatformProperties : public GenericPlatformProperties
	{
		static FORCEINLINE bool requiresCookedData()
		{
			//return !HAS_EDITOR_DATA;
			return false;
		}

		static FORCEINLINE bool supportsWindowedMode()
		{
			return true;
		}


		static FORCEINLINE bool isServerOnly()
		{
			return false;
		}

		static FORCEINLINE const TCHAR* platformName()
		{
			return TEXT("Windows");
		}

		static FORCEINLINE const char* iniPlatformName()
		{
			return "Windows";
		}

		static FORCEINLINE bool hasEditorOnlyData()
		{
			return HAS_EDITOR_DATA;
		}
	};
}