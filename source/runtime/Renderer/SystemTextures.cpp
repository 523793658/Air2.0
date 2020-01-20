#include "SystemTextures.h"
#include "PostProcess/RenderTargetPool.h"
#include "RHIUtilities.h"
namespace Air
{
	TGlobalResource<SystemTextures> GSystemTextures;

	void SystemTextures::internalInitializedTextures(RHICommandListImmediate& RHICmdList, ERHIFeatureLevel::Type inFeatureLevel)
	{
		{
			PooledRenderTargetDesc desc(PooledRenderTargetDesc::create2DDesc(int2(1, 1), PF_B8G8R8A8, ClearValueBinding::White, TexCreate_HideInVisualizeTexture, TexCreate_RenderTargetable | TexCreate_NoFastClear, false));
			desc.bAutoWritable = false;
			GRenderTargetPool.findFreeElement(RHICmdList, desc, mWhiteDummy, TEXT("WhiteDummy"));

			setRenderTarget(RHICmdList, mWhiteDummy->getRenderTargetItem().mTargetableTexture, TextureRHIRef(), ESimpleRenderTargetMode::EClearColorExistingDepth);
			RHICmdList.copyToResolveTarget(mWhiteDummy->getRenderTargetItem().mTargetableTexture, mWhiteDummy->getRenderTargetItem().mShaderResourceTexture, ResolveParams());
		}

		{
			PooledRenderTargetDesc desc(PooledRenderTargetDesc::create2DDesc(int2(1, 1), PF_B8G8R8A8, ClearValueBinding::White, TexCreate_HideInVisualizeTexture, TexCreate_RenderTargetable | TexCreate_NoFastClear, false));
			desc.bAutoWritable = false;
			GRenderTargetPool.findFreeElement(RHICmdList, desc, mBlackDummy, TEXT("BlackDummy"));

			setRenderTarget(RHICmdList, mWhiteDummy->getRenderTargetItem().mTargetableTexture, TextureRHIRef(), ESimpleRenderTargetMode::EClearColorExistingDepth);
			RHICmdList.copyToResolveTarget(mWhiteDummy->getRenderTargetItem().mTargetableTexture, mWhiteDummy->getRenderTargetItem().mShaderResourceTexture, ResolveParams());
		}
	}

	void SystemTextures::releaseDynamicRHI()
	{
		mWhiteDummy.safeRelease();
		mBlackDummy.safeRelease();
		mBlackAlphaOneDummy.safeRelease();
		mPerlinNoiseGradient.safeRelease();
		mPerlinNoise3D.safeRelease();
		mSSAORandomization.safeRelease();
		mPreintegratedGF.safeRelease();
		mMaxFP16Depth.safeRelease();
		mDepthDummy.safeRelease();
		mGreenDummy.safeRelease();
		mMidGrayDummy.safeRelease();
		bTexturesInitialized = false;
	}

}