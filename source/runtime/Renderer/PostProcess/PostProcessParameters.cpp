#include "PostProcess/PostProcessParameters.h"
#include "PostProcess/RenderingCompositionGraph.h"
#include "ShaderParameterUtils.h"
#include "SystemTextures.h"
namespace Air
{
	void PostProcessPassParameters::bind(const ShaderParameterMap& parameterMap)
	{
		mBilinearTextureSampler0.bind(parameterMap, TEXT("BilinearTextureSampler0"));
		mBilinearTextureSampler1.bind(parameterMap, TEXT("BilinearTextureSampler1"));
		mViewportSize.bind(parameterMap, TEXT("ViewportSize"));
		mViewportRect.bind(parameterMap, TEXT("ViewportRect"));
		mScreenPosToPixel.bind(parameterMap, TEXT("ScreenPosToPixel"));

		for (uint32 i = 0; i < ePId_Input_Max; ++i)
		{
			mPostprocessInputParameter[i].bind(parameterMap, String::printf(TEXT("PostprocessInput%d"), i).c_str());
			mPostprocessInputParameterSampler[i].bind(parameterMap, String::printf(TEXT("PostprocessInput%dSampler"), i).c_str());
			mPostprocessInputSizeParameter[i].bind(parameterMap, String::printf(TEXT("PostprocessInput%dSize"), i).c_str());
			mPostprocessInputMinMaxParameter[i].bind(parameterMap, String::printf(TEXT("PostprocessInput%dMinMax"), i).c_str());

		}
	}

	void PostProcessPassParameters::setPS(const PixelShaderRHIParamRef& shaderRHI, const RenderingCompositePassContext& context, SamplerStateRHIParamRef filter /* = TStaticSamplerState<>::getRHI() */, EFallbackColor fallbackColor /* = eFC_0000 */, SamplerStateRHIParamRef* filterOverrideArray /* = 0 */)
	{
		set(shaderRHI, context, context.mRHICmdList, filter, fallbackColor, filterOverrideArray);
	}

	template<typename TRHICmdList>
	void PostProcessPassParameters::setCS(const ComputeShaderRHIParamRef& shaderRHI, const RenderingCompositePassContext& context, TRHICmdList& RHICmdList, SamplerStateRHIParamRef filter /* = TStaticSamplerState<>::getRHI() */, EFallbackColor fallbackColor /* = eFC_0000 */, SamplerStateRHIParamRef* filterOverrideArray /* = nullptr */)
	{
		set(shaderRHI, context, RHICmdList, filter, fallbackColor, filterOverrideArray);
	}

	template void PostProcessPassParameters::setCS<RHICommandListImmediate>(const ComputeShaderRHIParamRef& shaderRHI, const RenderingCompositePassContext& context, RHICommandListImmediate& RHICmdList, SamplerStateRHIParamRef filter /* = TStaticSamplerState<>::getRHI() */, EFallbackColor fallbackColor /* = eFC_0000 */, SamplerStateRHIParamRef* filterOverrideArray /* = nullptr */);

	template void PostProcessPassParameters::setCS<RHIAsyncComputeCommandListImmediate>(const ComputeShaderRHIParamRef& shaderRHI, const RenderingCompositePassContext& context, RHIAsyncComputeCommandListImmediate& RHICmdList, SamplerStateRHIParamRef filter /* = TStaticSamplerState<>::getRHI() */, EFallbackColor fallbackColor /* = eFC_0000 */, SamplerStateRHIParamRef* filterOverrideArray /* = nullptr */);

	void PostProcessPassParameters::setVS(const VertexShaderRHIParamRef& shaderRHI, const RenderingCompositePassContext& context, SamplerStateRHIParamRef filter /* = TStaticSamplerState<>::getRHI() */, EFallbackColor fallbackColor /* = eFC_0000 */, SamplerStateRHIParamRef* filterOverrideArray /* = nullptr */)
	{
		set(shaderRHI, context, context.mRHICmdList, filter, fallbackColor, filterOverrideArray);
	}

