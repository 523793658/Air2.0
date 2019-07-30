#pragma once
#include "Classes/Materials/MaterialExpressionParameter.h"
namespace Air
{
	class RMaterialExpressionScalarParameter : public RMaterialExpressionParameter
	{
		GENERATED_RCLASS_BODY(RMaterialExpressionScalarParameter, RMaterialExpressionParameter)
	public:
#if WITH_EDITOR


		/*
		<MaterialExpression class="RMaterialExpressionScalarParameter">
			<RMaterialExpressionParameter>
				<RMaterialExpression isParameterExpression="1" guid="" id="1">
					<ExpressionOutputs>
						<ExpressionOutput>
							<Name></Name>
							<Mask>1</Mask>
							<MaskR>1</MaskR>
							<MaskG>1</MaskG>
							<MaskB>1</MaskB>
							<MaskA>1</MaskA>
						</ExpressionOutput>
					</ExpressionOutputs>
				</RMaterialExpression>
				<ParameterName>BaseColor</ParameterName>
			</RMaterialExpressionParameter>
			<DefaultValue>1.0</DefaultValue>
		</MaterialExpression>
		*/
		virtual void serialize(XMLNode* node) override;

		int32 compile(class MaterialCompiler* compiler, int32 outputIndex) override;
#endif
	public:
		float mDefaultValue;
		float mSliderMin;
		float mSliderMax;
	};
}