#pragma once
#include "Classes/Components/MovementComponent.h"
namespace Air
{
	class ENGINE_API NavMovementComponent : public MovementComponent
	{
		GENERATED_RCLASS_BODY(NavMovementComponent, MovementComponent)

	public:
		virtual void stopMovementImmediately() override;
	};

	inline void NavMovementComponent::stopMovementImmediately()
	{
		ParentType::stopMovementImmediately();
	}
}