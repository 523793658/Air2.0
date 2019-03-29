#include "Classes/Materials/MaterialInstanceConstant.h"
#include "SimpleReflection.h"
namespace Air
{
	MaterialInstanceConstant::MaterialInstanceConstant(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{

	}

	DECLARE_SIMPLER_REFLECTION(MaterialInstanceConstant);
}