#pragma once
#include "Classes/Materials/MaterialExpression.h"
namespace Air
{
	class ENGINE_API RMaterialExpressionTextureBase : public RMaterialExpression
	{
		GENERATED_RCLASS_BODY(RMaterialExpressionTextureBase, RMaterialExpression)
	public:
#if WITH_EDITOR
		virtual wstring getDescription() const override;
#endif

		virtual std::shared_ptr<class RTexture> getReferencedTexture() override
		{
			return mTexture;
		}

		void autoSetSamplerType();

		static EMaterialSamplerType getSamplerTypeForTexture(std::shared_ptr<const RTexture> texture);
	public:
		std::shared_ptr<class RTexture> mTexture;
	
		TEnumAsByte<enum EMaterialSamplerType> mSamplerType;

		uint32 isDefaultMeshpaintTexture : 1;



	};
}