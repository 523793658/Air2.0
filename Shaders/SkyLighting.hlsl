#include "Common.hlsl"
#include "DeferredShadingCommon.hlsl"
#include "ReflectionEnvironmentShared.hlsl"
#include "BRDF.hlsl"
#include "ShadingModels.hlsl"

Texture2D BentNormalAOTexture;
SamplerState BentNormalAOSampler;

Texture2D IrradianceTexture;
SamplerState IrradianceSampler;

float3 ContrastAndNormalizeMulAdd;
float4 OcclusionTintAndMinOcclusion;

void SkyLightDiffusePS(
	in noperspective float4 UVAndScreenPos : TEXCOORD0,
	out float4 outColor : SV_Target0)
{
	float2 uv = UVAndScreenPos.xy;
	uint shadingModelId = getShadingModelId(uv);
	float3 lighting = 0;
	BRANCH
	if (shadingModelId > 0)
	{
		ScreenSpaceData screenSpaceData = getScreenSpaceData(uv);
		GBufferData GBuffer = screenSpaceData.GBuffer;

		float skyVisibility = 1;
		float dotProductFactor = 1;
		float3 skyLightingNormal = GBuffer.WorldNormal;
		float3 diffuseIrradiance = 0;

		skyVisibility = min(skyVisibility, min(GBuffer.GBufferAO, screenSpaceData.AmbientOcclusion));

		float scalarFactors = lerp(skyVisibility, 1, OcclusionTintAndMinOcclusion.w);

		float3 diffuseColor = GBuffer.DiffuseColor;

		float3 diffuseLookup = getSkySHDiffuse(skyLightingNormal) * View.SkyLightColor.rgb;

		float directionalOcclusion = screenSpaceData.DirectionalOcclusion.r;
		diffuseLookup *= directionalOcclusion;

		lighting += ((scalarFactors * dotProductFactor) * diffuseLookup + (1 - skyVisibility) * OcclusionTintAndMinOcclusion.xyz) * diffuseColor;
		lighting += diffuseIrradiance * GBuffer.DiffuseColor * (GBuffer.GBufferAO * screenSpaceData.AmbientOcclusion);
	}
	{
		LightAccumulator lightAccumulator = (LightAccumulator)0;
		const bool bNeedsSeparateSubsurfaceLightAccumulation = shadingModelId == SHADINGMODELID_SUBSURFACE_PROFILE;

		lightAccumulator_Add(lightAccumulator, lighting, lighting, 1.0f, bNeedsSeparateSubsurfaceLightAccumulation);
		outColor = lightAccumulator_GetResult(lightAccumulator);
	}
}