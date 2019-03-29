#pragma once

#include "CoreMinimal.h"
#include "EngineMininal.h"
#include "ConstantBuffer.h"
#include "ShaderParameters.h"
#include "Math/Matrix.hpp"
namespace Air
{
	BEGIN_CONSTANT_BUFFER_STRUCT(PrimitiveConstantShaderParameters, ENGINE_API)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER(Matrix, LocalToWorld)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER(Matrix, WorldToLocal)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER(float4, ObjectWorldPositionAndRadius)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER(float3, ObjectBounds)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_EX(float, LocalToWorldDeterminantSign, EShaderPrecisionModifier::Half)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER(float3 ,ActorWorldPosition)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER(float, DecalReceiverMask, EShaderPrecisionModifier::Half)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_EX(float4, ObjectOrientation, EShaderPrecisionModifier::Half)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_EX(float4, NonUniformScale, EShaderPrecisionModifier::Half)
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_EX(float4, InvNonUniformScale, EShaderPrecisionModifier::Half)
	END_CONSTANT_BUFFER_STRUCT(PrimitiveConstantShaderParameters)

		inline PrimitiveConstantShaderParameters getPrimitiveConstantShaderParameters(
			const Matrix& localToWorld,
			float3 actorPosition,
			const BoxSphereBounds& worldBounds,
			const BoxSphereBounds& localBounds,
			bool bReceivesDecals
		)
	{
		PrimitiveConstantShaderParameters result;
		result.LocalToWorld = localToWorld;
		result.WorldToLocal = localToWorld.inverse();
		result.ObjectWorldPositionAndRadius = float4(worldBounds.mOrigin, worldBounds.mSphereRadius);
		result.ObjectBounds = worldBounds.mBoxExtent;
		result.LocalToWorldDeterminantSign = Math::floatSelect(localToWorld.rotDeterminant(), 1, -1);
		result.DecalReceiverMask = bReceivesDecals ? 1 : 0;
		result.ObjectOrientation = localToWorld.getUnitAxis(EAxis::Z);
		result.ActorWorldPosition = actorPosition;

		float3 worldX = float3(localToWorld.M[0][0], localToWorld.M[0][1], localToWorld.M[0][2]);
		float3 worldY = float3(localToWorld.M[1][0], localToWorld.M[1][1], localToWorld.M[1][2]);
		float3 worldZ = float3(localToWorld.M[2][0], localToWorld.M[2][1], localToWorld.M[2][2]);
		float scaleX = worldX.size();
		float scaleY = worldY.size();
		float scaleZ = worldZ.size();
		result.NonUniformScale = float4(scaleX, scaleY, scaleZ, 0);
		result.InvNonUniformScale = float4(scaleX > KINDA_SMALL_NUMBER ? 1.0f / scaleX : 0.0f, scaleY > KINDA_SMALL_NUMBER ? 1.0f / scaleY : 0.0f, scaleZ > KINDA_SMALL_NUMBER ? 1.0f / scaleZ : 0.0f, 0.0f);

		return result;
	}

	inline TConstantBufferRef<PrimitiveConstantShaderParameters> createPrimitiveConstantBufferImmediate(
		const Matrix& localToWorld,
		const BoxSphereBounds& worldBounds,
		const BoxSphereBounds& localBounds,
		bool bReceivesDecals,
		bool bUseEditorDepthTest,
		float lpvBiasMultiplier = 1.0f
	)
	{
		BOOST_ASSERT(isInRenderingThread());
		return TConstantBufferRef<PrimitiveConstantShaderParameters>::createConstantBufferImmediate(getPrimitiveConstantShaderParameters(localToWorld, worldBounds.mOrigin, worldBounds, localBounds, bReceivesDecals), ConstantBuffer_MultiFrame);
	}
}