#pragma once
#include "Object.h"
namespace Air
{
	class RTexture;
	class RMaterialFunction : public Object
	{
		GENERATED_RCLASS_BODY(RMaterialFunction, Object)
	public:
		void getDependenctFunction(TArray<RMaterialFunction*>& dependentFunctions) const;

		void appendReferencedTextures(TArray<std::shared_ptr<RTexture>>& inOutTextures) const;
	};
}