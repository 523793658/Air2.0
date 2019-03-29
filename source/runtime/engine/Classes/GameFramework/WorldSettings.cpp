#include "Classes/GameFramework/WorldSettings.h"
#include "EngineDefines.h"
#include "Classes/Engine/World.h"
#include "SimpleReflection.h"
namespace Air
{
	WorldSettings::WorldSettings(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		mKillY = -HALF_WORLD_MAX1;
	}

	void WorldSettings::notifyMatchStarted()
	{
		World* world = getWorld();
		world->bMatchStarted = true;
	}


	void WorldSettings::notifyBeginPlay()
	{
		World* world = getWorld();
		if (!world->bBegunPlay)
		{
			for (auto& actor : world->getActors())
			{
				actor->dispatchBeginPlay();
			}
			world->bBegunPlay = true;
		}
	}

	DECLARE_SIMPLER_REFLECTION(WorldSettings);
}