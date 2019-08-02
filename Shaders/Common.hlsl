#pragma once

#ifndef FORCE_FLOATS
#define FORCE_FLOATS 0
#endif

#if (!(COMPILER_GLSL_ES2 || COMPILER_GLSL_ES3_1) && !METAL_PROFILE) || FORCE_FLOATS
	#define half float
	#define half1 float1
	#define half2 float2
	#define half3 float3
	#define half4 float4
	#define half3x3 float3x3
	#define half4x4 float4x4
	#define half4x3 float4x3
	#define fixed float
	#define fixed1 float1
	#define fixed2 float2
	#define fixed3 float3
	#define fixed4 float4
	#define fixed3x3 float3x3
	#define fixed4x4 float4x4
	#define fixed4x3 float4x3

#endif

#if PIXELSHADER
	#define MaterialFloat half
	#define MaterialFloat2 half2
	#define MaterialFloat3 half3
	#define MaterialFloat4 half4
	#define MaterialFloat3x3 half3x3
	#define MaterialFloat4x4 half4x4
	#define MaterialFloat4x3 half4x3
#else
	#define MaterialFloat float
	#define MaterialFloat2 float2
	#define MaterialFloat3 float3
	#define MaterialFloat4 float4
	#define MaterialFloat3x3 float3x3
	#define MaterialFloat4x4 float4x4
	#define MaterialFloat4x3 float4x3
#endif
#include "GeneratedConstantBuffers.hlsl"

#include "InstancedStereo.hlsl"

#include "Definitions.hlsl"

const static MaterialFloat PI = 3.1415926535897932f;

#define FEATURE_LEVEL_ES2	1
#define FEATURE_LEVEL_ES3_1	2
#define FEATURE_LEVEL_SM3	3
#define FEATURE_LEVEL_SM4	4
#define FEATURE_LEVEL_SM5	5
#define FEATURE_LEVEL_MAX	6

#define A8_SAMPLE_MASK	.a
#if PS4_PROFILE
	#define FEATURE_LEVEL FEATURE_LEVEL_SM5
#elif SM5_PROFILE
	#define FEATURE_LEVEL FEATURE_LEVEL_SM5
#elif SM4_PROFILE
	#define FEATURE_LEVEL FEATURE_LEVEL_SM4
#elif SWITCH_PROFILE || SWITCH_PROFILE_FORWORD
	#undef ES3_1_PROFILE
	#if SWITCH_PROFILE
		#define FEATURE_LEVEL FEATURE_LEVEL_SM5
	#else
		#define ES3_1_PROFILE 1
	#endif

	#undef A8_SAMPLE_MASK
	#define A8_SAMPLE_MASK .r

#else
	#error Add your platform here
	#define FEATURE_LEVEL FEATURE_LEVEL_MAX
#endif

float square(float x)
{
	return x * x;
}

float2 square(float2 x)
{
	return x * x;
}

float3 square(float3 x)
{
	return x * x;
}

float4 square(float4 x)
{
	return x * x;
}


float pow2(float x)
{
	return x * x;
}

float2 pow2(float2 x)
{
	return x * x;
}

float3 pow2(float3 x)
{
	return x * x;
}

float4 pow2(float4 x)
{
	return x * x;
}


float pow3(float x)
{
	return x * x * x;
}

float2 pow3(float2 x)
{
	return x * x * x;
}

float3 pow3(float3 x)
{
	return x * x * x;
}

float4 pow3(float4 x)
{
	return x * x * x;
}

float pow4(float x)
{
	float xx = x * x;
	return xx * xx;
}

float2 pow4(float2 x)
{
	float2 xx = x * x;
	return xx * xx;
}

float3 pow4(float3 x)
{
	float3 xx = x * x;
	return xx * xx;
}

float4 pow4(float4 x)
{
	float4 xx = x * x;
	return xx * xx;
}

float pow5(float x)
{
	float xx = x * x;
	return xx * xx * x;
}

float2 pow5(float2 x)
{
	float2 xx = x * x;
	return xx * xx * x;
}


