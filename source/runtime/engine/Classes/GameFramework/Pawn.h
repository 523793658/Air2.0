#pragma once
#include "Classes/GameFramework/Actor.h"
#include "Classes/Components/PawnMovementComponent.h"
namespace Air
{
	class AController;
	class InputComponent;
	class ENGINE_API APawn : public AActor
	{
		GENERATED_RCLASS_BODY(APawn, AActor)
	public:
		virtual PawnMovementComponent* getMovementComponent() const;

		virtual void possessedBy(AController* newController);

		virtual void unPossessed();

		virtual void pawnClientRestart();

		virtual void restart();

	

		virtual float3 consumeMovementInputVector();

		FORCEINLINE AController* getController() const
		{
			return mController;
		}

		void receivePossessed(AController* newController) {}

		virtual void addControllerYawInput(float val);

		virtual void addControllerPitchInput(float val);

		virtual void addControllerRollInput(float val);

		virtual void addMovementInput(float3 worldDirection, float scaleValue = 1.0f, bool bForce = false);

		virtual void getActorEyesViewPoint(float3& outLocation, Rotator& outRotation) const override;

		virtual float3 getPawnViewLocation() const;

		virtual Rotator getViewRotation() const;

		virtual void faceRotation(Rotator& newControlRotation, float deltaTime = 0.f);

	public:
		virtual float3 internal_consumeMovementInputVector();

		inline float3 internal_getPendingMovementInputVector() const { return mControlInputVector; }

		inline float3 internal_getLastMovementInputVector() const { return mLastControlInputVector; }

		void internal_addMovementInput(float3 worldAccel, bool bForce = false);


		bool inputEnabled() const { return bInputEnabled; }

	protected:
		virtual InputComponent* createPlayerInputComponent();

		virtual void destroyPlayerInputComponent() {}

		virtual void setupPlayerInputComponent(InputComponent* playerInputComponent) {}
	public:
		AController* mController;

		class APlayerState* mPlayerState{ nullptr };

		float mBaseEyeHeight;

		bool bInputEnabled;
	protected:
		float3 mLastControlInputVector;

		float3 mControlInputVector;

		bool bUseControllerRotationPitch : 1;

		bool bUseControllerRotationYaw : 1;

		bool bUseControllerRotationRoll : 1;

		friend class APlayerController;
	};
}