#pragma once
#include "CoreMinimal.h"
#include "Classes/GameFramework/Pawn.h"
#include "Classes/Components/PawnMovementComponent.h"
namespace Air
{
	class SphereComponent;

	class ENGINE_API ADefaultPawn : public APawn
	{
		GENERATED_RCLASS_BODY(ADefaultPawn, APawn)
	private:
		virtual PawnMovementComponent* getMovementComponent() const override;

		float mBaseTurnRate;
		float mBaseLookUpRate;
	public:
		virtual void lookUp(float val);

		virtual void turn(float val);

		void turnAtRate(float rate);

		virtual void moveForward(float val);

		virtual void moveRight(float val);

		virtual void moveUp_World(float val);


		
		virtual void setupPlayerInputComponent(InputComponent* playerInputComponent) override;



	public:
		static wstring mMovementComponentName;
		static wstring mCollisionComponentName;

		uint32 bAddDefaultMovementBindings : 1;

		std::shared_ptr<SphereComponent> mCollisionComponent;
	private:
		std::shared_ptr<PawnMovementComponent> mMovementComponent;
	};
}