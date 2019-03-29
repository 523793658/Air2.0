#pragma once
#include "RendererMininal.h"
#include "RenderResource.h"
#include "RHI.h"
#include "RHICommandList.h"
#include "RendererInterface.h"
namespace Air
{
	class SystemTextures : public RenderResource
	{
	public:
		SystemTextures()
			:mFeatureLevelInitializedTo(ERHIFeatureLevel::ES2)
			,bTexturesInitialized(false)
		{}

		inline void initializeTextures(RHICommandListImmediate& RHICmdList, ERHIFeatureLevel::Type inFeatureLevel)
		{
			if (bTexturesInitialized && mFeatureLevelInitializedTo >= inFeatureLevel)
			{
				return;
			}
			internalInitializedTextures(RHICmdList, inFeatureLevel);

		}
		virtual void releaseDynamicRHI();

		TRefCountPtr<IPooledRenderTarget> mWhiteDummy;

		TRefCountPtr<IPooledRenderTarget> mBlackDummy;

		TRefCountPtr<IPooledRenderTarget> mBlackAlphaOneDummy;

		TRefCountPtr<IPooledRenderTarget> mPerlinNoiseGradient;

		TRefCountPtr<IPooledRenderTarget> mPerlinNoise3D;

		TRefCountPtr<IPooledRenderTarget> mSSAORandomization;

		TRefCountPtr<IPooledRenderTarget> mPreintegratedGF;

		TRefCountPtr<IPooledRenderTarget> mMaxFP16Depth;

		TRefCountPtr<IPooledRenderTarget> mDepthDummy;

		TRefCountPtr<IPooledRenderTarget> mGreenDummy;

		TRefCountPtr<IPooledRenderTarget> mMidGrayDummy;

	protected:
		ERHIFeatureLevel::Type mFeatureLevelInitializedTo;
		bool bTexturesInitialized;
		void internalInitializedTextures(RHICommandListImmediate& RHICmdList, ERHIFeatureLevel::Type inFeatureLevel);
	};

	extern TGlobalResource<SystemTextures> GSystemTextures;
}