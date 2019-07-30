#pragma once
#include "CoreMinimal.h"
#include "Classes/Materials/MaterialExpressionParameter.h"
namespace Air
{
	class RMaterialExpressionVectorParameter : public RMaterialExpressionParameter
	{
		GENERATED_RCLASS_BODY(RMaterialExpressionVectorParameter, RMaterialExpressionParameter)
	public:
	public:
		LinearColor mDefaultValue;

#if WITH_EDITOR
		/*
		<MaterialExpression class="RMaterialExpressionVectorParameter">
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
			<DefaultValue>R=1.0,G=1.0,B=1.0,A=1.0</DefaultValue>
		</MaterialExpression>
		*/


		virtual void serialize(XMLNode* node) override;

		virtual int32 compile(class MaterialCompiler* compiler, int32 outputIndex) override;
		virtual void getCaption(TArray<wstring>& outCaptions) const override;
#endif

		bool isNamedParameter(wstring inParameterName, LinearColor& outValue) const;

#if	WITH_EDITOR
#endif
	};
}