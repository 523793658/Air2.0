#include "Common.hlsl"
void MainVS(
	in float4 inPosition : ATTRIBUTE0,
	out noperspective float3 outWorldPosition : TEXCOORD0,
	out float4 outPosition : SV_POSITION
)
{
	drawRectangle(inPosition, outPosition);
	outWorldPosition = outPosition.xyz;
}

void MainPS(
	in noperspective float3 inWorldPosition : TEXCOORD0,
	float4 svPosition : SV_POSITION,
	out float4 outColor : SV_Target0
)
{
	float3 uv = mul(float4(inWorldPosition.xy, 1, 0), View.ScreenToTranslatedWorld).xyz;
	outColor = TextureCubeSample(View.SkyBoxTexture, View.SkyBoxTextureSampler, uv);
}