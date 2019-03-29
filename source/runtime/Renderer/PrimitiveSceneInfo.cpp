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

				mesh.checkConstantBuffers();
				mPrimitiveSceneInfo->mProxy->verifyUsedMaterial(mesh.mMaterialRenderProxy);
				StaticMesh* staticMesh = new(mPrimitiveSceneInfo->mStaticMeshes)StaticMesh(mPrimitiveSceneInfo, mesh, screenSize, HitProxyId());
			}
		}
	private:
		PrimitiveSceneInfo* mPrimitiveSceneInfo;
	};


	PrimitiveSceneInfo::PrimitiveSceneInfo(PrimitiveComponent* inComponent, Scene* inScene)
		:mProxy(inComponent->mSceneProxy),
		mPrimitiveComponentId(inComponent->mComponentId),
		mComponentLastRenderTime(&inComponent->mLastRenderTime),
		mComponentLastRenderTimeOnScreen(&inComponent->mLastRenderTimeOnScreen),
		mLastRenderTime(-FLT_MAX),
		mLastVisibilityChangeTime(0.0f),
		mScene(inScene),
		bNeedsStaticMeshUpdate(false),
		bNeedsConstantBufferUpdate(false),
		bPrecomputeLightingBufferDirty(false)
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

	void PrimitiveSceneInfo::addToScene(RHICommandListImmediate& RHICmdList, bool bUpdateStaticDrawLists)
	{
		BOOST_ASSERT(isInRenderingThread());
		if (bUpdateStaticDrawLists)
		{
			addStaticMeshes(RHICmdList);
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

	void PrimitiveSceneInfo::linkAttachmentGroup()
	{

	}

	void PrimitiveSceneInfo::addStaticMeshes(RHICommandListImmediate& RHICmdList)
	{
		BatchingSPDI batchingSPID(this);
		mProxy->drawStaticElements(&batchingSPID);
		mStaticMeshes.shrink();
		for (int32 meshIndex = 0; meshIndex < mStaticMeshes.size(); meshIndex++)
		{
			StaticMesh& mesh = mStaticMeshes[meshIndex];
			SparseArrayAllocationInfo sceneArrayAllocation = mScene->mStaticMeshes.addUninitialized();
			mScene->mStaticMeshes[sceneArrayAllocation.mIndex] = &mesh;
			mesh.mId = sceneArrayAllocation.mIndex;
			mesh.addToDrawList(RHICmdList, mScene);
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

	void PrimitiveSceneInfo::updateStaticMeshes(RHICommandListImmediate& RHICmdList)
	{
		BOOST_ASSERT(bNeedsStaticMeshUpdate);
		bNeedsStaticMeshUpdate = false;
		for (int32 meshIndex = 0; meshIndex < mStaticMeshes.size(); meshIndex++)
		{
			mStaticMeshes[meshIndex].removeFromDrawList();
			mStaticMeshes[meshIndex].addToDrawList(RHICmdList, mScene);
		}
	}
	
	void PrimitiveSceneInfo::updateConstantBuffer(RHICommandListImmediate& RHICmdList)
	{
		BOOST_ASSERT(bNeedsConstantBufferUpdate);
		bNeedsConstantBufferUpdate = false;
		mProxy->updateConstantBuffer();
	}
}