#include "Common.hlsl"
#include "Material.hlsl"
#include "BasePassCommon.hlsl"
#include "VertexFactory.hlsl"
#include "DeferredShadingCommon.hlsl"


void getSkyLighting(float3 worldNormal, float2 lightmapUV, out float3 outDiffuseLighting, out float3 outSubsurfaceLighting)
{
	outDiffuseLighting = 0;
	outSubsurfaceLighting = 0;
	
#if ENABLE_SKY_LIGHT
	float skyVisibility = 1;
	float geometryTerm = 1;
	float3 skyLightingNormal = worldNormal;

	float3 diffuseLookup = getEffecitiveSkySHDiffuse(skyLightingNormal) * resolvedView.SkyLightColor.rgb;

	outDiffuseLighting += diffuseLookup * (skyVisiblity * geometryTerm);

#if MATERIAL_SHADINGMODEL_TWOSIDED_FOLIAGE
	float3 backfaceDiffuseLookup = getEffecitiveSkySHDiffuse(-worldNormal) * resolvedView.SkyLightColor.rgb;
	outSubsurfaceLighting += backfaceDiffuseLookup * skyVisiblity;
#endif // MATERIAL_SHADINGMODEL_TWOSIDED_FOLIAGE


#endif // ENABLE_SKY_LIGHT

	

}

void getPrecomputedIndirectLightingAndSkyLight(
	MaterialPixelParameters materialParameters,
	VertexFactoryInterpolantsVSToPS interpolants,
	BasePassInterpolantsVSToPS basePassInterpolants,
	float3 diffuseDir,
	out float3 outDiffuseLighting,
	out float3 outSubsurfaceLighting,
	out float outIndirectIrradiance)
{
	outIndirectIrradiance = 0;
	outDiffuseLighting = 0;
	outSubsurfaceLighting = 0;
	float2 skyOcclusionUV = 0;

	float3 skyDiffuseLighting;
	float3 skySubsurfaceLighting;
	getSkyLighting(diffuseDir, skyOcclusionUV, skyDiffuseLighting, skySubsurfaceLighting);

	outSubsurfaceLighting += skySubsurfaceLighting;
	outDiffuseLighting += skyDiffuseLighting;
}

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
	float materialAO = getMaterialAmbientOcclusion(pixelMaterialInputs);

	float roughness = getMaterialRoughness(pixelMaterialInputs);

	float3 localBaseColor = baseColor;
	float localSpecular = specular;


	GBufferData GBuffer = (GBufferData)0;
	GBuffer.WorldNormal = materialParameters.WorldNormal;
	GBuffer.BaseColor = baseColor;
	GBuffer.Metallic = metallic;
	GBuffer.Specular = specular;
	GBuffer.Roughness = roughness;

	float3 subsurfaceColor = 0;

	#if MATERIAL_SHADINGMODEL_UNLIT
		GBuffer.ShadingModelID = SHADINGMODELID_UNLIT;
	#elif MATERIAL_SHADINGMODEL_DEFAULT_LIT
		GBuffer.ShadingModelID = SHADINGMODELID_DEFAULT_LIT;
	#endif
	
	GBuffer.SpecularColor = lerp(0.08 * localSpecular.xxx, localBaseColor, metallic.xxx);

	GBuffer.DiffuseColor = localBaseColor - localBaseColor * metallic;
	

	half3 diffuseColor = 0;
	half3 color = 0;
	float indirectIrradiance = 0;

#if !MATERIAL_SHADINGMODEL_UNLIT
	float3 diffuseDir = materialParameters.WorldNormal;
	float3 diffuseColorForIndirect = GBuffer.DiffuseColor;

	float3 diffuseIndirectLighting;
	float3 subsurfaceIndirectLighting;
	getPrecomputedIndirectLightingAndSkyLight(materialParameters, interpolants, basePassInterpolants, diffuseDir, diffuseIndirectLighting, subsurfaceIndirectLighting, indirectIrradiance);

	float indirectOcclusion = 1.0f;
	float2 nearestResolvedDepthScreenUV = 0;

	diffuseColor += (diffuseIndirectLighting * diffuseColorForIndirect + subsurfaceIndirectLighting * subsurfaceColor) * materialAO;




#endif
#if NEEDS_BASEPASS_FOGGING
	float4 vertexFog = basePassInterpolants.VertexFog;
