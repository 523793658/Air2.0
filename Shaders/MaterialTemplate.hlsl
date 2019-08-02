#include "ConstantBuffers/Material.hlsl"
#define NUM_MATERIAL_TEXCOORDS_VERTEX %s
#define NUM_MATERIAL_TEXCOORDS %s

#define FrontFaceSemantic SV_IsFrontFace
#define FIsFrontFace bool
	half getfloatFacingSign(FIsFrontFace bIsFrontFace)
	{
		return bIsFrontFace ? +1 : -1;
	}

#if MTERIAL_TWOSIDED_SEPARATE_PASS
	#define OPTIONAL_IsFrontFace
	static const FIsFrontFace bIsFrontFace = 1;
#else
	#define OPTIONAL_IsFrontFace , in FIsFrontFace bIsFrontFace : FrontFaceSemantic
#endif

struct PixelMaterialInputs
{
%s
};

MaterialFloat4 processMaterialLinearColorTextureLookup(MaterialFloat4 textureValue)
{
    return textureValue;
}

%s


struct MaterialPixelParameters
{
#if NUM_MATERIAL_TEXCOORDS
    float2 TexCoords[NUM_MATERIAL_TEXCOORDS];
#endif

	half4 VertexColor;
	half3 WorldNormal;
	half3 ReflectionVector;
	half3 CameraVector;
	float4 SvPosition;
	half TwoSidedSign;
	half3x3 TangentToWorld;
};

struct MaterialVertexParameters
{
	float3 WorldPosition;
	half3x3 TangentToWorld;
	half4 VertexColor;

#if NUM_MATERIAL_TEXCOORDS_VERTEX
	float2 TexCoords[NUM_MATERIAL_TEXCOORDS_VERTEX];
#endif
};

half getMaterialOpacityClipValue()
{
	%s;
}

half getMaterialAmbientOcclusionRaw(PixelMaterialInputs pixelMaterialInputs)
{
	return pixelMaterialInputs.AmbientOcclusion;
}

half getMaterialAmbientOcclusion(PixelMaterialInputs pixelMaterialInputs)
{
	return saturate(getMaterialAmbientOcclusionRaw(pixelMaterialInputs));
}

MaterialFloat3 reflectionAboutCustomWorldNormal(MaterialPixelParameters parameters, MaterialFloat3 worldNormal, bool bNormalizeInputNormal)
{
	if(bNormalizeInputNormal)
	{
		worldNormal = normalize(worldNormal);
	}
	return -parameters.CameraVector + worldNormal * dot(worldNormal, parameters.CameraVector) * 2.0;
}

half3x3 assembleTangentToWorld(half3 tangentToWorld0, half4 tangentToWorld1)
{
	half3 tangentToWorld2 = cross(tangentToWorld0, tangentToWorld1.xyz) * tangentToWorld1.w;
	return half3x3(tangentToWorld0, tangentToWorld2, tangentToWorld1.xyz);
}

half3 getMaterialNormalRaw(PixelMaterialInputs pixelMaterialInputs)
{
	return pixelMaterialInputs.Normal;
}

half3 getMaterialNormal(MaterialPixelParameters parameters, PixelMaterialInputs pixelMaterialInputs)
{
	half3 retNormal;
	retNormal = getMaterialNormalRaw(pixelMaterialInputs);

	return retNormal;
}

float3 transformTangentNormalToWorld(in MaterialPixelParameters parameters, float3 tangentNormal)
{
	return normalize(float3(transformTangentVectorToWorld(parameters.TangentToWorld, tangentNormal)));
}


#if NUM_MATERIAL_TEXCOORDS
void getMaterialCustomizedUVs(MaterialVertexParameters parameters, out float2 outTexCoords[NUM_MATERIAL_TEXCOORDS])
{
%s
}
#endif

void calcPixelMaterialInputs(in out MaterialPixelParameters parameters, in out PixelMaterialInputs pixelMaterialInputs)
{
	
%s


%s
	
	float3 materialNormal = getMaterialNormal(parameters, pixelMaterialInputs);

#if MATERIAL_TANGENTSPACENORMAL
#if	SIMPLE_FORWARD_SHADING
	parameters.WorldNormal = float3(0, 0, 1);
#endif

#if FEATURE_LEVEL >= FEATURE_LEVEL_SM4
	materialNormal = normalize(materialNormal);
#endif
	
	parameters.WorldNormal = transformTangentNormalToWorld(parameters, materialNormal);
#else
	parameters.WorldNormal = normalize(materialNormal);
#endif

#if MATERIAL_TANGENTSPACENORMAL
	parameters.WorldNormal *= parameters.TwoSidedSign;
#endif
	parameters.ReflectionVector = reflectionAboutCustomWorldNormal(parameters, parameters.WorldNormal, false);
%s
}

#line %s

half3 getMaterialBaseColorRaw(PixelMaterialInputs pixelMaterialInputs)
{
	return pixelMaterialInputs.BaseColor;
}

half3 getMaterialBaseColor(PixelMaterialInputs pixelMaterialInputs)
{

	return saturate(getMaterialBaseColorRaw(pixelMaterialInputs));
}

half getMaterialMetallicRaw(PixelMaterialInputs pixelMaterialInputs)
{
	return pixelMaterialInputs.Metallic;
}

half getMaterialMetallic(PixelMaterialInputs pixelMaterialInputs)
{
	return saturate(getMaterialMetallicRaw(pixelMaterialInputs));
}

half getMaterialSpecularRaw(PixelMaterialInputs pixelMaterialInputs)
{
	return pixelMaterialInputs.Specular;
}

half getMaterialSpecular(PixelMaterialInputs pixelMaterialInputs)
{
	return saturate(getMaterialSpecularRaw(pixelMaterialInputs));
}

half getMaterialRoughnessRaw(PixelMaterialInputs pixelMaterialInputs)
{
	return pixelMaterialInputs.Roughness;
}

half getMaterialRoughness(PixelMaterialInputs pixelMaterialInputs)
{
	return saturate(getMaterialRoughnessRaw(pixelMaterialInputs));
}

MaterialPixelParameters makeInitializedMaterialPixelParameters()
{
	MaterialPixelParameters MPP;
	MPP = (MaterialPixelParameters)0;
	MPP.TangentToWorld = float3x3(1, 0, 0, 0, 0, 1, 0, 1, 0);
	return MPP;
}

void calcMaterialParametersEx(
	in out MaterialPixelParameters parameters,
	in out PixelMaterialInputs pixelMaterialInputs,
	float4 SvPosition,
	float4 screenPosition,
	FIsFrontFace bIsFrontFace,
	float3 translatedWorldPosition,
	float3 translatedWorldPositionExcludingShaderOffsets)
{
	parameters.SvPosition = SvPosition;
	calcPixelMaterialInputs(parameters, pixelMaterialInputs);
}