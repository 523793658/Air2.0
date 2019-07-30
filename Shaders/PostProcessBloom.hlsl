#include "Common.hlsl"
#include "PostProcessCommon.hlsl"

float4 ColorScale4;

void MainPostprocessCommonVS(
	in float4 inPosition : ATTRIBUTE0,
	in float2 inTexCoord : ATTRIBUTE1,
	out noperspective float4 outTexCoord : TEXCOORD0,
	out float4 outPosition : SV_POSITION
)
{
	drawRectangle(inPosition, inTexCoord, outPosition, outTexCoord.xy);
	outTexCoord.zw = outPosition.xy;
}