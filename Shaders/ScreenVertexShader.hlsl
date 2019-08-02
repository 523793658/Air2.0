#include "Common.hlsl"

void Main(
	float2 inPosition	: ATTRIBUTE0,
	float2 inUV : ATTRIBUTE1,
	out ScreenVertexOutput output
)
{
	drawRectangle(float4(inPosition, 0, 1), inUV, output.Position, output.UV);
}

