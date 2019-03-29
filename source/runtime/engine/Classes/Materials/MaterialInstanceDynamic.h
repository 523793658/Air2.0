#pragma once
#include "Classes/Materials/MaterialInstance.h"
namespace Air
{
	class ENGINE_API RMaterialInstanceDynamic : public MaterialInstance
	{
		GENERATED_RCLASS_BODY(RMaterialInstanceDynamic, MaterialInstance)
	public:
		void setScalarParameterValue(wstring parameterName, float value);

		void setScalarParameterByInde(int32 parameterIndex, float value);

		void setVectorParameterByIndex(int32 parameterIndex, LinearColor& value);

		void setVectorParameterValue(wstring parameterName, LinearColor& value);

		static RMaterialInstanceDynamic* create(class MaterialInterface* parentMaterial, class Object* inOuter);

		static RMaterialInstanceDynamic* create(class MaterialInterface* parentMaterial, class Object* inOuter, wstring name);

	};
}