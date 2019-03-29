#pragma once
#include "Classes/Materials/MaterialExpressionParameter.h"
namespace Air
{
	class RMaterialExpressionScalarParameter : public RMaterialExpressionParameter
	{
		GENERATED_RCLASS_BODY(RMaterialExpressionScalarParameter, RMaterialExpressionParameter)
	public:
#if WITH_EDITOR
		virtual void serialize(XMLNode* node) override;

		int32 compile(class MaterialCompiler* compiler, int32 outputIndex) override;
#endif
	public:
		float mDefaultValue;
		float mSliderMin;
		float mSliderMax;
	};
}