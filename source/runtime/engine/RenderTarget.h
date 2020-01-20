#pragma once
#include "EngineMininal.h"
#include "RHIResource.h"
namespace Air
{
	class RenderTarget
	{
	public:
		virtual ~RenderTarget();

		virtual int2 getSizeXY() const = 0;

		virtual void ProcessToggleFreezeCommand() {};

		virtual bool hasToggleFreezeCommand() { return false; };

		ENGINE_API virtual float getDisplayGamma() const;

		ENGINE_API virtual const Texture2DRHIRef& getRenderTargetTexture() const;
	protected:
		Texture2DRHIRef mRenderTargetTextureRHI;
		
	};
}