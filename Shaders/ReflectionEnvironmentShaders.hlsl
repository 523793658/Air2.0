#include "Common.hlsl"
#include "SHCommon.hlsl"
#include "ReflectionEnvironmentShared.hlsl"
#include "MonteCarlo.hlsl"
int CubeFace;

float4 LowerHemisphereColor;

float3 SkyLightCaptureParameters;

TextureCube SourceTexture;
SamplerState SourceTextureSampler;

TextureCube ReflectionEnvironmentColorTexture;
SamplerState ReflectionEnvironmentColorTextureSampler;

int NumCaptureArrayMips;

int SourceMipIndex;

float CubemapMaxMip;

float3 getCubemapVector(float2 scaledUVs)
{
	float3 cubeCoordinates;
	if (CubeFace == 0)
	{
		cubeCoordinates = float3(1, -scaledUVs.y, -scaledUVs.x);
	}
	else if (CubeFace == 1)
	{
		cubeCoordinates = float3(-1, -scaledUVs.y, scaledUVs.x);
	}
	else if (CubeFace == 2)
	{
		cubeCoordinates = float3(scaledUVs.x, 1, scaledUVs.y);
	}
	else if (CubeFace == 3)
	{
		cubeCoordinates = float3(scaledUVs.x, -1, -scaledUVs.y);
	}
	else if (CubeFace == 4)
	{
		cubeCoordinates = float3(scaledUVs.x, -scaledUVs.y, 1);
	}
	else
	{
		cubeCoordinates = float3(-scaledUVs.x, -scaledUVs.y, -1);
	}
	return cubeCoordinates;
}

float4 sampleCubemap(float3 coordinates)
{
	return TextureCubeSampleLevel(SourceTexture, SourceTextureSampler, coordinates, SourceMipIndex);
}


float2 SinCosSourceCubemapRotation;

void CopyCubemapToCubeFaceColorPS(ScreenVertexOutput vsInput,
	out float4 outColor : SV_Target0
)
{
	float2 scaledUVs = vsInput.UV * 2 - 1;
	float3 cubeCoordinates = getCubemapVector(scaledUVs);
	cubeCoordinates.xy = float2(dot(cubeCoordinates.xy, float2(SinCosSourceCubemapRotation.y, -SinCosSourceCubemapRotation.x)), dot(cubeCoordinates.xy, SinCosSourceCubemapRotation));
	outColor = TextureCubeSampleLevel(SourceTexture, SourceTextureSampler, cubeCoordinates, 0);

	if (SkyLightCaptureParameters.x > 0)
	{
		if (cubeCoordinates.z < 0 && SkyLightCaptureParameters.z >= 1)
		{
			outColor.rgb = lerp(outColor.rgb, LowerHemisphereColor.rgb, LowerHemisphereColor.a);
		}
	}
	outColor.a = 1;
}

void DownsamplePS(
	ScreenVertexOutput vsOutput,
	out float4 outColor : SV_Target0
)
{
	float2 scaledUVs = vsOutput.UV * 2 - 1;
	float3 cubeCoordinates = getCubemapVector(scaledUVs);
	outColor = sampleCubemap(cubeCoordinates);
}

void ComputeBrightnessMain(
	in float4 UVAndScreenPos : TEXCOORD0,
	out float4 outColor : SV_Target0
)
{
	float3 averageColor = TextureCubeSampleLevel(ReflectionEnvironmentColorTexture, ReflectionEnvironmentColorTextureSampler, float3(1, 0, 0), NumCaptureArrayMips - 1).rgb;

	averageColor += TextureCubeSampleLevel(ReflectionEnvironmentColorTexture, ReflectionEnvironmentColorTextureSampler, float3(-1, 0, 0), NumCaptureArrayMips - 1).rgb;

	averageColor += TextureCubeSampleLevel(ReflectionEnvironmentColorTexture, ReflectionEnvironmentColorTextureSampler, float3(0, 1, 0), NumCaptureArrayMips - 1).rgb;

	averageColor = TextureCubeSampleLevel(ReflectionEnvironmentColorTexture, ReflectionEnvironmentColorTextureSampler, float3(0, -1, 0), NumCaptureArrayMips - 1).rgb;

	averageColor = TextureCubeSampleLevel(ReflectionEnvironmentColorTexture, ReflectionEnvironmentColorTextureSampler, float3(0, 0, 1), NumCaptureArrayMips - 1).rgb;

	averageColor = TextureCubeSampleLevel(ReflectionEnvironmentColorTexture, ReflectionEnvironmentColorTextureSampler, float3(0, 0, -1), NumCaptureArrayMips - 1).rgb;

	outColor = dot(averageColor / 6, 0.3333f);
}

