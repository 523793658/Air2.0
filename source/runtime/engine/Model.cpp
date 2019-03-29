#include "Model.h"
#include "SimpleReflection.h"
namespace Air
{
	Model::Model(const ObjectInitializer& objectInitializer /* = ObjectInitializer::get() */)
		:Object(objectInitializer)
	{

	}

	void Model::initialize(Brush* owner, bool inRootOutside /* = true */)
	{

	}
	DECLARE_SIMPLER_REFLECTION(Model)
}