#include "Common.hlsl"
#define SHADINGMODELID_UNLIT				0
#define SHADINGMODELID_DEFAULT_LIT			1
#define SHADINGMODELID_SUBSURFACE			2
#define SHADINGMODELID_SUBSURFACE_PROFILE	5
#define SHADINGMODELID_UNLIT				0
#define SHADINGMODELID_UNLIT				0
#define SHADINGMODELID_MASK					0xF

float encodeShadingModelIDAndSelectiveOutputMask(uint ShadingModelID, uint SelectiveOutputMask)
{
	uint value = (ShadingModelID & SHADINGMODELID_MASK) | SelectiveOutputMask;
	return (float)value / (float)0xFF;
}

uint decodeShadingModelId(float inPackedChannel)
{
	return ((uint)round(inPackedChannel * (float)0xFF)) & SHADINGMODELID_MASK;
}
float3 encodeNormal(float3 N)
{
	return N * 0.5 + 0.5;
}
float3 decodeNormal(float3 N)
{
	return N * 2 - 1;
}

float3 encodeBaseColor(float3 BaseColor)
{
	return BaseColor;
}


float3 decodeBaseColor(float3 BaseColor)
{
	return BaseColor;
}



struct GBufferData
{
	float3 WorldNormal;
	float3 DiffuseColor;
	float3 SpecularColor;
	float3 BaseColor;
	float Metallic;
	float Specular;
	float Roughness;
	uint ShadingModelID;
	uint SelectiveOutputMask;
	float PerObjectGBufferData;
	float GBufferAO;
	float4 PrecomputedShadowFactors;
	
	float4 CustomData;
	float IndirectIrradiance;
	float CustomDepth;
	uint CustomStencil;
	float Depth;
	float3 StoredBaseColor;
	float3 StoredSpecular;
};

void encodeGBuffer(
	GBufferData GBuffer,
	out float4 outGBufferA,
	out float4 outGBufferB,
	out float4 outGBufferC,
	out float4 outGBufferD,
	out float4 outGBufferE,
	float quantizationBias = 0)
{
	if(GBuffer.ShadingModelID == SHADINGMODELID_UNLIT)
	{
		outGBufferA = 0;
		outGBufferB = 0;
		outGBufferC = 0;
		outGBufferD = 0;
		outGBufferE = 0;
	}
	else
	{
		outGBufferA.rgb = encodeNormal(GBuffer.WorldNormal);
		outGBufferA.a = GBuffer.PerObjectGBufferData;
		outGBufferB.r = GBuffer.Metallic;
		outGBufferB.g = GBuffer.Specular;
		outGBufferB.b = GBuffer.Roughness;
		outGBufferB.a = encodeShadingModelIDAndSelectiveOutputMask(GBuffer.ShadingModelID, GBuffer.SelectiveOutputMask);
		
		outGBufferC.rgb = encodeBaseColor(GBuffer.BaseColor);
		outGBufferC.a = GBuffer.GBufferAO;
		outGBufferD = GBuffer.CustomData;
		outGBufferE = GBuffer.PrecomputedShadowFactors;
	}
}

bool checkerFromPixelPos(uint2 pixelPos)
{
	return true;
}

bool checkerFromSceneColorUV(float2 uvSceneColor)
{
	uint2 pixelPos = uint2(uvSceneColor * View.BufferSizeAndInvSize.xy);
	return checkerFromPixelPos(pixelPos);
}

GBufferData decodeGBufferData(
	float4 inGBufferA,
	float4 inGBufferB,
	float4 inGBufferC,
	float4 inGBufferD,
	float4 inGBufferE,
	float inCustomNativeDepth,
	uint	inCustomStencil,
	float inSceneDepth,
	bool bGetNormalizedNormal,
	bool bChecker
)
{
	GBufferData GBuffer;
	GBuffer.WorldNormal = decodeNormal(inGBufferA.xyz);
	if (bGetNormalizedNormal)
	{
		GBuffer.WorldNormal = normalize(GBuffer.WorldNormal);
	}

	GBuffer.PerObjectGBufferData = inGBufferA.a;
	GBuffer.Metallic = inGBufferB.r;
	GBuffer.Specular = inGBufferB.g;
	GBuffer.Roughness = inGBufferB.b;
	GBuffer.ShadingModelID = decodeShadingModelId(inGBufferB.a);
	GBuffer.BaseColor = decodeBaseColor(inGBufferC.rgb);
#if ALLOW_STATIC_LIGHTING
#else
	GBuffer.GBufferAO = inGBufferC.a;
	GBuffer.IndirectIrradiance = 1;
#endif // ALLOW_STATIC_LIGHTING

	GBuffer.CustomData = 0;

	GBuffer.CustomDepth = convertFromDeviceZ(inCustomNativeDepth);
	GBuffer.CustomStencil = inCustomStencil;
	GBuffer.Depth = inSceneDepth;

	GBuffer.StoredBaseColor = GBuffer.BaseColor;
	GBuffer.StoredSpecular = GBuffer.Specular;

	{
		GBuffer.SpecularColor = lerp(0.08 * GBuffer.Specular.xxx, GBuffer.BaseColor, GBuffer.Metallic);

		GBuffer.DiffuseColor = GBuffer.BaseColor - GBuffer.BaseColor * GBuffer.Metallic;

	}

	return GBuffer;


}

GBufferData getGBufferData(float2 uv, bool bGetNormalizedNormal = true)
{
	float4 GBufferA = Texture2DSampleLevel(GBuffers.GBufferATexture, GBuffers.GBufferATextureSampler, uv, 0);
	float4 GBufferB = Texture2DSampleLevel(GBuffers.GBufferBTexture, GBuffers.GBufferBTextureSampler, uv, 0);
	float4 GBufferC = Texture2DSampleLevel(GBuffers.GBufferCTexture, GBuffers.GBufferCTextureSampler, uv, 0);
	float4 GBufferD = Texture2DSampleLevel(GBuffers.GBufferDTexture, GBuffers.GBufferDTextureSampler, uv, 0);
	float CustomNativeDepth = Texture2DSampleLevel(CustomDepthTexture, CustomDepthTextureSampler, uv, 0).r;

	uint CustomStencil = 0;
#if ALLOW_STATIC_LIGHTING
	float4 GBufferE = Texture2DSampleLevel(GBuffers.GBufferETexture, GBufferETextureSampler, uv, 0);
#else
	float4 GBufferE = 1;

#endif // ALLOW_STATIC_LIGHTING
	float SceneDepth = calcSceneDepth(uv);
	return decodeGBufferData(GBufferA, GBufferB, GBufferC, GBufferD, GBufferE, CustomNativeDepth, CustomStencil, SceneDepth, bGetNormalizedNormal, true);
}

struct ScreenSpaceData
{
	GBufferData GBuffer;
	float AmbientOcclusion;
	float2 DirectionalOcclusion;
};

ScreenSpaceData getScreenSpaceData(float2 uv, bool bGetNormalizedNormal = true)
{
	ScreenSpaceData o;
	o.GBuffer = getGBufferData(uv, bGetNormalizedNormal);
	o.AmbientOcclusion = 1;
	o.DirectionalOcclusion = 1;
	return o;
}
	