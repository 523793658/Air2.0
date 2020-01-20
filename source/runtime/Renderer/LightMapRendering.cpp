#include "LightMapRendering.h"
#include "PrimitiveSceneProxy.h"
#include "PrimitiveSceneInfo.h"
#include "ShaderParameterUtils.h"
namespace Air
{
	


	void getPrecomputeLightingParameters(ERHIFeatureLevel::Type featureLevel, PrecomputedLightingParameters& parameters, const IndirectLightingCache* lightingCache,
		const IndirectLightingCacheAllocation* lightingAllocation, const LightCacheInterface* LCI)
	{

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
