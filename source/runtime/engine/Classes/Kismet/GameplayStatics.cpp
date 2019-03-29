#include "Classes/Kismet/GameplayStatics.h"
#include "Classes/Engine/Engine.h"
#include "SimpleReflection.h"
namespace Air
{
	GameplayStatics::GameplayStatics(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{

	}

	class APlayerController* GameplayStatics::getPlayerController(const Object* worldContextObject, int32 playerIndex)
	{
		if (World* world = GEngine->getWorldFromContextObject(worldContextObject))
		{
			uint32 index = 0; 
			for (ConstPlayerControllerIterator iterator = world->getPlayerControllerIterator(); iterator; ++iterator)
			{
				APlayerController* playerController = *iterator;
				if (index == playerIndex)
				{
					return playerController;
				}
				index++;
			}
		}
		return nullptr;
	}

	DECLARE_SIMPLER_REFLECTION(GameplayStatics);
}