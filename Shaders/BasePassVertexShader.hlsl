#include "BasePassVertexCommon.hlsl"


void Main(VertexFactoryInput input, out FBasePassVSOutput output)
{
#if	INSTANCED_STEREO
#else
	uint EyeIndex = 0;
	ResolvedView = ResolveView();
#endif

	VertexFactoryIntermediates VFIntermediates = getVertexFactoryIntermediates(input);

	float4 worldPosition = vertexFactoryGetWorldPosition(input);
	float4 clipSpacePosition;
	float3x3 tangentToLocal = vertexFactoryGetTangentToLocal(input, VFIntermediates);
	MaterialVertexParameters vertexParameters = getMaterialVertexParameters(input, VFIntermediates, worldPosition.xyz, tangentToLocal);

#if USING_TESSELLATION
#else
	{
		float4 rasterizedWorldPosition = vertexFactoryGetRasterizedWorldPosition(input, VFIntermediates, worldPosition);
		clipSpacePosition = mul(rasterizedWorldPosition, ResolvedView.TranslatedWorldToClip);
		output.Position = clipSpacePosition; 
	}
#endif
	output.FactoryInterpolants = vertexFactoryGetInterpolants(input, VFIntermediates, vertexParameters);



}