float3 pow5(float3 x)
{
	float3 xx = x * x;
	return xx * xx * x;
}

float4 pow5(float4 x)
{
	float4 xx = x * x;
	return xx * xx * x;
}

float pow6(float x)
{
	float xx = x * x;
	return xx * xx * xx;
}

float2 pow6(float2 x)
{
	float2 xx = x * x;
	return xx * xx * xx;
}


float3 pow6(float3 x)
{
	float3 xx = x * x;
	return xx * xx * xx;
}

float4 pow6(float4 x)
{
	float4 xx = x * x;
	return xx * xx * xx;
}

MaterialFloat clampedPow(MaterialFloat x, MaterialFloat y)
{
	return pow(max(abs(x), 0.000001f), y);
}

MaterialFloat2 clampedPow(MaterialFloat2 x, MaterialFloat2 y)
{
	return pow(max(abs(x), 0.000001f), y);
}

MaterialFloat3 clampedPow(MaterialFloat3 x, MaterialFloat3 y)
{
	return pow(max(abs(x), 0.000001f), y);
}

MaterialFloat4 clampedPow(MaterialFloat4 x, MaterialFloat4 y)
{
	return pow(max(abs(x), 0.000001f), y);
}

MaterialFloat phongShadingPow(MaterialFloat x, MaterialFloat y)
{
	return clampedPow(x, y);
}

Texture2D CustomDepthTexture;
SamplerState CustomDepthTextureSampler;
Texture2D SceneDepthTexture;
SamplerState SceneDepthTextureSampler;

MaterialFloat4 Texture2DSampleLevel(Texture2D tex, SamplerState s, float2 uv, MaterialFloat mip)
{
	return tex.SampleLevel(s, uv, mip);
}

MaterialFloat4 TextureCubeSampleLevel(TextureCube tex, SamplerState s, float3 uv, MaterialFloat mip)
{
	return tex.SampleLevel(s, uv, mip);
}

MaterialFloat4 Texture2DSample(Texture2D tex, SamplerState s, float2 uv)
{
#if COMPUTESHADER
	return tex.SampleLevel(s, uv, 0);
#else
	return tex.Sample(s, uv);
#endif // COMPUTESHADER

}

MaterialFloat4 TextureCubeSample(TextureCube tex, SamplerState s, float3 uv)
{
#if COMPUTESHADER
	return tex.SampleLevel(s, uv, 0);
#else
	return tex.Sample(s, uv);
#endif // COMPUTESHADER
}

uint reverseBits32(uint bits)
{
#if SM5_PROFILE && !METAL_SM5_PROFILE
	return reversebits(bits);
#else
	bits = (bits << 16) | (bits >> 16);
	bits = ((bits & 0x00ff00ff) << 8) | ((bits & 0xff00ff00) >> 8);
	bits = ((bits & 0x0f0f0f0f) << 4) | ((bits & 0xf0f0f0f0) >> 4);
	bits = ((bits & 0x33333333) << 2) | ((bits & 0xcccccccc) >> 2);
	bits = ((bits & 0x55555555) << 1) | ((bits & 0xaaaaaaaa) >> 1);
	return bits;
#endif
}



float convertFromDeviceZ(float deviceZ)
{
	return deviceZ * View.InvDeviceZToWorldZTransform[0] + View.InvDeviceZToWorldZTransform[1] + 1.0f / (deviceZ * View.InvDeviceZToWorldZTransform[2] - View.InvDeviceZToWorldZTransform[3]);
}

float calcSceneDepth(float2 screenUV)
{
#if SCENE_TEXTURES_DISABLED
	return 0.0f;
#else
#if FEATURE_LEVEL > FEATURE_LEVEL_ES3_1 || MOBILE_FORCE_FORCE_DEPTH_TEXTURE_READS
	return convertFromDeviceZ(Texture2DSampleLevel(SceneDepthTexture, SceneDepthTextureSampler, screenUV, 0).r);
#else

#endif // FEATURE_LEVEL > FEATURE_LEVEL_ES3_1 || MOBILE_FORCE_FORCE_DEPTH_TEXTURE_READS

#endif // SCENE_TEXTURES_DISABLED

}


