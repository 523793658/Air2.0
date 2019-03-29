#pragma once
#include "EngineMininal.h"
#include "PackedNormal.h"
#include "RenderUtils.h"
#include "Math/Color.h"
namespace Air
{
	struct DynamicMeshVertex
	{
		DynamicMeshVertex() {}
		DynamicMeshVertex(const float3& inPosition) :
			mPosition(inPosition),
			mTextureCoordinate(float2::Zero),
			mNormal(float3(1, 0, 0)),
			mTangent(float3(0, 0, 1)),
			mColor(Color(255, 255, 255))
		{
			mTangent.Vector.w = 255;
		}



		DynamicMeshVertex(const float3& inPosition, const float3& inNormal, const float3& inTangent, const float2& inTexCoord, const Color& inColor)
			:mPosition(inPosition)
			,mNormal(inNormal)
			,mTangent(inTangent)
			,mColor(inColor)
			,mTextureCoordinate(inTexCoord)
		{
			mTangent.Vector.w = 255;
		}

		void setTangents(const float3& inNormal, const float3& inTangentY, const float3& inTangent)
		{
			mNormal = inNormal;
			mTangent = inTangent;
			mTangent.Vector.w = getBasisDeterminantSign(inNormal, inTangentY, inTangent) < 0.0f ? 0 : 255;
		}

		float3 getTangentY()
		{
			float3 tanX = mNormal;
			float3 tanZ = mTangent;
			return (tanZ ^ tanX) * ((float)mTangent.Vector.w / 127.5f - 1.0f);
		}

		float3 mPosition;
		float2 mTextureCoordinate;
		PackedNormal mNormal;
		PackedNormal mTangent;
		Color mColor;
	};
}