#include "RenderTarget.h"
#include "AirEngine.h"
namespace Air
{
	const Texture2DRHIRef& RenderTarget::getRenderTargetTexture() const
	{
		return mRenderTargetTextureRHI;
	}

	RenderTarget::~RenderTarget()
	{

	}

	float RenderTarget::getDisplayGamma() const
	{
		if (GEngine == nullptr)
		{
			return 2.2f;
		}
		else
		{
			if (Math::abs(GEngine->mDisplayGamma) <= 0.0f)
			{
				GEngine->mDisplayGamma = 2.2f;
			}
			return GEngine->mDisplayGamma;
		}
	}
}