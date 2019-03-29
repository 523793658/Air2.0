#pragma once
#include "EngineMininal.h"
#include "Classes/GameFramework/Info.h"
namespace Air
{
	class WorldSettings : public Info
	{
		GENERATED_RCLASS_BODY(WorldSettings, Info)
	public:

		virtual void notifyMatchStarted();

		virtual void notifyBeginPlay();

	public:
		float mWorldToMeters;

		uint32 bEnableWorldBoundsCheck : 1;

		float mKillY;
	};
}