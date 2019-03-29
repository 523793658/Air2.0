#pragma once
#include "Classes/Components/PawnMovementComponent.h"
namespace Air
{
	class ENGINE_API FloatingPawnMovement : public PawnMovementComponent
	{
		GENERATED_RCLASS_BODY(FloatingPawnMovement, PawnMovementComponent)
	public:
		virtual void tickComponent(float deltaTime, enum ELevelTick tickType, ActorComponentTickFunction* thisTickFunction) override;

		virtual float getMaxSpeed() const override { return mMaxSpeed; }


	public:
		float mMaxSpeed;

		float mAcceleration;

		float mDeceleration;

		float mTurningBoost;

	protected:

		virtual void applyControlInputToVelocity(float deltaTime);

		virtual bool limitWorldBounds();

		uint32 bPositionCorrected : 1;
	};
}