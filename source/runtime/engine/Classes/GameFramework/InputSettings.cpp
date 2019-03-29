#include "Classes/GameFramework/InputSettings.h"
#include "SimpleReflection.h"
namespace Air
{
	InputSettings::InputSettings(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{}


	DECLARE_SIMPLER_REFLECTION(InputSettings);
}