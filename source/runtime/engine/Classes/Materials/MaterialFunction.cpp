#include "Classes/Materials/MaterialFunction.h"
#include "SimpleReflection.h"
namespace Air
{
	RMaterialFunction::RMaterialFunction(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{

	}

	void RMaterialFunction::getDependenctFunction(TArray<RMaterialFunction*>& dependentFunctions) const
	{
	}

	void RMaterialFunction::appendReferencedTextures(TArray<std::shared_ptr<RTexture>>& inOutTextures) const
	{

	}

	DECLARE_SIMPLER_REFLECTION(RMaterialFunction);
}