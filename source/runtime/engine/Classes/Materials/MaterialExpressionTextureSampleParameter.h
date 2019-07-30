#pragma once
#include "Classes/Materials/MaterialExpressionTextureSample.h"
namespace Air
{
	class ENGINE_API RMaterialExpressionTextureSampleParameter : public RMaterialExpressionTextureSample
	{
		GENERATED_RCLASS_BODY(RMaterialExpressionTextureSampleParameter, RMaterialExpressionTextureSample)
	public:
		wstring mParameterName;

		Guid mExpressionGUID;

		wstring mGroup;

#if WITH_EDITOR
		virtual int32 compile(class MaterialCompiler* compiler, int32 outputIndex) override;

		virtual void getCaption(TArray<wstring>& outCaptions) const override;

		virtual const TCHAR* getRequirements();

		virtual bool textureIsValid(std::shared_ptr<RTexture> inTexture);

		/*
		<MaterialExpression class="RMaterialExpressionTextureSampleParameter">
            <RMaterialExpressionTextureSample>
            </RMaterialExpressionTextureSample>
            <ParameterName></ParameterName>
        </MaterialExpression>
		*/
		virtual void serialize(XMLNode* node) override;
#endif

		virtual void setDefaultTexture();

	};
}