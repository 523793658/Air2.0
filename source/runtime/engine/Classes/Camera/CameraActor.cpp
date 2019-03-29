#include "Classes/Camera/CameraActor.h"
#include "SimpleReflection.h"
namespace Air
{
	ACameraActor::ACameraActor(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{}

	DECLARE_SIMPLER_REFLECTION(ACameraActor);
}