#include "DeferredShadingCommon.hlsl"
#include "DeferredLightingCommon.hlsl"
DeferredLightData setupLightDataForStandardDeferred()
{
	DeferredLightData lightData;
	lightData.LightPositionAndInvRadius = float4(DeferredLightConstants.LightPosition, DeferredLightConstants.LightInvRadius);
	lightData.LightColorAndFalloffExponent = float4(DeferredLightConstants.LightColor, DeferredLightConstants.LightFalloffExponent);
	lightData.LightDirection = DeferredLightConstants.NormalizedLightDirection;
	lightData.SpotAnglesAndSourceRadius = float4(DeferredLightConstants.SpotAngles, DeferredLightConstants.SourceRadius, DeferredLightConstants.SourceLength);
	lightData.MinRoughness = DeferredLightConstants.MinRoughness;
	lightData.ContactShadowLength = DeferredLightConstants.ContactShadowLength;
	lightData.ShadowedBits = DeferredLightConstants.ShadowedBits;
	lightData.bInverseSquared = INVERSE_SQUARED_FALLOFF;
	lightData.bRadialLight = RADIAL_ATTENUATION;
	lightData.bSpotLight = RADIAL_ATTENUATION;
	return lightData;
}

void DirectionalPixelMain(
	float2	inUV : TEXCOORD0,
	float3	screenVector : TEXCOORD1,
	float4	SVPos : SV_POSITION,
	out float4 outColor : SV_Target0
)
{
	outColor = 0;
	float3 cameraVector = normalize(screenVector);
	ScreenSpaceData screenSpaceData = getScreenSpaceData(inUV);
	BRANCH if (screenSpaceData.GBuffer.ShadingModelID > 0)
	{
		float sceneDepth = calcSceneDepth(inUV);
		float3 worldPosition = screenVector * sceneDepth + View.WorldCameraOrigin;
		DeferredLightData lightData = setupLightDataForStandardDeferred();
		uint2 random = scrambleTEA(uint2(SVPos.xy));
		random.x ^= View.Random;
		random.y ^= View.Random;
		outColor = getDynamicLighting(worldPosition, cameraVector, screenSpaceData.GBuffer, screenSpaceData.AmbientOcclusion, screenSpaceData.GBuffer.ShadingModelID, lightData, random);
	}
}