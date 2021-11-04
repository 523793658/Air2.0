#include "PrimitiveConstantShaderParameters.h"
#include "PrimitiveSceneProxy.h"
namespace Air
{
	void PrimitiveSceneShaderData::setup(const PrimitiveConstantShaderParameters& primitiveConstantShaderParameters)
	{
		static_assert(sizeof(PrimitiveConstantShaderParameters) == sizeof(PrimitiveSceneShaderData), "The FPrimitiveSceneShaderData manual layout below and in usf must match FPrimitiveUniformShaderParameters.  Update this assert when adding a new member.");
		mData[0] = *(const float4*)&primitiveConstantShaderParameters.LocalToWorld.M[0][0];
		mData[1] = *(const float4*)&primitiveConstantShaderParameters.LocalToWorld.M[1][0];
		mData[2] = *(const float4*)&primitiveConstantShaderParameters.LocalToWorld.M[2][0];
		mData[3] = *(const float4*)&primitiveConstantShaderParameters.LocalToWorld.M[3][0];

		mData[4] = *(const float4*)&primitiveConstantShaderParameters.WorldToLocal.M[0][0];
		mData[5] = *(const float4*)&primitiveConstantShaderParameters.WorldToLocal.M[1][0];
		mData[6] = *(const float4*)&primitiveConstantShaderParameters.WorldToLocal.M[2][0];
		mData[7] = *(const float4*)&primitiveConstantShaderParameters.WorldToLocal.M[3][0];
		mData[8] = primitiveConstantShaderParameters.ObjectWorldPositionAndRadius;
		mData[9] = float4(primitiveConstantShaderParameters.ObjectBounds, primitiveConstantShaderParameters.LocalToWorldDeterminantSign);
		mData[10] = float4(primitiveConstantShaderParameters.ActorWorldPosition, primitiveConstantShaderParameters.DecalReceiverMask);
		mData[11] = primitiveConstantShaderParameters.ObjectOrientation;
		mData[12] = primitiveConstantShaderParameters.NonUniformScale;
		mData[13] = primitiveConstantShaderParameters.InvNonUniformScale;
	}


	PrimitiveSceneShaderData::PrimitiveSceneShaderData(const PrimitiveSceneProxy* RESTRICT Proxy)
	{
		bool bHasPrecomputedVolumetricLightmap;
		Matrix PreviousLocalToWorld;
		int32 SingleCaptureIndex;
		bool bOutputVelocity;

		Proxy->GetScene().getPrimitiveConstantShaderParameters_RenderThread(Proxy->getPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex, bOutputVelocity);

		BoxSphereBounds PreSkinnedLocalBounds;
		Proxy->getPreSkinnedLocalBounds(PreSkinnedLocalBounds);

		setup(getPrimitiveConstantShaderParameters(
			Proxy->getLocalToWorld(),
			PreviousLocalToWorld,
			Proxy->getActorPosition(),
			Proxy->getBounds(),
			Proxy->getLocalBounds(),
			Proxy->receivesDecals()
			));
	}


}