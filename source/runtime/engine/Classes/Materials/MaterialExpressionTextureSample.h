#pragma once
#include "Classes/Materials/MaterialExpressionTextureBase.h"
namespace Air
{
	enum ETextureMipValueMode
	{
		TMVM_None,
		TMVM_MipLevel,
		TMVM_MipBias,
		TMVM_Derivative,
		TMVM_Max,
	};

	class ENGINE_API RMaterialExpressionTextureSample : public RMaterialExpressionTextureBase
	{
		GENERATED_RCLASS_BODY(RMaterialExpressionTextureSample, RMaterialExpressionTextureBase)
	public:
		ExpressionInput mCoordinates;

		ExpressionInput mTextureObject;

		ExpressionInput mMipValue;

		ExpressionInput mCoordinatesDX;

		ExpressionInput mCoordinatesDY;

		TEnumAsByte<enum ETextureMipValueMode> mMipValueMode;

		TEnumAsByte<enum ESamplerSourceMode> mSamplerSource;

		uint32 mConstCoordinate;

		int32 mConstMipValue;

	public:

		virtual const TArray<ExpressionInput*> getInputs() override;

		virtual ExpressionInput* getInput(int32 inputIndex) override;

		virtual wstring getInputName(uint32 inputInde) const override;

#if WITH_EDITOR
		virtual int32 compile(class MaterialCompiler* compiler, int32 outputIndex) override;
		virtual void getCaption(TArray<wstring>& outCaptions) const override;

		virtual uint32 getInputType(int32 inputIndex) override;

		virtual void serialize(XMLNode* node) override;
#endif

		void updateTextureResource(class RTexture* inTexture);
#if WITH_EDITOR
		int32 compileMipValue0(class MaterialCompiler* compiler);

		int32 compileMipValue1(class MaterialCompiler* compiler);
#endif


	protected:
		bool bShowTextureInputPin;
	};
}