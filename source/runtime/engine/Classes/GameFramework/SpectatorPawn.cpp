#include "Classes/GameFramework/SpectatorPawn.h"
#include "Classes/Components/SphereComponent.h"
#include "SimpleReflection.h"
#include "PlayerController.h"
namespace Air
{
	ASpectatorPawn::ASpectatorPawn(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		mCollisionComponent->bVisible = false;
	}

	void ASpectatorPawn::possessedBy(AController* newController)
	{
		AController* oldController = mController.get();
		mController = std::dynamic_pointer_cast<AController>(newController->shared_from_this());
		if (oldController != newController)
		{
			receivePossessed(mController);
		}
	}

	DECLARE_SIMPLER_REFLECTION(ASpectatorPawn);
}