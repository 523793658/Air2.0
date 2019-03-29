#include "Classes/GameFramework/SpectatorPawn.h"
#include "Classes/Components/SphereComponent.h"
#include "SimpleReflection.h"
namespace Air
{
	ASpectatorPawn::ASpectatorPawn(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		mCollisionComponent->bVisible = false;
	}

	void ASpectatorPawn::possessedBy(AController* newController)
	{
		AController* const oldController = mController;
		mController = newController;
		if (oldController != newController)
		{
			receivePossessed(mController);
		}
	}

	DECLARE_SIMPLER_REFLECTION(ASpectatorPawn);
}