#include "Classes/Components/NavMovementComponent.h"
#include "Classes/GameFramework/Actor.h"
#include "SimpleReflection.h"
namespace Air
{
	NavMovementComponent::NavMovementComponent(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{

	}

	DECLARE_SIMPLER_REFLECTION(NavMovementComponent);
}