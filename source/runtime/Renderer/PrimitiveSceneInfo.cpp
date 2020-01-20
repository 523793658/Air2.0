#include "PrimitiveSceneInfo.h"
#include "VertexFactory.h"
#include "MeshBatch.h"
#include "ScenePrivate.h"
namespace Air
{
	class BatchingSPDI : public StaticPrimitiveDrawInterface
	{
	public:
		BatchingSPDI(PrimitiveSceneInfo* inPrimitiveSceneInfo)
			:mPrimitiveSceneInfo(inPrimitiveSceneInfo)
		{}

		virtual void drawMesh(const MeshBatch& mesh, float screenSize)
		{
			if (mesh.getNumPrimitives() > 0)
			{
				BOOST_ASSERT(mesh.mVertexFactory);
				BOOST_ASSERT(mesh.mVertexFactory->isInitialized());
				BOOST_ASSERT(isInRenderingThread());

				PrimitiveSceneProxy* primitiveSceneProxy = mPrimitiveSceneInfo->mProxy;
				primitiveSceneProxy->verifyUsedMaterial(mesh.mMaterialRenderProxy);

				StaticMeshBatch* staticMesh = new (mPrimitiveSceneInfo->mStaticMeshes)StaticMeshBatch(mPrimitiveSceneInfo, mesh, HitProxyId());

				const ERHIFeatureLevel::Type featureLevel = mPrimitiveSceneInfo->mScene->getFeatureLevel();
				staticMesh->preparePrimitiveConstantBuffer(primitiveSceneProxy, featureLevel);

				const bool bSupportsCachingMeshDrawCommands = supportsCachingMeshDrawCommands(primitiveSceneProxy, *staticMesh, featureLevel);

				StaticMeshBatchRelevance* staticMeshRelevance = new (mPrimitiveSceneInfo->mStaticMeshes) StaticMeshBatchRelevance(
					*staticMesh,
					screenSize,
					bSupportsCachingMeshDrawCommands
				);
				
			}
		}
	private:
		PrimitiveSceneInfo* mPrimitiveSceneInfo;
	};

	PrimitiveFlagsCompact::PrimitiveFlagsCompact(const PrimitiveSceneProxy* proxy)
		:bCastDynamicShadow(proxy->castsDynamicShadow())
		,bStaticLighting(proxy->hasStaticLighting())
		,bCastStaticShadow(proxy->castsStaticShadow())
	{}
		
	
	PrimitiveSceneInfo::PrimitiveSceneInfo(PrimitiveComponent* inComponent, Scene* inScene)
		:mProxy(inComponent->mSceneProxy),
		mPrimitiveComponentId(inComponent->mComponentId),
		mComponentLastRenderTime(&inComponent->mLastRenderTime),
		mComponentLastRenderTimeOnScreen(&inComponent->mLastRenderTimeOnScreen),
		mLastRenderTime(-FLT_MAX),
		mLastVisibilityChangeTime(0.0f),
		mScene(inScene),
		bNeedsConstantBufferUpdate(false),
		bPrecomputeLightingBufferDirty(false),
		mNumLightmapDataEntries(0)
	{
		BOOST_ASSERT(mPrimitiveComponentId.isValid());
		BOOST_ASSERT(mProxy);
		PrimitiveComponent* searchParentComponent = dynamic_cast<PrimitiveComponent*>(inComponent->getAttachmentRoot());
		if (searchParentComponent && searchParentComponent != inComponent)
		{
			mLightingAttachmentRoot = searchParentComponent->mComponentId;
		}
	}

	void PrimitiveSceneInfo::linkLODParentComponent()
	{
		if (mLODParentComponentId.isValid())
		{
			
		}
	}

	void PrimitiveSceneInfo::beginDeferredUpdateStaticMeshes()
	{
		if (isIndexValid())
		{
			mScene->mPrimitivesNeedingStaticMeshUpdate[mPackedIndex] = true;
		}
	}


