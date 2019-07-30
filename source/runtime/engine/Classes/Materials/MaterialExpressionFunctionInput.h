#pragma once
#include "Classes/Materials/MaterialExpression.h"
namespace Air
{
	class RMaterialExpressionFunctionInput : public RMaterialExpression
	{
		GENERATED_RCLASS_BODY(RMaterialExpressionFunctionInput, RMaterialExpression);
	public:
		ExpressionInput mPreview;

		ExpressionInput mEffectivePreviewDuringCompile;

		uint32 bCompilingFunctionPreview : 1;

#if WITH_EDITOR
		RMaterialExpression* getEffectivePreviewExpression()
		{
			return bCompilingFunctionPreview ? mPreview.mExpression : mEffectivePreviewDuringCompile.mExpression;
		}
#endif
	};
}