float4 CoefficientMask0;
float4 CoefficientMask1;
float CoefficientMask2;

void DiffuseIrradianceCopyPS(
	ScreenVertexOutput psInput,
	out float4 outColor : SV_Target0
)
{
	float2 scaledUVs = psInput.UV * 2 - 1;
	float3 cubeCoordinates = normalize(getCubemapVector(scaledUVs));

	float squaredUVs = 1 + dot(scaledUVs, scaledUVs);

	float texelWeight = 4 / (sqrt(squaredUVs) * squaredUVs);

	ThreeBandSHVector shCoefficients = SHBasisFunction3(cubeCoordinates);
	float currentSHCoefficient = dot(shCoefficients.V0, CoefficientMask0) + dot(shCoefficients.V1, CoefficientMask1) + shCoefficients.V2 * CoefficientMask2;

	float3 texelLighting = sampleCubemap(cubeCoordinates).rgb;
	outColor = float4(texelLighting * currentSHCoefficient * texelWeight, texelWeight);
}

float4 Sample01;
float4 Sample23;

void DiffuseIrradianceAccumulatePS(
	ScreenVertexOutput psInput,
	out float4 outColor : SV_Target0)
{
	float4 accumulatedValue = 0;
	{
		float2 scaledUVs = saturate(psInput.UV + Sample01.xy) * 2 - 1;
		float3 cubeCoordinates = getCubemapVector(scaledUVs);
		accumulatedValue += sampleCubemap(cubeCoordinates);
	}
	{
		float2 scaledUVs = saturate(psInput.UV + Sample01.zw) * 2 - 1;
		float3 cubeCoordinates = getCubemapVector(scaledUVs);
		accumulatedValue += sampleCubemap(cubeCoordinates);
	}
	{
		float2 scaledUVs = saturate(psInput.UV + Sample23.xy) * 2 - 1;
		float3 cubeCoordinates = getCubemapVector(scaledUVs);
		accumulatedValue += sampleCubemap(cubeCoordinates);
	}
	{
		float2 scaledUVs = saturate(psInput.UV + Sample23.zw) * 2 - 1;
		float3 cubeCoordinates = getCubemapVector(scaledUVs);
		accumulatedValue += sampleCubemap(cubeCoordinates);
	}

	outColor = accumulatedValue / 4.0f;
}

#define NUM_FILTER_SAMPLES 1024

void FilterPS(
	ScreenVertexOutput psInput,
	out float4 outColor : SV_Target0)
{
	float2 scaledUVs = psInput.UV * 2.0 - 1.0;
	float3 cubeCoordinates = getCubemapVector(scaledUVs);

#if METAL_PROFILE
#define FILTER_CUBEMAP 0
#else 
#define FILTER_CUBEMAP 1
#endif

#if FILTER_CUBEMAP

	float3 N = normalize(cubeCoordinates);
	float roughness = computeReflectionCaptureRoughnessFromMip(SourceMipIndex, CubemapMaxMip);

	float4 filteredColor = 0;
	float weight = 0;
	LOOP
		for (int i = 0; i < NUM_FILTER_SAMPLES; i++)
		{
			float2 E = hammersley(i, NUM_FILTER_SAMPLES, 0);
			float3 H = tangentToWorld(importanceSampleGGX(E, roughness).xyz, N);
			float3 L = 2 * dot(N, H) * H - N;

			float NoL = saturate(dot(N, L));

			if (NoL > 0)
			{
				filteredColor += sampleCubemap(L) * NoL;
				weight += NoL;
			}
		}

	outColor = filteredColor / max(weight, 0.001);
#else
	outColor = SampleCubemap(cubeCoordinates);
#endif
}