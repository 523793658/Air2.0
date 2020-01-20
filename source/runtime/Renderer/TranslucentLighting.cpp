#include "DeferredShadingRenderer.h"
namespace Air
{
	int32 GUseTranslucentLightingVolumes = 0;

	void DeferredShadingSceneRenderer::clearTranslucentVolumeLighting(RHICommandListImmediate& RHICmdList, int32 viewIndex)
	{
		if (GUseTranslucentLightingVolumes && GSupportsVolumeTextureRendering)
		{
			SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);
			sceneContext.clearTranslucentVolumeLighting(RHICmdList, viewIndex);
		}
	}


	class ClearTrsnlucentLightingVolumeCS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(ClearTrsnlucentLightingVolumeCS, Global)
	public:
		static const int32 CLEAR_BLOCK_SIZE = 4;
		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			return isFeatureLevelSupported(parameters.mPlatform, ERHIFeatureLevel::SM5);
		}

		static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(parameters, outEnvironment);
			outEnvironment.setDefine(TEXT("CLEAR_COMPUTE_SHADER"), 1);
			outEnvironment.setDefine(TEXT("CLEAR_BLOCK_SIZE"), CLEAR_BLOCK_SIZE);
		}

		ClearTrsnlucentLightingVolumeCS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mAmbient0.bind(initializer.mParameterMap, TEXT("Ambietn0"));
			mDirectional0.bind(initializer.mParameterMap, TEXT("Directional0"));
			mAmbient1.bind(initializer.mParameterMap, TEXT("Ambient1"));
			mDirectional1.bind(initializer.mParameterMap, TEXT("Directional1"));
		}

		ClearTrsnlucentLightingVolumeCS()
		{}

		void setParameters(RHIAsyncComputeCommandListImmediate& RHICmdList, RHIUnorderedAccessView** volumeUAVs, int32 numUAVs)
		{
			BOOST_ASSERT(numUAVs == 4);
			RHIComputeShader* shaderRHI = getComputeShader();
			mAmbient0.setTexture(RHICmdList, shaderRHI, NULL, volumeUAVs[0]);
			mDirectional0.setTexture(RHICmdList, shaderRHI, NULL, volumeUAVs[1]);
			mAmbient1.setTexture(RHICmdList, shaderRHI, NULL, volumeUAVs[2]);
			mDirectional1.setTexture(RHICmdList, shaderRHI, NULL, volumeUAVs[3]);
		}

		void unsetParameters(RHIAsyncComputeCommandListImmediate& RHICmdList)
		{
			RHIComputeShader* shaderRHI = getComputeShader();
			mAmbient0.unsetUAV(RHICmdList, shaderRHI);
			mDirectional0.unsetUAV(RHICmdList, shaderRHI);
			mAmbient1.unsetUAV(RHICmdList, shaderRHI);
			mDirectional1.unsetUAV(RHICmdList, shaderRHI);
		}

		virtual bool serialize(Archive& ar) override
		{
			bool b = GlobalShader::serialize(ar);
			ar << mAmbient0;
			ar << mAmbient1;
			ar << mDirectional0;
			ar << mDirectional1;
			return b;
		}

	private:
		RWShaderParameter mAmbient0;
		RWShaderParameter mDirectional0;
		RWShaderParameter mAmbient1;
		RWShaderParameter mDirectional1;

	};

	IMPLEMENT_SHADER_TYPE(, ClearTrsnlucentLightingVolumeCS, TEXT("TranslucentLightInjectionShaders.hlsl"), TEXT("ClearTranslucentLightingVolumeCS"), SF_Compute);

	void DeferredShadingSceneRenderer::clearTranslucentVolumeLightingAsyncCompute(RHICommandListImmediate& RHICmdList)
	{
		SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);
		const int32 numUAVs = 4;
		for (int i = 0; i < mViews.size(); ++i)
		{
			RHIUnorderedAccessView* volumeUAVs[4] = {
				sceneContext.mTranslucencyLightingVolumeAmbient[(i * NumTranslucentVolumeRenderTargetSets)]->getRenderTargetItem().mUAV,
				sceneContext.mTranslucencyLightingVolumeDirectional[(i * NumTranslucentVolumeRenderTargetSets)]->getRenderTargetItem().mUAV,
				sceneContext.mTranslucencyLightingVolumeAmbient[(i * NumTranslucentVolumeRenderTargetSets) + 1]->getRenderTargetItem().mUAV,
				sceneContext.mTranslucencyLightingVolumeDirectional[(i * NumTranslucentVolumeRenderTargetSets) + 1]->getRenderTargetItem().mUAV
			};

			ClearTrsnlucentLightingVolumeCS* computeShader = *TShaderMapRef<ClearTrsnlucentLightingVolumeCS>(getGlobalShaderMap(mFeatureLevel));
			static const wstring EndComputeFenceName(TEXT("TranslucencyLightingVolumeClearEndComputeFence"));
			mTranslucencyLightingVolumeClearEndFence = RHICmdList.createComputeFence(EndComputeFenceName);

			static const wstring beginComputeFenceName(TEXT("TranslucencyLightingVolumeClearBeginComputeFence"));
			ComputeFenceRHIRef clearBeginFence = RHICmdList.createComputeFence(beginComputeFenceName);

			RHICmdList.transitionResources(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EGfxToCompute, volumeUAVs, numUAVs, clearBeginFence);

			const int32 translucencyLightingVolumeDim = getTranslucencyLightingVolumeDim();

			RHIAsyncComputeCommandListImmediate& RHICmdListComputeImmediate = RHICommandListExecutor::getImmediateAsyncComputeCommandList();
			{
				RHICmdListComputeImmediate.waitComputeFence(clearBeginFence);

				RHICmdListComputeImmediate.setComputeShader(computeShader->getComputeShader());

				computeShader->setParameters(RHICmdListComputeImmediate, volumeUAVs, numUAVs);

				int32 groupsPerDim = translucencyLightingVolumeDim / ClearTrsnlucentLightingVolumeCS::CLEAR_BLOCK_SIZE;
				dispatchComputeShader(RHICmdListComputeImmediate, computeShader, groupsPerDim, groupsPerDim, groupsPerDim);

				computeShader->unsetParameters(RHICmdListComputeImmediate);

				RHICmdListComputeImmediate.transitionResources(EResourceTransitionAccess::EReadable, EResourceTransitionPipeline::EComputeToGfx, volumeUAVs, numUAVs, mTranslucencyLightingVolumeClearEndFence);

			}

			RHIAsyncComputeCommandListImmediate::immediateDispatch(RHICmdListComputeImmediate);
		}
	}

	void DeferredShadingSceneRenderer::injectAmbientCubemapTranslucentVolumeLighting(RHICommandList& RHICmdList, const ViewInfo& view, int32 viewIndex)
	{

	}

	void DeferredShadingSceneRenderer::filterTranslucentVolumeLighting(RHICommandListImmediate& RHICmdList, const ViewInfo& view, const int32 viewIndex)
	{
	}

	
}