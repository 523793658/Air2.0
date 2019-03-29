#pragma once
#include "CoreMinimal.h"
#include "Classes/GameFramework/DefaultPawn.h"
namespace Air
{
	class ENGINE_API ASpectatorPawn : public ADefaultPawn
	{
		GENERATED_RCLASS_BODY(ASpectatorPawn, ADefaultPawn)
	public:
		virtual void possessedBy(AController* newController) override;
	};
}