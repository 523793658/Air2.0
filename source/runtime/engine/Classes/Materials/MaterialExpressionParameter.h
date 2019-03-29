#pragma once
#include "Classes/Materials/MaterialExpression.h"
namespace Air
{
	class RMaterialExpressionParameter : public RMaterialExpression
	{
		GENERATED_RCLASS_BODY(RMaterialExpressionParameter, RMaterialExpression)
	public:
		virtual void serialize(XMLNode* node) override;

	public:
		wstring mParameterName;
		Guid mExpressionGUID;
	};
}