#else
	float4 vertexFog = float4(0, 0, 0, 1);
#endif // NEEDS_BASEPASS_FOGGING

#if (MATERIAL_SHADINGMODEL_DEFAULT_LIT || MATERIAL_SHADINGMODEL_SUBSURFACE) && (MATERIALBLENDING_TRANSLUCENT || MATERIALBLENDING_ADDITIVE) && !SIMPLE_FORWARD_SHADING && !FORWARD_SHADING
	color += getTranslucencyVolumeLighting(materialParameters, pixelMaterialInputs, basePassInterpolants, GBuffer, indirectIrradiance);
#endif // (MATERIAL_SHADINGMODEL_DEFAULT_LIT || MATERIAL_SHADINGMODEL_SUBSURFACE) && (MATERIALBLENDING_TRANSLUCENT || MATERIALBLENDING_ADDITIVE) && !SIMPLE_FORWARD_SHADING && !FORWARD_SHADING

#if !MATERIAL_SHADINGMODEL_UNLIT && USE_DEVELOPMENT_SHADERS
	//color = 
#endif // !MATERIAL_SHADINGMODEL_UNLIT && USE_DEVELOPMENT_SHADERS

	half3 emissive = getMaterialEmissive(pixelMaterialInputs);

#if !(MATERIAL_SHADINGMODEL_SUBSURFACE_PROFILE && USES_GBUFFER)
	color += diffuseColor;
#endif // !(MATERIAL_SHADINGMODEL_SUBSURFACE_PROFILE && USES_GBUFFER)

	color += emissive;

#if MATERIALBLENDING_ALPHACOMPOSITE
	Out.MRT[0] = half4(color * vertexFog.a + vertexFog.rgb * opacity, opacity);
	Out.MRT[0] = RETURN_COLOR(Out.MRT[0]);
#elif MATERIALBLENDING_TRANSLUCENT
	Out.MRT[0] = half4(color * vertexFog.a + vertexFog.rgb, opacity);
	Out.MRT[0] = RETURN_COLOR(Out.MRT[0]);
#elif MATERIALBLENDING_ADDITIVE
	Out.MRT[0] = half4(color * vertexFog.a * opacity, 0.0);
	Out.MRT[0] = RETURN_COLOR(Out.MRT[0]);
#elif MATERIALBLENDING_MODULATE
	half3 foggedColor = lerp(float3(1, 1, 1), color, vertexFog.aaa * vertex.aaa);
	Out.MRT[0] = half4(foggedColor, opacity);
#else
	{
		LightAccumulator lightAccumulator = (LightAccumulator)0;

		color = color * vertexFog.a + vertexFog.rgb;

#if MATERIAL_SHADINGMODEL_SUBSURFACE_PROFILE && USES_GBUFFER
#else
		lightAccumulator_Add(lightAccumulator, color, 0, 1.0f, false);
#endif
		Out.MRT[0] = RETURN_COLOR(lightAccumulator_GetResult(lightAccumulator));

#if !USES_GBUFFER
		Out.MRT[0].a = 0;
#endif // !USES_GBUFFER

	}
#endif // MATERIALBLENDING_ALPHACOMPOSITE



#if USES_GBUFFER
	GBuffer.IndirectIrradiance = indirectIrradiance;

	encodeGBuffer(GBuffer, Out.MRT[1], Out.MRT[2], Out.MRT[3], OutGBufferD, OutGBufferE);
#endif
}

#define PIXELSHADEROUTPUT_BASEPASS 1
#define PIXELSHADEROUTPUT_MRT0 (!USES_GBUFFER || !SELECTIVE_BASEPASS_OUTPUTS)
#define PIXELSHADEROUTPUT_MRT1 (USES_GBUFFER &&(!SELECTIVE_BASEPASS_OUTPUTS || !MATERIAL_SHADERINGMODE_UNLIT))
#define PIXELSHADEROUTPUT_MRT2 (USES_GBUFFER &&(!SELECTIVE_BASEPASS_OUTPUTS || !MATERIAL_SHADERINGMODE_UNLIT))
#define PIXELSHADEROUTPUT_MRT3 (USES_GBUFFER &&(!SELECTIVE_BASEPASS_OUTPUTS || !MATERIAL_SHADERINGMODE_UNLIT))


#include "PixelShaderOutputCommon.hlsl"