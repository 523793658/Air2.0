#include "Classes/Components/StaticMeshComponent.h"
#include "StaticMeshResources.h"
#include "MeshBatch.h"
#include "Classes/Materials/MaterialInterface.h"

static bool GUseReversedIndexBuffer = false;

static bool GUsePreCulledIndexBuffer = true;
namespace Air
{
	PrimitiveSceneProxy* StaticMeshComponent::createSceneProxy()
	{
		if (getStaticMesh() == nullptr || getStaticMesh()->mRenderData == nullptr || getStaticMesh()->mRenderData->mLODResources.size() == 0 || getStaticMesh()->mRenderData->mLODResources[0].mVertexBuffer.getNumVertices() == 0)
		{
			return nullptr;
		}

		PrimitiveSceneProxy* proxy = new StaticMeshSceneProxy(this, getStaticMesh()->bLODsShareStaticLighting);

		return proxy;
	}

	float StaticMeshSceneProxy::getScreenSize(int32 lodIndex) const
	{
		return mRenderData->mScreenSize[lodIndex];
	}

	void StaticMeshSceneProxy::drawStaticElements(StaticPrimitiveDrawInterface* PDI)
	{
		BOOST_ASSERT(isInParallelRenderingThread());
		int32 numLODs = mRenderData->mLODResources.size();
		bool bUseSelectedMaterial = false;
		const bool bUseHoveredMaterial = false;
		const auto featureLevel = getScene().getFeatureLevel();

		if (mForcedLodModel > 0)
		{

		}
		else
		{
			for (int32 lodIndex = 0; lodIndex < numLODs; lodIndex++)
			{
				const StaticMeshLODResources& lodModel = mRenderData->mLODResources[lodIndex];
				float screenSize = getScreenSize(lodIndex);
				bool bUseUnifiedMeshForShadow = false;
				bool bUseUnifiedMeshForDepth = false;
				for (int32 sectionIndex = 0; sectionIndex < lodModel.mSections.size(); sectionIndex++)
				{
					const int32 numBatches = getNumMeshBatches();

					for (int32 batchIndex = 0; batchIndex < numBatches; batchIndex++)
					{
						MeshBatch meshBatch;
						if (getMeshElement(lodIndex, batchIndex, sectionIndex, 0, bUseSelectedMaterial, bUseHoveredMaterial, true, meshBatch))
						{
							meshBatch.bCastShadow &= !bUseUnifiedMeshForShadow;
							meshBatch.bUseAsOccluder &= !bUseUnifiedMeshForDepth;
							PDI->drawMesh(meshBatch, screenSize);
						}
					}
				}
			}
		}
	}

