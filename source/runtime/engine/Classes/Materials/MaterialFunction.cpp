#include "Classes/Materials/MaterialFunction.h"
#include "SimpleReflection.h"
namespace Air
{
	RMaterialFunction::RMaterialFunction(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{

	}

	DECLARE_SIMPLER_REFLECTION(RMaterialFunction);
}