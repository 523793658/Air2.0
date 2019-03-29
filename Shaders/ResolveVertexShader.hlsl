#include "Common.hlsl"

void Main(
	float2 inPosition	: ATTRIBUTE0,
	float2 inUV : ATTRIBUTE1,
	out float2 outUV : TEXCOORD0,
	out float4 outPosition : SV_POSITION
)
{
	outPosition = float4(inPosition, 0, 1);
	outUV = inUV;
}