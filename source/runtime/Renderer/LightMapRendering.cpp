#include "LightMapRendering.h"
#include "PrimitiveSceneProxy.h"
#include "PrimitiveSceneInfo.h"
#include "ShaderParameterUtils.h"
namespace Air
{
	IMPLEMENT_CONSTANT_BUFFER_STRUCT(PrecomputedLightingParameters, TEXT("mPrecomputedLightingBuffer"));


	void getPrecomputeLightingParameters(ERHIFeatureLevel::Type featureLevel, PrecomputedLightingParameters& parameters, const IndirectLightingCache* lightingCache,
		const IndirectLightingCacheAllocation* lightingAllocation, const LightCacheInterface* LCI)
	{

	}

	void ConstantLightMapPolicy::set(RHICommandList& RHICmdList, const VertexParametersType* vertexShaderParameters, const PixelParametersType* pixelShaderParameters, Shader* vertexShader, Shader* pixelShader, const VertexFactory* vertexFactory, const MaterialRenderProxy* materialRenderProxy, const SceneView* view)const
	{
		BOOST_ASSERT(vertexFactory);
		vertexFactory->set(RHICmdList);
	}

	void ConstantLightMapPolicy::setMesh(RHICommandList& RHICmdList, const SceneView& view, const PrimitiveSceneProxy* primitiveSceneProxy, const VertexParametersType* vertexShaderParameter, const PixelParametersType* pixelShaderParameters, Shader* vertexShader, Shader* pixelShader, const VertexFactory* vertexFactory, const MaterialRenderProxy* materialRenderProxy, const LightCacheInterface* LCI) const
	{
		ConstantBufferRHIParamRef precomputedLightingBuffer = nullptr;
		if (LCI)
		{
			precomputedLightingBuffer = LCI->getPrecomputedLightingBuffer();
		}
		if (!precomputedLightingBuffer && primitiveSceneProxy && primitiveSceneProxy->getPrimitiveSceneInfo())
		{
			precomputedLightingBuffer = primitiveSceneProxy->getPrimitiveSceneInfo()->mIndirectLightingCacheConstantBuffer;
			
		}
		if (!precomputedLightingBuffer)
		{
			precomputedLightingBuffer = GEmptyPrecomputedLightingConstantBuffer.getConstantBufferRHI();
		}
		if (vertexShaderParameter && vertexShaderParameter->mBufferParameter.isBound())
		{
			setConstantBufferParameter(RHICmdList, vertexShader->getVertexShader(), vertexShaderParameter->mBufferParameter, precomputedLightingBuffer);
		}
		if (pixelShaderParameters && pixelShaderParameters->mBufferParameter.isBound())
		{
			setConstantBufferParameter(RHICmdList, pixelShader->getPixelShader(), pixelShaderParameters->mBufferParameter, precomputedLightingBuffer);
		}
	}

	void EmptyPrecomputedLightingConstantBuffer::initDynamicRHI()
	{
		PrecomputedLightingParameters parameters;
		getPrecomputeLightingParameters(GMaxRHIFeatureLevel, parameters);
		setContentsNoUpdate(parameters);
		Supper::initDynamicRHI();
	}


	TGlobalResource<EmptyPrecomputedLightingConstantBuffer> GEmptyPrecomputedLightingConstantBuffer;
}
