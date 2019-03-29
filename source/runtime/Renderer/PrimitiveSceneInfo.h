#pragma once
#include "CoreMinimal.h"
#include "RenderingThread.h"
#include "PrimitiveSceneProxy.h"
#include "Containers/IndirectArray.h"
#include "GenericOctree.h"
namespace Air
{
	class IndirectLightingCacheAllocation
	{

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
		TindirectArray<class StaticMesh> mStaticMeshes;
		float mLastRenderTime;
		float mLastVisibilityChangeTime;
		bool bNeedsStaticMeshUpdate;
		bool bNeedsConstantBufferUpdate;
		bool bPrecomputeLightingBufferDirty;
		Scene* mScene;
		ConstantBufferRHIRef mIndirectLightingCacheConstantBuffer;
	private:
		friend Scene;
		int32 mPackedIndex;
	public:
		PrimitiveSceneInfo(PrimitiveComponent* inComponent, Scene* inScene);

		virtual void finishCleanup() { delete this; }

		void linkAttachmentGroup();

		void linkLODParentComponent();

		void addToScene(RHICommandListImmediate& RHICmdList, bool bUpdateStaticDrawLists);

		void addStaticMeshes(RHICommandListImmediate& RHICmdList);

		FORCEINLINE void setNeedsConstantBufferUpdate(bool bInNeedsConstantBufferUpdate)
		{
			bNeedsConstantBufferUpdate = bInNeedsConstantBufferUpdate;
		}

		FORCEINLINE bool needsConstantBufferUpdate() const
		{
			return bNeedsConstantBufferUpdate;
		}

		void updateConstantBuffer(RHICommandListImmediate& RHICmdList);

		FORCEINLINE void conditionalUpdateConstantBuffer(RHICommandListImmediate& RHICmdList)
		{
			if (needsConstantBufferUpdate())
			{
				updateConstantBuffer(RHICmdList);
			}
		}

		FORCEINLINE bool needsUpdateStaticMeshes()
		{
			return bNeedsStaticMeshUpdate;
		}

		void updateStaticMeshes(RHICommandListImmediate& RHICmdList);

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

		void removeStaticMeshes();

		void removeFromScene(bool bUpdateStaticDrawList);
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