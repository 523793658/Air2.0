#pragma once
#include "BRDF.hlsl"
float3 standardShading(float3 diffuseColor, float3 specularColor, float3 lobeRoughness, float3 lobeEnergy, float3 L, float3 V, float3 N)
{
	float3 H = normalize(V + L);
	float NoL = saturate(dot(N, L));
	float NoV = saturate(abs(dot(N, V)) + 1e-5);
	float NoH = saturate(dot(N, H));
	float VoH = saturate(dot(V, H));

	float D = D_GGX(lobeRoughness[1], NoH) * lobeEnergy[1];
	float Vis = Vis_SmithJointApprox(lobeRoughness[1], NoV, NoL);
	float3 F = F_Schlick(specularColor, VoH);

	float3 diffuse = Diffuse_Lambert(diffuseColor);
	return diffuse * lobeEnergy[2] + (D * Vis) * F;
}

float3 surfaceShading(GBufferData GBuffer, float3 lobeRoughness, float3 lobeEnergy, float3 L, float3 V, float3 N, int2 random)
{
	switch (GBuffer.ShadingModelID)
	{
	case SHADINGMODELID_UNLIT:
	case SHADINGMODELID_DEFAULT_LIT:
		return standardShading(GBuffer.DiffuseColor, GBuffer.SpecularColor, lobeRoughness, lobeEnergy, L, V, N);
	default:
		return 0;
	}
}