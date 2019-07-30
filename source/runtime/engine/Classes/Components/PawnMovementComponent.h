#pragma once
#include "Classes/Components/NavMovementComponent.h"
namespace Air
{
	class ENGINE_API PawnMovementComponent : public NavMovementComponent
	{
		GENERATED_RCLASS_BODY(PawnMovementComponent, NavMovementComponent)

	public:
		virtual float3 consumeInputVector();

		virtual void setUpdatedComponent(SceneComponent* newUpdatedComponent) override;

		virtual void addInputVector(float3 worldVector, bool bForce = false);

		float3 getPendingInputVector() const;

		float3 getLastInputVector() const;

		const std::shared_ptr<class APawn>& getPawnOwner() const;

		float3 getInputVector() const;

	private:

	protected:
		std::shared_ptr<class APawn> mPawnOwner;

		friend class APawn;
	};
}