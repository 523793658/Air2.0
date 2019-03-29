#pragma once
#include "CoreMinimal.h"
#include "Classes/Materials/MaterialExpression.h"
#include "MaterialExpressionIO.h"
namespace Air
{
	enum ECustomMaterialOutputType
	{
		CMOT_Float1,
		CMOT_Float2,
		CMOT_Float3,
		CMOT_Float4,
		CMOT_MAX,
	};

	struct CustomInput
	{
		wstring mInputName;
		ExpressionInput mInput;
	};

	class RMaterialExpressionCustom : public RMaterialExpression
	{
		GENERATED_RCLASS_BODY(RMaterialExpressionCustom, RMaterialExpression)
	public:
		wstring mCode;
		TArray<enum ECustomMaterialOutputType> mOutputTypes;
		wstring mDescription;
		TArray<struct CustomInput> mInputs;

		virtual void serialize(Archive& ar) override;

		virtual void serialize(XMLNode* node) override;

#if WITH_EDITOR
		virtual int32 compile(class MaterialCompiler* compiler, int32 outputIndex);

#endif
		virtual const void innerGetInputs(TArray<ExpressionInput *>& result) override;

		virtual ExpressionInput* getInput(int32 inputIndex)override;
	
		virtual wstring getInputName(uint32 inputInde) const override;

#if WITH_EDITOR
		virtual uint32 getInputType(int32 inputIndex) override { return MCT_Unknown; }

		virtual uint32 getOutputType(int32 outputIndex) override;
#endif
	};
}