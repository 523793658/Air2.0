#pragma once

#include "CoreMinimal.h"
#include "EngineMininal.h"
#include "ConstantBuffer.h"
#include "ShaderParameters.h"
#include "Math/Matrix.hpp"
namespace Air
{
	BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(PrimitiveConstantShaderParameters, ENGINE_API)
		SHADER_PARAMETER(Matrix, LocalToWorld)
		SHADER_PARAMETER(Matrix, WorldToLocal)
		SHADER_PARAMETER_EX(float, LocalToWorldDeterminantSign, EShaderPrecisionModifier::Half)
		SHADER_PARAMETER(float4, ObjectWorldPositionAndRadius)
		SHADER_PARAMETER(float3, ObjectBounds)
		SHADER_PARAMETER(float3, ActorWorldPosition)
		SHADER_PARAMETER_EX(float, DecalReceiverMask, EShaderPrecisionModifier::Half)
		SHADER_PARAMETER(float4, ObjectOrientation, EShaderPrecisionModifier::Half)
		SHADER_PARAMETER(float4, NonUniformScale, EShaderPrecisionModifier::Half)
		SHADER_PARAMETER(float4, InvNonUniformScale, EShaderPrecisionModifier::Half)
		END_GLOBAL_SHADER_PARAMETER_STRUCT(PrimitiveConstantShaderParameters)

		inline PrimitiveConstantShaderParameters getPrimitiveConstantShaderParameters(
			const Matrix& localToWorld,
			const Matrix& previousLocalToWorld,
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
		return TConstantBufferRef<PrimitiveConstantShaderParameters>::createConstantBufferImmediate(getPrimitiveConstantShaderParameters(localToWorld, localToWorld, worldBounds.mOrigin, worldBounds, localBounds, bReceivesDecals), ConstantBuffer_MultiFrame);
	}

	inline PrimitiveConstantShaderParameters getIdentityPrimitiveParameters()
	{
		return getPrimitiveConstantShaderParameters(
			Matrix(Plane(1, 0, 0, 0), Plane(0, 1, 0, 0), Plane(0, 0, 1, 0), Plane(0, 0, 0, 1)),
			Matrix(Plane(1, 0, 0, 0), Plane(0, 1, 0, 0), Plane(0, 0, 1, 0), Plane(0, 0, 0, 1)),
			float3(0.0f, 0.0f, 0.0f),
			BoxSphereBounds(EForceInit::ForceInit),
			BoxSphereBounds(EForceInit::ForceInit),
			false);
	}

	struct PrimitiveSceneShaderData
	{
		enum {PrimitiveDataStrideInFloat4s = 35};

		float4 mData[PrimitiveDataStrideInFloat4s];

		PrimitiveSceneShaderData()
		{
			setup(getIdentityPrimitiveParameters());
		}

		explicit PrimitiveSceneShaderData(const PrimitiveConstantShaderParameters& primitiveConstantShaderParameters)
		{
			setup(primitiveConstantShaderParameters);
		}


		ENGINE_API void setup(const PrimitiveConstantShaderParameters& primitiveConstantShaderParameters);

		ENGINE_API PrimitiveSceneShaderData(const class PrimitiveSceneProxy* RESTRICT proxy);
	};
}