#pragma once
#include "EngineMininal.h"
#include "Classes/GameFramework/Info.h"
namespace Air
{
	class WorldSettings : public AInfo
	{
		GENERATED_RCLASS_BODY(WorldSettings, AInfo)
	public:

		virtual void notifyMatchStarted();

		virtual void notifyBeginPlay();

	public:
		float mWorldToMeters;

		uint32 bEnableWorldBoundsCheck : 1;

		float mKillY;
	};
}