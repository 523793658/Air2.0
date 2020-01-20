#pragma once
#include "CoreMinimal.h"
#include "MeshBatch.h"
#include "MeshPassProcessor.h"
namespace Air
{
	class PrimitiveSceneInfo;

	class StaticMeshBatch : public MeshBatch
	{
	public:
		PrimitiveSceneInfo* mPrimitiveSceneInfo;

		int32 mId;

		int32 mBatchVisibilityId;

		StaticMeshBatch(
			PrimitiveSceneInfo* inPrimitiveSceneInfo,
			const MeshBatch& inMesh,
			HitProxyId inHitProxyId
		)
			:MeshBatch(inMesh)
			, mPrimitiveSceneInfo(inPrimitiveSceneInfo)
			, mId(INDEX_NONE)
			, mBatchVisibilityId(INDEX_NONE)
		{
			mBatchHitPrixyId = inHitProxyId;
		}

		~StaticMeshBatch();

		StaticMeshBatch(const StaticMeshBatch& inStaticMesh)
			:MeshBatch(inStaticMesh)
			,mPrimitiveSceneInfo(inStaticMesh.mPrimitiveSceneInfo)
			,mId(inStaticMesh.mId)
			,mBatchVisibilityId(inStaticMesh.mBatchVisibilityId)
		{}
	};

	class StaticMeshBatchRelevance
	{
	public:
		StaticMeshBatchRelevance(const StaticMeshBatch& staticMesh, float inScreenSize, bool inbSupportsCachingMeshDrawCommands)
			:mId(staticMesh.mId)
			,mScreenSize(inScreenSize)
			,mCommandInfosBase(0)
			,mLODIndex(staticMesh.mLODIndex)
			,mNumElements(staticMesh.mElements.size())
			,bDitheredLODTransition(staticMesh.bDitheredLODTransition)
			,bRequiresPerElementVisibility(staticMesh.bRequiresPerElementVisibility)
			,bSelectable(staticMesh.bSelectable)
			,bCastShadow(staticMesh.bCastShadow)
			,bUseForMaterial(staticMesh.bUseForMaterial)
			,bUseForDepthPass(staticMesh.bUseForDepthPass)
			,bUseAsOccluder(staticMesh.bUseAsOccluder)
			, bSupportsCachingMeshDrawCommands(inbSupportsCachingMeshDrawCommands)
		{}

		int32 mId;

		float mScreenSize;

		MeshPassMask mCommandInfosMask;

		uint16 mCommandInfosBase;

		int8 mLODIndex;

		uint16 mNumElements;

		uint8 bDitheredLODTransition : 1;

		uint8 bRequiresPerElementVisibility : 1;

		uint8 bSelectable : 1;

		uint8 bCastShadow : 1;

		uint8 bUseForMaterial : 1;

		uint8 bUseForDepthPass : 1;

		uint8 bUseAsOccluder : 1;

		uint8 bSupportsCachingMeshDrawCommands : 1;

		int32 getStaticMeshCommandInfoIndex(EMeshPass::Type meshPass) const;

	};
}