	template<typename ShaderRHIParamRef, typename TRHICmdList>
	void PostProcessPassParameters::set(const ShaderRHIParamRef& shaderRHI, const RenderingCompositePassContext& context, TRHICmdList& RHICmdList, SamplerStateRHIParamRef filter, EFallbackColor fallbackColor, SamplerStateRHIParamRef* filterOverrideArray /* = nullptr */)
	{
		RenderingCompositeOutput* output = context.mPass->getOutput(ePId_Output0);
		BOOST_ASSERT(output);
		BOOST_ASSERT(filterOverrideArray || filter);
		BOOST_ASSERT(!filterOverrideArray || !filter);

		if (mBilinearTextureSampler0.isBound())
		{
			RHICmdList.setShaderSampler(shaderRHI, mBilinearTextureSampler0.getBaseIndex(), TStaticSamplerState<SF_Bilinear>::getRHI());
		}

		if (mBilinearTextureSampler1.isBound())
		{
			RHICmdList.setShaderSampler(shaderRHI, mBilinearTextureSampler1.getBaseIndex(), TStaticSamplerState<SF_Bilinear>::getRHI());
		}

		if (mViewportSize.isBound() || mScreenPosToPixel.isBound() || mViewportRect.isBound())
		{
			IntRect localViewport = context.getViewport();
			int2 viewportOffset = localViewport.min;
			int2 viewportExtent = localViewport.size();

			{
				float4 value(viewportExtent.x, viewportExtent.y, 1.0f / viewportExtent.x, 1.0f / viewportExtent.y);
				setShaderValue(RHICmdList, shaderRHI, mViewportSize, value);
			}

			{
				setShaderValue(RHICmdList, shaderRHI, mViewportRect, context.getViewport());
			}
			{
				float4 screenPosToPixelValue(
					viewportExtent.x * 0.5f,
					-viewportExtent.y * 0.5f,
					viewportExtent.x * 0.5f - 0.5f + viewportOffset.x,
					viewportExtent.y * 0.5f - 0.5f + viewportOffset.y);
				setShaderValue(RHICmdList, shaderRHI, mScreenPosToPixel, screenPosToPixelValue);
			}
		}
		
		IntRect contextViewportRect = context.isViewportValid() ? context.getViewport() : IntRect(0, 0, 0, 0);
		const int2 sceneRTSize = SceneRenderTargets::get(RHICmdList).getBufferSizeXY();
		float4 baseSceneTexMinMax(
			((float)contextViewportRect.min.x / sceneRTSize.x),
			((float)contextViewportRect.min.y / sceneRTSize.y),
			((float)contextViewportRect.max.x / sceneRTSize.x),
			((float)contextViewportRect.max.y / sceneRTSize.y)
		);

		IPooledRenderTarget* fallbackTexture = nullptr;
		switch (fallbackColor)
		{
		case Air::eFC_0000:
			fallbackTexture = GSystemTextures.mBlackDummy; 
			break;
		case Air::eFC_0001:
			fallbackTexture = GSystemTextures.mBlackAlphaOneDummy;
			break;
		case Air::eFC_1111:
			fallbackTexture = GSystemTextures.mWhiteDummy;
			break;
		default:
			BOOST_ASSERT(false);
			break;
		}

		for (uint32 id = 0; id < (uint32)ePId_Input_Max; ++id)
		{
			RenderingCompositeOutputRef* outputRef = context.mPass->getInput((EPassInputId)id);
			if (!outputRef)
			{
				break;
			}
			const auto featureLevel = context.getFeatureLevel();

			RenderingCompositeOutput* input = outputRef->getOutput();

			TRefCountPtr<IPooledRenderTarget> inputPooledElement;

			if (input)
			{
				inputPooledElement = input->requestInput();
			}

			SamplerStateRHIParamRef localFilter = filterOverrideArray ? filterOverrideArray[id] : filter;
			if (inputPooledElement)
			{
				BOOST_ASSERT(!inputPooledElement->isFree());
				const TextureRHIRef& srcTexture = inputPooledElement->getRenderTargetItem().mShaderResourceTexture;

				setTextureParameter(RHICmdList, shaderRHI, mPostprocessInputParameter[id], mPostprocessInputParameterSampler[id], localFilter, srcTexture);

				if (mPostprocessInputSizeParameter[id].isBound() || mPostprocessInputMinMaxParameter[id].isBound())
				{
					float width = inputPooledElement->getDesc().mExtent.x;
					float height = inputPooledElement->getDesc().mExtent.y;

					float2 onePPInputPixelUVSize = float2(1.0f / width, 1.0f / height);
					float4 textureSize(width, height, onePPInputPixelUVSize.x, onePPInputPixelUVSize.y);
					setShaderValue(RHICmdList, shaderRHI, mPostprocessInputSizeParameter[id], textureSize);

					float4 PPInputMinMax = baseSceneTexMinMax;
					PPInputMinMax.z -= onePPInputPixelUVSize.x;
					PPInputMinMax.w -= onePPInputPixelUVSize.y;
					setShaderValue(RHICmdList, shaderRHI, mPostprocessInputMinMaxParameter[id], PPInputMinMax);
				}
			}
			else
			{
				setTextureParameter(RHICmdList, shaderRHI, mPostprocessInputParameter[id], mPostprocessInputParameterSampler[id], localFilter, fallbackTexture->getRenderTargetItem().mTargetableTexture);

				float4 dummy(1, 1, 1, 1);
				setShaderValue(RHICmdList, shaderRHI, mPostprocessInputSizeParameter[id], dummy);
				setShaderValue(RHICmdList, shaderRHI, mPostprocessInputMinMaxParameter[id], dummy);
			}
		}
	}

	Archive & operator << (Archive& ar, PostProcessPassParameters& p)
	{
		ar << p.mBilinearTextureSampler0 << p.mBilinearTextureSampler1 << p.mViewportSize << p.mViewportRect;
		for (uint32 i = 0; i < ePId_Input_Max; i++)
		{
			ar << p.mPostprocessInputParameter[i];
			ar << p.mPostprocessInputParameterSampler[i];
			ar << p.mPostprocessInputSizeParameter[i];
			ar << p.mPostprocessInputMinMaxParameter[i];
		}
		return ar;
	}
}