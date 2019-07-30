#include "Classes/Components/PawnMovementComponent.h"
#include "Classes/GameFramework/Actor.h"
#include "Classes/GameFramework/Pawn.h"
#include "SimpleReflection.h"

namespace Air
{
	PawnMovementComponent::PawnMovementComponent(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{

	}

	float3 PawnMovementComponent::consumeInputVector()
	{
		return mPawnOwner ? mPawnOwner->internal_consumeMovementInputVector() : float3::Zero;
	}

	float3 PawnMovementComponent::getPendingInputVector() const
	{
		return mPawnOwner ? mPawnOwner->internal_getPendingMovementInputVector() : float3::Zero;
	}

	void PawnMovementComponent::addInputVector(float3 worldVector, bool bForce /* = false */)
	{
		if (mPawnOwner)
		{
			mPawnOwner->internal_addMovementInput(worldVector, bForce);
		}
	}

	void PawnMovementComponent::setUpdatedComponent(SceneComponent* newUpdatedComponent)
	{
		if (newUpdatedComponent)
		{
			if (!newUpdatedComponent->getOwner())
			{
				return;
			}
		}

		ParentType::setUpdatedComponent(newUpdatedComponent);
		mPawnOwner = newUpdatedComponent ? std::dynamic_pointer_cast<APawn>(newUpdatedComponent->getOwner()->shared_from_this()) : std::shared_ptr<APawn>();
	}

	float3 PawnMovementComponent::getLastInputVector() const
	{
		return mPawnOwner ? mPawnOwner->internal_getLastMovementInputVector() : float3::Zero;
	}
	const std::shared_ptr<class APawn>& PawnMovementComponent::getPawnOwner() const
	{
		return mPawnOwner;
	}

	float3 PawnMovementComponent::getInputVector() const
	{
		return getPendingInputVector();
	}


	DECLARE_SIMPLER_REFLECTION(PawnMovementComponent);
}