#include "Classes/Components/InputComponent.h""
#include "Classes/GameFramework/Actor.h"
#include "SimpleReflection.h"
namespace Air
{
	InputComponent::InputComponent(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{

	}

	void InputComponent::clearBindingValues()
	{
		for (InputAxisBinding& axisBinding : mAxisBindings)
		{
			axisBinding.mAxisValue = 0.f;
		}

		for (InputAxisKeyBinding& axisKeyBinding : mAxisKeyBindings)
		{
			axisKeyBinding.mAxisValue = 0.f;
		}

		for (InputVectorAxisBinding& vectorAxisBinding : mVectorAxisBindings)
		{
			vectorAxisBinding.mAxisValue = float3::Zero;
		}

	}

	DECLARE_SIMPLER_REFLECTION(InputComponent);
}