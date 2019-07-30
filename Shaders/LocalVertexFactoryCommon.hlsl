#if FEATURE_LEVEL >= FEATURE_LEVEL_ES3_1
struct VertexFactoryInterpolantsVSToPS
{
	TANGENTTOWORLD_INTERPOLATOR_BLOCK
	float4 Color : COLOR0;

#if NUM_MATERIAL_TEXCOORDS
	float4 TexCoords[(NUM_MATERIAL_TEXCOORDS + 1) / 2] : TEXCOORD0;
#endif
};

float3 getTangentToWorld0(VertexFactoryInterpolantsVSToPS interpolants)
{
	return interpolants.Tangent.xyz;
}

float4 getTangentToWorld1(VertexFactoryInterpolantsVSToPS interpolants)
{
	return interpolants.Normal;
}

float4 getColor(VertexFactoryInterpolantsVSToPS interpolants)
{
#if INTERPOLANT_VERTEX_COLOR
	return interpolants.Color;
#else
	return 0;
#endif
}

void setTangents(inout VertexFactoryInterpolantsVSToPS interpolants, float3 inTangent, float3 inNormal, float inTangentToWorldSign)
{
	interpolants.Normal = float4(inNormal.xyz, inTangentToWorldSign);
	interpolants.Tangent = float4(inTangent, 0);
}

#if NUM_MATERIAL_TEXCOORDS

void setUV(inout VertexFactoryInterpolantsVSToPS interpolants, int uvIndex, float2 inValue)
{
	FLATTEN
	if (uvIndex % 2)
	{
		interpolants.TexCoords[uvIndex / 2].zw = inValue;
	}
	else
	{
		interpolants.TexCoords[uvIndex / 2].xy = inValue;
	}
}

float2 getUV(VertexFactoryInterpolantsVSToPS interpolants, int uvIndex)
{
	float4 uvVector = interpolants.TexCoords[uvIndex / 2];
	return uvIndex % 2 ? uvVector.zw : uvVector.xy;
}
#endif

#else

#endif
