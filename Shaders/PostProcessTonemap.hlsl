#include "Common.hlsl"
#include "PostprocessCommon.hlsl"
#include "TonemapCommon.hlsl"
void MainVS(
	in					float4	inPosition		:ATTRIBUTE0,
	in					float2	inTexCoord		:ATTRIBUTE1,
	out	noperspective	float2	outTexCoord		:TEXCOORD0,
	out	noperspective	float4	outFullViewUV	:TEXCOORD1,
	out					float4	outPosition		:SV_POSITION

)
{
#if NEEDTOSWITCHVERTICLEAXIS
	inTexCoord.y = 1.0 - inTexCoord.y;
#endif // NEEDTOSWITCHVERTICLEAXIS

	drawRectangle(inPosition, inTexCoord, outPosition, outTexCoord.xy);
	outFullViewUV.xy = outPosition.xy * float2(0.5, -0.5) + 0.5;

#if NEEDTOSWITCHVERTICLEAXIS
	outFullViewUV.y = 1.0 - outFullViewUV.y;
#endif // NEEDTOSWITCHVERTICLEAXIS

}

void MainPS(
	in noperspective float2 uv			: TEXCOORD0,
	in noperspective float4 fullViewUV : TEXCOORD1,
	float4	svPosition : SV_POSITION,
	out float4 outColor : SV_Target0
)
{
	outColor = 0;

	float2 sceneUV = uv.xy;

	half3 sceneColor = Texture2DSample(PostprocessInput0, PostprocessInput0Sampler, sceneUV).rgb;

#if USE_GAMMA_ONLY
	outColor.rgb = pow(sceneColor, InverseGamma.x);
#endif // USE_GAMMA_ONLY

}