	bool StaticMeshSceneProxy::getMeshElement(int32 lodIndex, int32 batchIndex, int32 sectionIndex, uint8 inDepthPriorityGroup, bool bUseSelectedMaterial, bool bUseHoveredMaterial, bool bAllowPreCulledIndices, MeshBatch& outMeshBatch) const
	{
		const StaticMeshLODResources& lod = mRenderData->mLODResources[lodIndex];
		const StaticMeshSection& section = lod.mSections[sectionIndex];

		const LODInfo& proxyLODInfo = mLODs[lodIndex];

		MaterialInterface* material = proxyLODInfo.mSections[sectionIndex].mMaterial;
		outMeshBatch.mMaterialRenderProxy = material->getRenderProxy(bUseSelectedMaterial, bUseHoveredMaterial);
		outMeshBatch.mVertexFactory = &lod.mVertexFactory;

		const bool bWireframe = false;

		const bool bRequiresAdjacencyInformation = false;

		const bool bUseReversedIndices = !bWireframe && GUseReversedIndexBuffer && isLocalToWorldDeterminantNegative() && lod.bHasReversedIndices && !bRequiresAdjacencyInformation && !material->isTwoSided();
		setIndexSource(lodIndex, sectionIndex, outMeshBatch, bWireframe, bRequiresAdjacencyInformation, bUseReversedIndices, bAllowPreCulledIndices);
		MeshBatchElement& outBatchElement = outMeshBatch.mElements[0];
		if (proxyLODInfo.mOverrideColorVertexBuffer != nullptr)
		{
			BOOST_ASSERT(section.mMaxVertexIndex < proxyLODInfo.mOverrideColorVertexBuffer->getNumVertices());
			outMeshBatch.mVertexFactory = &lod.mVertexFactoryOverrideColorVertexBuffer;
			outBatchElement.mUserData = proxyLODInfo.mOverrideColorVertexBuffer;
			outBatchElement.bUserDataIsColorVertexBuffer = true;
		}

		if (outBatchElement.mNumPrimitives > 0)
		{
			outMeshBatch.mDynamicVertexData = nullptr;
			outMeshBatch.LCI = &proxyLODInfo;
			outBatchElement.mPrimitiveConstantBufferResource = &getConstantBuffer();
			outBatchElement.mMinVertexIndex = section.mMinVertexIndex;
			outBatchElement.mMaxVertexIndex = section.mMaxVertexIndex;

			outMeshBatch.mLODIndex = lodIndex;

			outMeshBatch.bUseDynamicData = false;
			outMeshBatch.bReverseCulling = isLocalToWorldDeterminantNegative() && !bUseReversedIndices;
			outMeshBatch.bCastShadow = bCastShadow && section.bCastShadow;

			outMeshBatch.mDepthPriorityGroup = (ESceneDepthPriorityGroup)inDepthPriorityGroup;
			if (mForcedLodModel > 0)
			{
				outBatchElement.mMaxScreenSize = 0.0f;
				outBatchElement.mMinScreenSize = -1.0f;
			}
			else
			{
				outMeshBatch.bDitheredLODTransition = !isMovable() && material->isDitheredLODTransition();

				outBatchElement.mMaxScreenSize = getScreenSize(lodIndex);
				outBatchElement.mMinScreenSize = 0.0f;
				if (lodIndex < MAX_STATIC_MESH_LODS - 1)
				{
					outBatchElement.mMinScreenSize = getScreenSize(lodIndex + 1);
				}
			}
			return true;
		}
		else
		{
			return false;
		}
	}

	void StaticMeshSceneProxy::setIndexSource(int32 lodIndex, int32 elementIndex, MeshBatch& outMeshElement, bool bWireframe, bool bRequiresAdjacencyInformation, bool bUseInversedIndices, bool bAllowPreCulledIndices) const
	{
		MeshBatchElement& outElement = outMeshElement.mElements[0];
		const StaticMeshLODResources& lodModel = mRenderData->mLODResources[lodIndex];
		if (bWireframe)
		{
			
		}
		else
		{
			const StaticMeshSection& section = lodModel.mSections[elementIndex];
			outMeshElement.mType = PT_TriangleList;
			if (bAllowPreCulledIndices
				&& GUsePreCulledIndexBuffer
				&& mLODs[lodIndex].mSections[elementIndex].mNumPreCulledTriangles >= 0)
			{
				outElement.mIndexBuffer = mLODs[lodIndex].mPreCulledIndexBuffer;
				outElement.mFirstIndex = mLODs[lodIndex].mSections[elementIndex].mFirstPreCulledIndex;
				outElement.mNumPrimitives = mLODs[lodIndex].mSections[elementIndex].mNumPreCulledTriangles;
			}
			else
			{
				outElement.mIndexBuffer = bUseInversedIndices ? &lodModel.mReversedIndexBuffer : &lodModel.mIndexBuffer;
				outElement.mFirstIndex = section.mFirstIndex;
				outElement.mNumPrimitives = section.mNumTriangles;
			}
		}
		if (bRequiresAdjacencyInformation)
		{
			BOOST_ASSERT(false);
		}
	}

	PrimitiveViewRelevance StaticMeshSceneProxy::getViewRelevance(const SceneView* view) const
	{
		BOOST_ASSERT(isInParallelRenderingThread());
		PrimitiveViewRelevance result;
		result.bDrawRelevance = isShown(view) && view->mFamily->mEngineShowFlags.StaticMeshes;
		result.bRenderInMainPass = shouldRenderInMainPass();
		if (!isStaticPathAvailable())
		{
			result.bDynamicRelevance = true;
		}
		else
		{
			result.bStaticRelevance = true;
		}
		mMaterialRelevance.setPrimitiveViewRelevance(result);

		return result;
	}

}