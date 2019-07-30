#include "VertexFactoryCommon.hlsl"
#include "LocalVertexFactoryCommon.hlsl"

struct VertexFactoryInput
{
	float4 Position : ATTRIBUTE0;
	float3 Tangent	: ATTRIBUTE1;
	float4 Normal	: ATTRIBUTE2;
	float4 Color	: ATTRIBUTE3;

#if NUM_MATERIAL_TEXCOORDS_VERTEX
#if NUM_MATERIAL_TEXCOORDS_VERTEX > 1
	float4 PackedTexCoords4[NUM_MATERIAL_TEXCOORDS_VERTEX / 2] : ATTRIBUTE4;
#endif

#if NUM_MATERIAL_TEXCOORDS_VERTEX == 1
	float2 PackedTexCoords2 : ATTRIBUTE4;
#elif NUM_MATERIAL_TEXCOORDS_VERTEX == 3
	float2 PackedTexCoords2 : ATTRIBUTE5;
#elif NUM_MATERIAL_TEXCOORDS_VERTEX == 5
	float2 PackedTexCoords2 : ATTRIBUTE6;
#elif NUM_MATERIAL_TEXCOORDS_VERTEX == 7
	float2 PackedTexCoords2 : ATTRIBUTE7;
#endif
};

struct VertexFactoryIntermediates
{
	half3x3 TangentToLocal;
	half3x3 TangentToWorld;
	half TangentToWorldSign;
	half4 Color;
};

half3x3 calcTangentToWorldNoScale(in half3x3 tangentToLocal)
{
	half3x3 localToWorld = getLocalToWorld3x3();
	half3 invScale = Primitive.InvNonUniformScale.xyz;
	localToWorld[0] *= invScale.x;
	localToWorld[1] *= invScale.y;
	localToWorld[2] *= invScale.z;
	return mul(tangentToLocal, localToWorld);
}

half3x3 calcTangentToLocal(VertexFactoryInput input)
{
	half3x3 result;
	half4 normal = tangentBias(input.Normal);
	half3 tangent = tangentBias(input.Tangent);
	half3 bTangent = cross(tangent, normal.xyz) * normal.w;
	result[0] = cross(normal.xyz, bTangent) * normal.w;
	result[1] = normal.xyz;
	result[2] = bTangent;
	return result;
}

half3x3 calcTangentToWorld(VertexFactoryInput input, half3x3 tangentToLocal)
{
	half3x3 tangentToWorld = calcTangentToWorldNoScale(tangentToLocal);
	return tangentToWorld;
}

VertexFactoryIntermediates getVertexFactoryIntermediates(VertexFactoryInput input)
{
	VertexFactoryIntermediates intermediates;
	intermediates.TangentToLocal = calcTangentToLocal(input);
	intermediates.TangentToWorld = calcTangentToWorld(input, intermediates.TangentToLocal);
	intermediates.TangentToWorldSign = tangentBias(input.Normal.w) * Primitive.LocalToWorldDeterminantSign;
	intermediates.Color = input.Color;
	return intermediates;
}



float4 calcWorldPosition(float4 Position)
{
	return transformLocalToTranslatedWorld(Position.xyz);
}

float4 vertexFactoryGetWorldPosition(VertexFactoryInput input)
{
	return calcWorldPosition(input.Position);
}

VertexFactoryInterpolantsVSToPS vertexFactoryGetInterpolantsVSToPS(VertexFactoryInput input, VertexFactoryIntermediates intermediates, MaterialVertexParameters vertexParameters)
{
	VertexFactoryInterpolantsVSToPS interpolants = (VertexFactoryInterpolantsVSToPS)0;
#if NUM_MATERIAL_TEXCOORDS
	float2 customizedUVs[NUM_MATERIAL_TEXCOORDS];
	getMaterialCustomizedUVs(vertexParameters, customizedUVs);

	UNROLL
	for (int coordinateIndex = 0; coordinateIndex < NUM_MATERIAL_TEXCOORDS; coordinateIndex++)
	{
		setUV(interpolants, coordinateIndex, customizedUVs[coordinateIndex]);
	}

#endif


	interpolants.Color = float4(1.0, 0.0, 0.0, 1.0);
	setTangents(interpolants, intermediates.TangentToWorld[0], intermediates.TangentToWorld[1], intermediates.TangentToWorldSign);
	return interpolants;
}

MaterialPixelParameters getMaterialPixelParameters(VertexFactoryInterpolantsVSToPS interpolants, float4 svPosition)
{
	MaterialPixelParameters result = makeInitializedMaterialPixelParameters();

#if NUM_MATERIAL_TEXCOORDS
	UNROLL
	for (int coordinateIndex = 0; coordinateIndex < NUM_MATERIAL_TEXCOORDS; coordinateIndex++)
	{
		result.TexCoords[coordinateIndex] = getUV(interpolants, coordinateIndex);
	}
#endif

	result.VertexColor = getColor(interpolants);
	half3 tangent = getTangentToWorld0(interpolants);
	half4 normal = getTangentToWorld1(interpolants);
	result.TangentToWorld = assembleTangentToWorld(tangent, normal);
	result.TwoSidedSign = 1;
	return result;
}

MaterialVertexParameters getMaterialVertexParameters(VertexFactoryInput input, VertexFactoryIntermediates intermediates, float3 worldPisition, half3x3 tangentToLocal)
{
	MaterialVertexParameters result = (MaterialVertexParameters)0;
	result.WorldPosition = worldPisition;
	result.VertexColor = intermediates.Color;
	result.TangentToWorld = intermediates.TangentToWorld;

#if NUM_MATERIAL_TEXCOORDS_VERTEX
#if ifNUM_MATERIAL_TEXCOORDS_VERTEX > 1
	UNROLL
	for (int coordinateIndex = 0; coordinateIndex < ifNUM_MATERIAL_TEXCOORDS_VERTEX - 1; coordinateIndex+=2)
	{
		result.TexCoords[coordinateIndex] = input.PackedTexCoords4[coordinateIndex / 2].xy;
		if (coordinateIndex + 1 < NUM_MATERIAL_TEXCOORDS_VERTEX)
		{
			result.TexCoords[coordinateIndex + 1] = input.PackedTexCoords4[coordinateIndex / 2].zw;
		}
	}
#endif
#if NUM_MATERIAL_TEXCOORDS_VERTEX % 2 == 1
	result.TexCoords[NUM_MATERIAL_TEXCOORDS_VERTEX - 1] = input.PackedTexCoords2;
#endif
#endif
	return result;
}

float4 vertexFactoryGetRasterizedWorldPosition(VertexFactoryInput input, VertexFactoryIntermediates intermediates, float4 inWorldPosition)
{
	return inWorldPosition;
}

float3x3 vertexFactoryGetTangentToLocal(VertexFactoryInput input, VertexFactoryIntermediates intermediates)
{
	return intermediates.TangentToLocal;
}