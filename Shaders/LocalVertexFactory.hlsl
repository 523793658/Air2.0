#include "VertexFactoryCommon.hlsl"
#include "LocalVertexFactoryCommon.hlsl"

struct VertexFactoryInput
{
	float4 Position : ATTRIBUTE0;
	float3 Normal	: ATTRIBUTE1;
	float4 Tangent	: ATTRIBUTE2;
	float4 Color	: ATTRIBUTE3;
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
	half3 normal = tangentBias(input.Normal.xyz);
	half4 tangent = tangentBias(input.Tangent);
	half3 bTangent = cross(tangent.xyz, normal) * tangent.w;
	result[0] = cross(bTangent, tangent.xyz) * tangent.w;
	result[1] = bTangent;
	result[2] = tangent.xyz;
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
	intermediates.TangentToWorldSign = tangentBias(input.Tangent.w) * Primitive.LocalToWorldDeterminantSign;
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
	interpolants.Color = float4(1.0, 0.0, 0.0, 1.0);
	setTangents(interpolants, intermediates.TangentToWorld[0], intermediates.TangentToWorld[2], intermediates.TangentToWorldSign);
	return interpolants;
}

MaterialPixelParameters getMaterialPixelParameters(VertexFactoryInterpolantsVSToPS interpolants, float4 svPosition)
{
	MaterialPixelParameters result = makeInitializedMaterialPixelParameters();
	result.VertexColor = getColor(interpolants);
	half3 normal = getTangentToWorld0(interpolants).xyz;
	half4 tangent = getTangentToWorld2(interpolants);
	result.TangentToWorld = assembleTangentToWorld(normal, tangent);
	result.TwoSidedSign = 1;
	return result;
}

MaterialVertexParameters getMaterialVertexParameters(VertexFactoryInput input, VertexFactoryIntermediates intermediates, float3 worldPisition, half3x3 tangentToLocal)
{
	MaterialVertexParameters result = (MaterialVertexParameters)0;
	result.WorldPosition = worldPisition;
	result.VertexColor = intermediates.Color;
	result.TangentToWorld = intermediates.TangentToWorld;
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