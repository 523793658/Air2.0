#include "Classes/Components/ChildActorComponent.h"
#include "Classes/GameFramework/Actor.h"
#include "SimpleReflection.h"

namespace Air
{
	ChildActorComponent::ChildActorComponent(const ObjectInitializer& objectInitializer /* = ObjectInitializer::get() */)
		:SceneComponent(objectInitializer)
	{
		bAllowReregistration = false;
	}

	DECLARE_SIMPLER_REFLECTION(ChildActorComponent);
}