	void PrimitiveSceneInfo::addToScene(RHICommandListImmediate& RHICmdList, bool bUpdateStaticDrawLists, bool bAddToStaticDrawList)
	{
		BOOST_ASSERT(isInRenderingThread());
		if (bUpdateStaticDrawLists)
		{
			addStaticMeshes(RHICmdList, bAddToStaticDrawList);
		}

		PrimitiveBounds &primitiveBounds = mScene->mPrimitiveBounds[mPackedIndex];
		BoxSphereBounds boxSphereBounds = mProxy->getBounds();
		primitiveBounds.mOrigin = boxSphereBounds.mOrigin;
		primitiveBounds.mSphereRadius = boxSphereBounds.mSphereRadius;
		primitiveBounds.mBoxExtent = boxSphereBounds.mBoxExtent;
		primitiveBounds.mMinDrawDistanceSq = Math::square(mProxy->getMinDrawDistance());
		primitiveBounds.mMaxDrawDistance = mProxy->getMaxDrawDistance();

		int32 visibilityBitIndex = mProxy->getVisibilityId();
		PrimitiveVisiblilityId& visibilityId = mScene->mPrimitiveVisibilityIds[mPackedIndex];
		visibilityId.mByteIndex = visibilityBitIndex / 8;
		visibilityId.mBitMask = (1 << (visibilityBitIndex & 0x7));

		uint8 occlusionFlag = EOcclusionFlags::None;
		mScene->mPrimitiveOcclusionFlags[mPackedIndex] = occlusionFlag;

		BoxSphereBounds occlusionBounds = boxSphereBounds;
		if (false)
		{

		}
		mScene->mPrimitiveOcclusionBounds[mPackedIndex] = occlusionBounds;
		mScene->mPrimitiveComponentIds[mPackedIndex] = mPrimitiveComponentId;


	}

	bool PrimitiveSceneInfo::needsUpdateStaticMeshes()
	{
		return mScene->mPrimitivesNeedingStaticMeshUpdate[mPackedIndex];
	}

	void PrimitiveSceneInfo::linkAttachmentGroup()
	{

	}

	void PrimitiveSceneInfo::addStaticMeshes(RHICommandListImmediate& RHICmdList, bool bAddToStaticDrawLists)
	{
		BatchingSPDI batchingSPID(this);
		mProxy->drawStaticElements(&batchingSPID);
		mStaticMeshes.shrink();
		mStaticMeshRelevances.shrink();
		for (int32 meshIndex = 0; meshIndex < mStaticMeshes.size(); meshIndex++)
		{
			StaticMeshBatchRelevance& meshRelevance = mStaticMeshRelevances[meshIndex];
			StaticMeshBatch& mesh = mStaticMeshes[meshIndex];

			SparseArrayAllocationInfo sceneArrayAllocation = mScene->mStaticMeshes.addUninitialized();
			mScene->mStaticMeshes[sceneArrayAllocation.mIndex] = &mesh;
			mesh.mId = sceneArrayAllocation.mIndex;
			meshRelevance.mId = sceneArrayAllocation.mIndex;
			if (mesh.bRequiresPerElementVisibility)
			{
				mesh.mBatchVisibilityId = mScene->mStaticMeshBatchVisibility.addUninitialized().mIndex;
				mScene->mStaticMeshBatchVisibility[mesh.mBatchVisibilityId] = true;
			}
		}
		if (bAddToStaticDrawLists)
		{
			cacheMeshDrawCommand(RHICmdList);
		}
	}

	void PrimitiveSceneInfo::removeFromScene(bool bUpdateStaticDrawList)
	{
		BOOST_ASSERT(isInRenderingThread());

		if (bUpdateStaticDrawList)
		{
			removeStaticMeshes();
		}
	}

	void PrimitiveSceneInfo::removeStaticMeshes()
	{
		mStaticMeshes.empty();
	}

