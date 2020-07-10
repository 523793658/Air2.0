#include "DeferredShadingRenderer.h"
#include "RenderResource.h"
#include "BasePassRendering.h"
namespace Air
{
	int32 GLightCullingQuality = 1;

	class MinimalDummyForwardLightingResources : public RenderResource
	{
	public:
		ForwardLightingViewResources mForwardLightingResources;

		virtual ~MinimalDummyForwardLightingResources()
		{}

		virtual void initRHI()
		{
			if (GMaxRHIFeatureLevel >= ERHIFeatureLevel::SM4)
			{
				if (GMaxRHIFeatureLevel >= ERHIFeatureLevel::SM5)
				{
					mForwardLightingResources.mForwardLocalLightBuffer.initialize(sizeof(float4), sizeof(ForwardLocalLightData) / sizeof(float4), PF_A32B32G32R32F, BUF_Dynamic);
					mForwardLightingResources.mNumCulledLightsGrid.initialize(sizeof(uint32), 1, PF_R32_UINT);
					const bool bSupportFormatConversion = RHISupportsBufferLoadTypeConversion(GMaxRHIShaderPlatform);

					if (bSupportFormatConversion)
					{
						mForwardLightingResources.mCulledLightDataGrid.initialize(sizeof(uint16), 1, PF_R16_UINT);
					}
					else
					{
						mForwardLightingResources.mCulledLightDataGrid.initialize(sizeof(uint32), 1, PF_R32_UINT);
					}

					mForwardLightingResources.mForwardLightData.ForwardLocalLightBuffer = mForwardLightingResources.mForwardLocalLightBuffer.mSRV;
					mForwardLightingResources.mForwardLightData.NumCulledLightsGrid = mForwardLightingResources.mNumCulledLightsGrid.mSRV;
					mForwardLightingResources.mForwardLightData.CulledLightDataGrid = mForwardLightingResources.mCulledLightDataGrid.mSRV;
				}
				else
				{
					mForwardLightingResources.mForwardLightData.ForwardLocalLightBuffer = GNullColorVertexBuffer.mVertexBufferSRV;
					mForwardLightingResources.mForwardLightData.NumCulledLightsGrid = GNullColorVertexBuffer.mVertexBufferSRV;
					mForwardLightingResources.mForwardLightData.CulledLightDataGrid = GNullColorVertexBuffer.mVertexBufferSRV;
				}
				mForwardLightingResources.mForwardLightDataConstantBuffer = TConstantBufferRef<ForwardLightData>::createConstantBufferImmediate(mForwardLightingResources.mForwardLightData, ConstantBuffer_MultiFrame);
			}
		}

		virtual void releaseRHI()
		{
			mForwardLightingResources.release();
		}
	};


	ForwardLightingViewResources* getMinimalDymmyForwardLightingResource()
	{
		static TGlobalResource<MinimalDummyForwardLightingResources>* GMinimalDummyForwardLightingResources = nullptr;
		if (!GMinimalDummyForwardLightingResources)
		{
			GMinimalDummyForwardLightingResources = new TGlobalResource<MinimalDummyForwardLightingResources>();
		}
		return &GMinimalDummyForwardLightingResources->mForwardLightingResources;
	}

	void DeferredShadingSceneRenderer::computeLightGrid(RHICommandListImmediate& RHICmdList, bool bNeedLightGrid, SortedLightSetSceneInfo& sortedLightSet)
	{
		if (!bNeedLightGrid || mFeatureLevel < ERHIFeatureLevel::SM5)
		{
			for (auto& view : mViews)
			{
				view.mForwardLightingResources = getMinimalDymmyForwardLightingResource();
			}
			return;
		}


		static const auto allowStaticLighting = false;
		const bool bAllowStaticLighting = false;
		const bool bAllowFormatConversion = RHISupportsBufferLoadTypeConversion(GMaxRHIShaderPlatform);

		bool bAnyViewUsesForwardLighting = false;

		for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
		{
			ViewInfo& view = mViews[viewIndex];
			bAnyViewUsesForwardLighting |= view.bTranslucentSurfaceLighting || shouldRenderVolumetricFog();
		}

		const bool bCullLightsToGrid = GLightCullingQuality && (mViewFamily.mEngineShowFlags.DirectLighting && (isForwardShadingEnabled(mShaderPlatform) || bAnyViewUsesForwardLighting || isRayTracingEnabled() || shouldUseClusteredDeferredShading()));

		bClusteredShadingLightsInLightGrid = bCullLightsToGrid;

		for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
		{
			ViewInfo& view = mViews[viewIndex];
		}

	}

	IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(ForwardLightData, "ForwardLightData");

	ForwardLightData::ForwardLightData()
	{
		Memory::memzero(*this);
		ForwardLocalLightBuffer = nullptr;
		NumCulledLightsGrid = nullptr;
		CulledLightDataGrid = nullptr;
	}
}