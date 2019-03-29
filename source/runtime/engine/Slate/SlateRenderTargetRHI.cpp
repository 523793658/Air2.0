#include "Slate/SlateRenderTargetRHI.h"
namespace Air
{
	void SlateRenderTargetRHI::setRHIRef(Texture2DRHIRef inRenderTargetTexture, uint32 width, uint32 height)
	{
		BOOST_ASSERT(isInRenderingThread());
		mShaderResource = inRenderTargetTexture;
		mWidth = width;
		mHeight = height;
	}
}