#pragma once
#include "LightAccumulator.hlsl"
#include "ShadingModels.hlsl"
#define REFERENCE_QUALITY	0

struct DeferredLightData
{
	float4 LightPositionAndInvRadius;
	float4 LightColorAndFalloffExponent;
	float3 LightDirection;
	float4 SpotAnglesAndSourceRadius;
	float MinRoughness;
	float ContactShadowLength;

	bool bInverseSquared;
	bool bRadialLight;
	bool bSpotLight;
	uint ShadowedBits;
};

float3 areaLightSpecular(DeferredLightData lightData, inout float3 lobeRoughness, inout float3 toLight, inout float3 L, float3 V, float3 N)
{
	float3 lobeEnergy = 1;
	lobeRoughness = max(lobeRoughness, lightData.MinRoughness);
	float3 m = lobeRoughness * lobeRoughness;

	const float sourceRadius = lightData.SpotAnglesAndSourceRadius.z;
	const float sourceLength = lightData.SpotAnglesAndSourceRadius.w;

	float3 R = reflect(-V, N);

	float invDistToLight = rsqrt(dot(toLight, toLight));

	float a = square(lobeRoughness[1]);
	R = lerp(N, R, (1 - a) * (sqrt(1 - 1) + a));
	R = normalize(R);

	BRANCH
	if (sourceLength > 0)
	{

	}

	BRANCH
	if (sourceRadius > 0)
	{

	}

	L = normalize(toLight);
	return lobeEnergy;
}

float4 getDynamicLighting(float3 worldPosition, float3 cameraVector, GBufferData GBuffer, float ambientOcclusion, uint shadingModelID, DeferredLightData lightData, uint2 random)
{
	LightAccumulator lightAccumulator = (LightAccumulator)0;
	float3 V = -cameraVector;
	float3 N = GBuffer.WorldNormal;
	float3 L = lightData.LightDirection;
	float3 toLight = L * GBuffer.Depth;
	float NoL = saturate(dot(N, L));
	float distanceAttenuation = 1;
	float lightRadiusMask = 1;
	float spotFalloff = 1;

	if (lightData.bRadialLight)
	{

	}
	else
	{

	}

	lightAccumulator.EstimatedCost += 0.3f;

	BRANCH
	if (lightRadiusMask > 0 && spotFalloff > 0)
	{
		float surfaceShadow = 1.0f;
		float subsurfaceShadow = 1;

		BRANCH
		if (lightData.ShadowedBits)
		{


		}
		else
		{
			surfaceShadow = ambientOcclusion;
		}

		float surfaceAttenuation = (distanceAttenuation * lightRadiusMask * spotFalloff) * surfaceShadow;

		lightAccumulator.EstimatedCost += 0.3f;

		const bool bNeedsSeparatedSubsurfaceLightAccumulation = GBuffer.ShadingModelID == SHADINGMODELID_SUBSURFACE_PROFILE;
		const float3 lightColor = lightData.LightColorAndFalloffExponent.rgb;

#if NON_DIRECTIONAL_DIRECT_LIGHTTING
#else
		const float clearCoatRoughness = GBuffer.CustomData.y;
		float3 lobeRoughness = float3(clearCoatRoughness, GBuffer.Roughness, 1);

#if REFERENCE_QUALITY

#else
		float3 lobeEnergy = areaLightSpecular(lightData, lobeRoughness, toLight, L, V, N);
		{
			float3 surfaceLighting = surfaceShading(GBuffer, lobeRoughness, lobeEnergy, L, V, N, random);

			lightAccumulator_Add(lightAccumulator, surfaceLighting, (1.0f / PI), lightColor * (NoL * surfaceAttenuation), bNeedsSeparatedSubsurfaceLightAccumulation);
		}
#endif // REFERENCE_QUALITY


#endif // NON_DIRECTIONAL_DIRECT_LIGHTTING


	}

	return lightAccumulator_GetResult(lightAccumulator);
}
