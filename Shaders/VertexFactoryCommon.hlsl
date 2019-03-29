#include "Common.hlsl"
float4 transformLocalToTranslatedWorld(float3 localPosition)
{
	float3 rotatedPosition = Primitive.LocalToWorld[0].xyz*localPosition.xxx + Primitive.LocalToWorld[1].xyz * localPosition.yyy + Primitive.LocalToWorld[2].xyz * localPosition.zzz;
	return float4(rotatedPosition + (Primitive.LocalToWorld[3].xyz + ResolvedView.PreViewTranslation.xyz), 1.0);
	
}