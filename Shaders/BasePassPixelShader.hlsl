#include "Common.hlsl"
#include "Material.hlsl"
#include "BasePassCommon.hlsl"
#include "VertexFactory.hlsl"
#include "DeferredShadingCommon.hlsl"

void PixelShaderInOut_MainPS(
	VertexFactoryInterpolantsVSToPS interpolants,
	BasePassInterpolantsVSToPS basePassInterpolants,
	in PixelShaderIn In,
	inout PixelShaderOut Out)
{
	float4 OutGBufferD = 0;
	float4 OutGBufferE = 0;
	MaterialPixelParameters materialParameters = getMaterialPixelParameters(interpolants, In.SvPosition);
	PixelMaterialInputs pixelMaterialInputs;
	{
		float4 screenPosition = SvPositionToResolvedScreenPosition(In.SvPosition);
		float3 translatedWorldPosition = SvPositionToResolvedTranslatedWorld(In.SvPosition);
		calcMaterialParametersEx(materialParameters, pixelMaterialInputs, In.SvPosition, screenPosition, In.bIsFrontFace, translatedWorldPosition, translatedWorldPosition);
	}


	half3 baseColor = getMaterialBaseColor(pixelMaterialInputs);
	half metallic = getMaterialMetallic(pixelMaterialInputs);
	half specular = getMaterialSpecular(pixelMaterialInputs);
	float roughness = getMaterialRoughness(pixelMaterialInputs);

	float3 localBaseColor = baseColor;
	float localSpecular = specular;


	GBufferData GBuffer = (GBufferData)0;
	GBuffer.WorldNormal = materialParameters.WorldNormal;
	GBuffer.BaseColor = baseColor;
	GBuffer.Metallic = metallic;
	GBuffer.Specular = specular;
	GBuffer.Roughness = roughness;

	#if MATERIAL_SHADINGMODEL_UNLIT
		GBuffer.ShadingModelID = SHADINGMODELID_UNLIT;
	#elif MATERIAL_SHADINGMODEL_DEFAULT_LIT
		GBuffer.ShadingModelID = SHADINGMODELID_DEFAULT_LIT;
	#endif
	
	GBuffer.SpecularColor = lerp(0.08 * localSpecular.xxx, localBaseColor, metallic.xxx);
	
#if USES_GBUFFER
	encodeGBuffer(GBuffer, Out.MRT[1], Out.MRT[2], Out.MRT[3], OutGBufferD, OutGBufferE);
#endif
}

#define PIXELSHADEROUTPUT_BASEPASS 1
#define PIXELSHADEROUTPUT_MRT0 (!USES_GBUFFER || !SELECTIVE_BASEPASS_OUTPUTS)
#define PIXELSHADEROUTPUT_MRT1 (USES_GBUFFER &&(!SELECTIVE_BASEPASS_OUTPUTS || !MATERIAL_SHADERINGMODE_UNLIT))
#define PIXELSHADEROUTPUT_MRT2 (USES_GBUFFER &&(!SELECTIVE_BASEPASS_OUTPUTS || !MATERIAL_SHADERINGMODE_UNLIT))
#define PIXELSHADEROUTPUT_MRT3 (USES_GBUFFER &&(!SELECTIVE_BASEPASS_OUTPUTS || !MATERIAL_SHADERINGMODE_UNLIT))


#include "PixelShaderOutputCommon.hlsl"