struct PixelShaderIn
{
	float4 SvPosition;
	uint Coverage;
	bool bIsFrontFace;
};

struct PixelShaderOut
{
	float4 MRT[8];
	uint Coverage;
	float Depth;
};

float4 SvPositionToResolvedScreenPosition(float4 SvPosition)
{
	float2 pixelPos = SvPosition.xy - ResolvedView.ViewRectMin.xy;
	float3 NDCPos = float3((pixelPos * ResolvedView.ViewSizeAndInvSize.zw - 0.5f) * float2(2, -2), SvPosition.z);
	return float4(NDCPos.xyz, 1) * SvPosition.w;
}
	
float3 SvPositionToResolvedTranslatedWorld(float4 SvPosition)
{
	float4 homWorldPos = mul(float4(SvPosition.xyz, 1.0), ResolvedView.SVPositionToTranslatedWorld);
	return homWorldPos.xyz / homWorldPos.w;
}

float3 transformTangentVectorToWorld(float3x3 tangentToWorld, float3 inTangentVector)
{
	return mul(inTangentVector, tangentToWorld);
}

MaterialFloat3x3 getLocalToWorld3x3()
{
	return (MaterialFloat3x3)Primitive.LocalToWorld;
}
#define tangentBias(X) (X * 2.0f - 1.0f)

void drawRectangle(in float4 inPosition, in float2 inTexCoord, out float4 outPosition, out float2 outTexCoord)
{
	outPosition = inPosition;
	outPosition.xy = -1.0f + 2.0f * (DrawRectangleParameters.PosScaleBias.zw + (inPosition.xy * DrawRectangleParameters.PosScaleBias.xy)) * DrawRectangleParameters.InvTargetSizeAndTextureSize.xy;
	outPosition.xy *= float2(1, -1);
	outTexCoord.xy = (DrawRectangleParameters.UVScaleBias.zw + (inTexCoord.xy * DrawRectangleParameters.UVScaleBias.xy)) * DrawRectangleParameters.InvTargetSizeAndTextureSize.zw;
}

void drawRectangle(in float4 inPosition, out float4 outPosition)
{
	outPosition = inPosition;
	outPosition.xy = -1.0f + 2.0f * (DrawRectangleParameters.PosScaleBias.zw + (inPosition.xy * DrawRectangleParameters.PosScaleBias.xy)) * DrawRectangleParameters.InvTargetSizeAndTextureSize.xy;
	outPosition.xy *= float2(1, -1);
	
}

#if FEATURE_LEVEL >= FEATURE_LEVEL_SM5
#define TANGENTTOWORLD0		TEXCOORD10
#define TANGENTTOWORLD1		TEXCOORD11

#define TANGENTTOWORLD_INTERPOLATOR_BLOCK	float4 Normal : TEXCOORD10_centroid; float4 Tangent : TEXCOORD11_centroid;

#else 
#define TANGENTTOWORLD0		TEXCOORD10
#define TANGENTTOWORLD1		TEXCOORD11
#if METAL_PROFILE || COMPILER_GLSL_ES3_1
#define TANGENTTOWORLD_INTERPOLATOR_BLOCK float4 Normal : TANGENTTOWORLD0; float4 Tangent : TANGENTTOWORLD1
#else 
#define TANGENTTOWORLD_INTERPOLATOR_BLOCK MaterialFloat4 Normal : TANGENTTOWORLD0; MaterialFloat4 Tangent : TANGENTTOWORLD1
#endif
#endif

struct ScreenVertexOutput
{
#if METAL_PROFILE || COMPILER_GLSL_ES3_1
	noperspective float2 UV : TEXCOORD0;
#else
	noperspective MaterialFloat2 UV : TEXCOORD0;
#endif
	float4 Position : SV_POSITION;
};

#include "Random.hlsl"