	void PrimitiveSceneInfo::updateStaticMeshes(RHICommandListImmediate& RHICmdList, bool bReAddToDrawLists)
	{
		const bool bNeedsStaticMeshUpdate = !bReAddToDrawLists;
		mScene->mPrimitivesNeedingStaticMeshUpdate[mPackedIndex] = bNeedsStaticMeshUpdate;
		if (!bNeedsStaticMeshUpdate && bNeedsStaticMeshUpdateWithoutVisibilityCheck)
		{
			mScene->mPrimitivesNeedingStaticMeshUpdateWithoutVisibilityCheck.remove(this);
			bNeedsStaticMeshUpdateWithoutVisibilityCheck = false;
		}
		removeCachedDrawMeshCommand();
		if (bReAddToDrawLists)
		{
			cacheMeshDrawCommand(RHICmdList);
		}
	}
	
	void PrimitiveSceneInfo::updateConstantBuffer(RHICommandListImmediate& RHICmdList)
	{
		BOOST_ASSERT(bNeedsConstantBufferUpdate);
		bNeedsConstantBufferUpdate = false;
		mProxy->updateConstantBuffer();
	}

	void PrimitiveSceneInfo::cacheMeshDrawCommand(RHICommandListImmediate& RHICmdList)
	{
		BOOST_ASSERT(mStaticMeshCommandInfos.size() == 0);
		int32 meshWithCachedCommandsNum = 0;
		for (int32 meshIndex = 0; meshIndex < mStaticMeshes.size(); meshIndex++)
		{
			const StaticMeshBatch& mesh = mStaticMeshes[meshIndex];
			if (supportsCachingMeshDrawCommands(mProxy, mesh))
			{
				++meshWithCachedCommandsNum;
			}
		}

		if (meshWithCachedCommandsNum > 0)
		{
			MaterialRenderProxy::updateDeferredCachedConstantExpressions();

			mStaticMeshCommandInfos.reserve(meshWithCachedCommandsNum * 2);

			MemMark mark(MemStack::get());

			const EShadingPath shadingPath = mScene->getShadingPath();

			for (int32 meshIndex = 0; meshIndex < mStaticMeshes.size(); meshIndex++)
			{
				StaticMeshBatchRelevance& meshRelevance = mStaticMeshRelevances[meshIndex];
				StaticMeshBatch& mesh = mStaticMeshes[meshIndex];

				BOOST_ASSERT(meshRelevance.mCommandInfosMask.isEmpty());

				meshRelevance.mCommandInfosBase = mStaticMeshCommandInfos.size();

				if (supportsCachingMeshDrawCommands(mProxy, mesh))
				{
					for (int32 passIndex = 0; passIndex < EMeshPass::Num; passIndex++)
					{
						EMeshPass::Type passType = (EMeshPass::Type)passIndex;

						if ((PassProcessorManager::getPassFlags(shadingPath, passType) & EMeshPassFlags::CachedMeshCommands) != EMeshPassFlags::None)
						{
							CachedMeshDrawCommandInfo commandInfo;
							commandInfo.mMeshPass = passType;

							CachedPassMeshDrawList& sceneDrawList = mScene->mCachedDrawLists[passType];
							CachedPassMeshDrawListContext cachedPassMeshDrawListContext(commandInfo, sceneDrawList, *mScene);

							PassProcessorCreateFunction createFunction = PassProcessorManager::getCreateFunction(shadingPath, passType);
							MeshPassProcessor* passMeshProcessor = createFunction(mScene, nullptr, &cachedPassMeshDrawListContext);

							if (passMeshProcessor != nullptr)
							{
								BOOST_ASSERT(!mesh.bRequiresPerElementVisibility);
								uint64 batchElementMask = ~0ull;
								passMeshProcessor->addMeshBatch(mesh, batchElementMask, mProxy);
								passMeshProcessor->~MeshPassProcessor();
							}

							if (commandInfo.mCommandIndex != -1 || commandInfo.mStateBucketId != -1)
							{
								static_assert(sizeof(meshRelevance.mCommandInfosMask) * 8 >= EMeshPass::Num, "CommandInfosMask is too small to contain all mesh passes.");

								meshRelevance.mCommandInfosMask.set(passType);

								mStaticMeshCommandInfos.add(commandInfo);
							}
						}
					}
				}
			}
		}
	}
}