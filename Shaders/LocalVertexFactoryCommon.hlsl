
struct VertexFactoryInterpolantsVSToPS
{
	float4 Color : COLOR0;
	float3 Normal : TEXCOORD0;
	float4 Tangent : TEXCOORD1;
};

float3 getTangentToWorld0(VertexFactoryInterpolantsVSToPS interpolants)
{
	return interpolants.Normal;
}

float4 getTangentToWorld2(VertexFactoryInterpolantsVSToPS interpolants)
{
	return interpolants.Tangent;
}

float4 getColor(VertexFactoryInterpolantsVSToPS interpolants)
{
#if INTERPOLANT_VERTEX_COLOR
	return interpolants.Color;
#else
	return 0;
#endif
}

void setTangents(inout VertexFactoryInterpolantsVSToPS interpolants, float3 inTangentToWorld0, float3 inTangentToWorld2, float inTangentToWorldSign)
{
	interpolants.Normal = inTangentToWorld0;
	interpolants.Tangent = float4(inTangentToWorld2, inTangentToWorldSign);
}
