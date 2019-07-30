#pragma once
#include "Classes/Materials/MaterialExpressionTextureSampleParameter.h"
namespace Air
{
	class ENGINE_API RMaterialExpressionTextureSampleParameter2D : public RMaterialExpressionTextureSampleParameter
	{
		GENERATED_RCLASS_BODY(RMaterialExpressionTextureSampleParameter2D, RMaterialExpressionTextureSampleParameter)
	public:
#if WITH_EDITOR
		virtual void getCaption(TArray<wstring>& outCaptions) const override;
#endif

		virtual bool textureIsValid(std::shared_ptr<RTexture> inTexture) override;

		virtual const TCHAR* getRequirements() override;

		virtual void setDefaultTexture() override;
	};


}