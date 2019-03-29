#pragma once
#include "Classes/Materials/MaterialExpression.h"
#include "Classes/Materials/MaterialFunction.h"
namespace Air
{
	class RMaterialExpressionMaterialFunctionCall : public RMaterialExpression
	{
		GENERATED_RCLASS_BODY(RMaterialExpressionMaterialFunctionCall, RMaterialExpression)
	public:
		class RMaterialFunction * mMaterialFunction;
	};
}