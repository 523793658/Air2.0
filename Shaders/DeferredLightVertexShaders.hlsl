#include "Common.hlsl"

void DirectionalVertexMain(
	in float2 inPosition	: ATTRIBUTE0,
	in float2 inUV : ATTRIBUTE1,
	out float2 outTexCoord : TEXCOORD0,
	out float3 outScreenVector : TEXCOORD1,
	out float4 outPosition : SV_POSITION
)
{
	drawRectangle(float4(inPosition.xy, 0, 1), inUV, outPosition, outTexCoord);
	outScreenVector = mul(float4(outPosition.xy, 1, 0), View.ScreenToTranslatedWorld).xyz;
}