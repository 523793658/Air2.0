#pragma once
#include "EngineMininal.h"
#include "Textures/SlateShaderResource.h"
#include "RHIResource.h"
#include "RenderResource.h"
namespace Air
{
	class SlateRenderTargetRHI : public TSlateTexture<Texture2DRHIRef>,
		public RenderResource
	{
	public:
		SlateRenderTargetRHI(Texture2DRHIRef inRenderTargetTexture, uint32 width, uint32 height)
			:TSlateTexture(inRenderTargetTexture),
			mWidth(width),
			mHeight(height)
		{

		}

		virtual void initDynamicRHI() override {}

		virtual void releaseDynamicRHI() override
		{
			mShaderResource.safeRelease();
		}

		virtual uint32 getWidth() const override { return mWidth; }
		virtual uint32 getHeight() const override { return mHeight; }

		ENGINE_API void setRHIRef(Texture2DRHIRef inRenderTargetTexture, uint32 width, uint32 height);

		Texture2DRHIRef getRHIRef() const { return mShaderResource; }

	private:
		uint32 mWidth;
		uint32 mHeight;
	};
}