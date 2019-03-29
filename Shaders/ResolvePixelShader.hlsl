#include "Common.hlsl"
#if FEATURE_LEVEL >= FEATURE_LEVEL_SM5

Texture2DMS<float4>	UnresolvedSurface;

void MainDepth(
	float2 inUV : TEXCOORD0,
	out float outDepth : SV_DEPTH
)
{
	float2 surfaceDimension;
	int numSurfaceSamples;
	UnresolvedSurface.GetDimensions(surfaceDimension.x, surfaceDimension.y, numSurfaceSamples);
	int2 intUV = trunc(inUV);
	float resolvedDepth = UnresolvedSurface.Load(intUV, 0).r;
	for (int sampleIndex = 1; sampleIndex < numSurfaceSamples; ++sampleIndex)
	{
		float s = UnresolvedSurface.Load(intUV, sampleIndex).r;
		resolvedDepth = max(resolvedDepth, s);
	}
	outDepth = resolvedDepth;
}

uint SingleSampleIndex;
void MainSingleSample(
	float2 inUV : TEXCOORD0,
	out float4 outColor : SV_Target0
)
{
	float2 surfaceDimensions;
	int numSurfaceSamples;
	UnresolvedSurface.GetDimensions(surfaceDimensions.x, surfaceDimensions.y, numSurfaceSamples);
	int2 intUV = trunc(inUV);
	outColor = UnresolvedSurface.Load(intUV, SingleSampleIndex);
}
#else

Texture2D	UnresolvedSurfaceNonMS;
void MainDepthNonMS(
	float2 inUV : TEXCOORD0,
	out float outDepth : SV_DEPTH
)
{
	int3 intUV = int3(trunc(inUV), 0);
	float s = UnresolvedSurfaceNonMS.Load(intUV).r;
	outDepth = s;
}
#endif