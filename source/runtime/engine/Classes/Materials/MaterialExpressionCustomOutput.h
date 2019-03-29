#pragma once
#include "CoreMinimal.h"
#include "Classes/Materials/MaterialExpression.h"
namespace Air
{
	class RMaterialExpressionCustomOutput : public RMaterialExpression
	{
		GENERATED_RCLASS_BODY(RMaterialExpressionCustomOutput, RMaterialExpression)
	public:
		virtual uint32 getNumOutputs() const { return 1; }
		virtual wstring getFunctionName() const PURE_VIRTRUAL(RMaterialExpressionCustomOutput::getFunctionName, return TEXT("getCustomOutput"););
		virtual wstring getDisplayName() const { return getFunctionName(); }
	};
}