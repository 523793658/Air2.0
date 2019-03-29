#include "Common.hlsl"
#include "Material.hlsl"

#include "VertexFactory.hlsl"

#define vertexFactoryGetInterpolants	vertexFactoryGetInterpolantsVSToPS

struct FBasePassVSToPS
{
	VertexFactoryInterpolantsVSToPS FactoryInterpolants;
	float4 Position : SV_Position;
};

#define FBasePassVSOutput FBasePassVSToPS