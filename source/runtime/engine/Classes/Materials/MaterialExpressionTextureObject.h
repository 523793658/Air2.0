#pragma once
#include "Classes/Materials/MaterialExpressionTextureBase.h"
namespace Air
{
	class RMaterialExpressionTextureObject : public RMaterialExpressionTextureBase
	{
		GENERATED_RCLASS_BODY(RMaterialExpressionTextureObject, RMaterialExpressionTextureBase)
	public:
#if WITH_EDITOR
		virtual int32 compile(class MaterialCompiler* compiler, int32 outputIndex) override;

		virtual void getCaption(TArray<wstring>& outCaptions) const override;

		virtual uint32 getOutputType(int32 outputIndex) override;
#endif
	};
}