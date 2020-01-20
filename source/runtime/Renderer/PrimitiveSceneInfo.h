#pragma once
#include "CoreMinimal.h"
#include "RenderingThread.h"
#include "PrimitiveSceneProxy.h"
#include "Containers/IndirectArray.h"
#include "GenericOctree.h"
#include "MeshPassProcessor.h"
#include "SceneCore.h"
namespace Air
{
	class IndirectLightingCacheAllocation
	{

	};

	struct PrimitiveFlagsCompact
	{
		uint8 bCastDynamicShadow : 1;
		uint8 bStaticLighting : 1;
		uint8 bCastStaticShadow : 1;
		PrimitiveFlagsCompact(const PrimitiveSceneProxy* proxy);
	};

	class PrimitiveSceneInfo : public DeferredCleanupInterface
	{
	public:
		PrimitiveSceneProxy* mProxy;
		PrimitiveComponentId mPrimitiveComponentId;
		float* mComponentLastRenderTime;
		float* mComponentLastRenderTimeOnScreen;

		PrimitiveComponentId mLightingAttachmentRoot;

		PrimitiveComponentId mLODParentComponentId;
		TArray<class StaticMeshBatch> mStaticMeshes;
		TArray<class StaticMeshBatchRelevance> mStaticMeshRelevances;
		TArray<class CachedMeshDrawCommandInfo> mStaticMeshCommandInfos;
		float mLastRenderTime;
		float mLastVisibilityChangeTime;
		uint8 bNeedsConstantBufferUpdate : 1;
		uint8 bPrecomputeLightingBufferDirty : 1;
		uint8 bNeedsStaticMeshUpdateWithoutVisibilityCheck : 1;
		Scene* mScene;
		ConstantBufferRHIRef mIndirectLightingCacheConstantBuffer;


	private:
		friend Scene;
		int32 mPackedIndex;

		int32 mNumLightmapDataEntries;
	public:
		PrimitiveSceneInfo(PrimitiveComponent* inComponent, Scene* inScene);

		virtual void finishCleanup() { delete this; }

		void linkAttachmentGroup();

		void linkLODParentComponent();

		int32 getNumLightmapDataEntries() const { return mNumLightmapDataEntries; }

		void addToScene(RHICommandListImmediate& RHICmdList, bool bUpdateStaticDrawLists, bool bAddToStaticDrawList = true);

		void addStaticMeshes(RHICommandListImmediate& RHICmdList, bool bAddToStaticDrawLists);

		FORCEINLINE void setNeedsConstantBufferUpdate(bool bInNeedsConstantBufferUpdate)
		{
			bNeedsConstantBufferUpdate = bInNeedsConstantBufferUpdate;
		}

		FORCEINLINE bool needsConstantBufferUpdate() const
		{
			return bNeedsConstantBufferUpdate;
		}

		FORCEINLINE bool isIndexValid() const { return mPackedIndex != INDEX_NONE && mPackedIndex != std::numeric_limits<int32>::max(); }

		void updateConstantBuffer(RHICommandListImmediate& RHICmdList);

		FORCEINLINE void conditionalUpdateConstantBuffer(RHICommandListImmediate& RHICmdList)
		{
			if (needsConstantBufferUpdate())
			{
				updateConstantBuffer(RHICmdList);
			}
		}

	
		void updateStaticMeshes(RHICommandListImmediate& RHICmdList, bool bReAddToDrawLists = true);

		FORCEINLINE void conditionalUpdateStaticMeshes(RHICommandListImmediate& RHICmdList)
		{
			if (needsUpdateStaticMeshes())
			{
				updateStaticMeshes(RHICmdList);
			}
		}

		FORCEINLINE void conditionalLazyUpdateForRendering(RHICommandListImmediate& RHICmdList)
		{
			conditionalUpdateConstantBuffer(RHICmdList);
			conditionalUpdateStaticMeshes(RHICmdList);
		}

		bool needsUpdateStaticMeshes();

		void removeStaticMeshes();

		void removeFromScene(bool bUpdateStaticDrawList);

		void beginDeferredUpdateStaticMeshes();

	private:
		void cacheMeshDrawCommand(RHICommandListImmediate& RHICmdList);

		void removeCachedDrawMeshCommand();
	};

	class PrimitiveSceneInfoCompact
	{
	public:
		PrimitiveSceneInfo * mPrimitiveSceneInfo;
		PrimitiveSceneProxy* mProxy;
		BoxSphereBounds	mBounds;
		float mMinDrawDistance;
		float mMaxDrawDistance;
		float mLPVBiasMuliplier;

		int32 mVisibilityId;
		uint32 bHasViewDependentDPG : 1;
		uint32 bCastDynamicShadow : 1;
		uint32 bAffectDynamicIndirectLighting : 1;
		uint32 mStaitcDepthPriorityGroup : SDPG_NumBits;

		void init(PrimitiveSceneInfo* inPrimitiveSceneInfo);

		PrimitiveSceneInfoCompact()
			:mPrimitiveSceneInfo(nullptr)
			, mProxy(nullptr)
		{

		}
		PrimitiveSceneInfoCompact(PrimitiveSceneInfo* inPrimitiveSceneInfo)
		{
			init(inPrimitiveSceneInfo);
		}

	};

	struct PrimitiveOctreeSemantics
	{
		enum { MaxElementPerLeaf = 16 };
		enum { MinInclusiveElementsPerNode = 7 };
		enum { MaxNodeDepth = 12 };

		typedef TInlineAllocator<MaxElementPerLeaf> ElementAllocator;

		FORCEINLINE static const BoxSphereBounds& getBoundingBox(const PrimitiveSceneInfoCompact& primitiveSceneInfoCompact)
		{
			return primitiveSceneInfoCompact.mBounds;
		}
	};

	typedef TOctree<PrimitiveSceneInfoCompact, struct PrimitiveOctreeSemantics> ScenePrimitiveOctree;
}