#include "Classes/GameMapsSetting.h"
namespace Air
{
	const wstring GameMapsSettings::getGameDefaultMap()
	{
		return TEXT("");
	}

	GameMapsSettings* GameMapsSettings::mDefaultSettings = nullptr;

	GameMapsSettings* GameMapsSettings::getDefault()
	{
		if (mDefaultSettings == nullptr)
		{
			mDefaultSettings = new GameMapsSettings();
		}
		return mDefaultSettings;
	}
}