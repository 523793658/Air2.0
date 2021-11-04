#pragma once
#include "EngineSetting.h"
#include "misc/StringClassReference.h"
#include "UObject/Object.h"
namespace Air
{
	class ENGINE_SETTING_API GameMapsSettings : Object
	{
	public:

	public:
		static const wstring getGameDefaultMap();

		static GameMapsSettings* getDefault();

	private:
		static GameMapsSettings* mDefaultSettings;
	};
}