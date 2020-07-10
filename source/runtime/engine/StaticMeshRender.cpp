#include "Classes/Components/StaticMeshComponent.h"
#include "StaticMeshResources.h"
#include "MeshBatch.h"
#include "Classes/Materials/MaterialInterface.h"
#include "TessellationRendering.h"

static bool GUseReversedIndexBuffer = false;

static bool GUsePreCulledIndexBuffer = true;

static bool GUseShadowIndexBuffer = false;
namespace Air
{
	size_t StaticMeshSceneProxy::getTypeHash() const
	{
		static size_t uniquePointer;
		return reinterpret_cast<size_t>(&uniquePointer);
	}

	PrimitiveSceneProxy* StaticMeshComponent::createSceneProxy()
	{
		if (getStaticMesh() == nullptr || getStaticMesh()->mRenderData == nullptr)
		{
			return nullptr;
		}

		const TindirectArray<StaticMeshLODResources>& lodResources = getStaticMesh()->mRenderData->mLODResources;
		if (lodResources.size() == 0 || lodResources[Math::clamp<int32>(getStaticMesh()->mMinLOD, 0, lodResources.size() - 1)].mVertexBuffers.mStaticMeshVertexBuffer.getNumVertices() == 0)
		{
			return nullptr;
		}
		PrimitiveSceneProxy* proxy = new StaticMeshSceneProxy(this, false);

		return proxy;
	}

	float StaticMeshSceneProxy::getScreenSize(int32 lodIndex) const
	{
		return mRenderData->mScreenSize[lodIndex];
	}

	void StaticMeshSceneProxy::drawStaticElements(StaticPrimitiveDrawInterface* PDI)
	{
		BOOST_ASSERT(isInParallelRenderingThread());
		if (!hasViewDependentPDG())
		{
			uint8 primitiveDBG = getStaticDepthPriorityGroup();
			int32 numLODs = mRenderData->mLODResources.size();
			bool bIsMeshElementSelected = false;
			const auto featureLevel = getScene().getFeatureLevel();

			if (mForcedLodModel > 0)
			{
				int32 lodIndex = Math::clamp(mForcedLodModel, mClampedMinLOD + 1, numLODs) - 1;
				const StaticMeshLODResources& lodModel = mRenderData->mLODResources[lodIndex];
				for (int32 sectionIndex = 0; sectionIndex < lodModel.mSections.size(); sectionIndex++)
				{
					const int32 numBatches = getNumMeshBatches();

					PDI->reserveMemoryForMeshes(numBatches);

					for (int32 batchIndex = 0; batchIndex < numBatches; batchIndex++)
					{
						MeshBatch meshBatch;
						if (getMeshElement(lodIndex, batchIndex, sectionIndex, primitiveDBG, bIsMeshElementSelected, true, meshBatch))
						{
							PDI->drawMesh(meshBatch, FLT_MAX);
						}
					}
				}
			}
			else
			{
				for (int32 lodIndex = mClampedMinLOD; lodIndex < numLODs; lodIndex++)
				{
					const StaticMeshLODResources& lodModel = mRenderData->mLODResources[lodIndex];
					float sceneSize = getScreenSize(lodIndex);
					bool bUseUnifiedMeshForShadow = false;
					bool bUseUnifiedMeshForDepth = false;
					if (GUseShadowIndexBuffer)
					{

					}

					for (int32 sectionIndex = 0; sectionIndex < lodModel.mSections.size(); sectionIndex++)
					{
						const int32 numBatches = getNumMeshBatches();
						const int32 numRuntimeVirtualTextureTypes = 0;
						PDI->reserveMemoryForMeshes(numBatches);
						for (int32 batchIndex = 0; batchIndex < numBatches; batchIndex++)
						{
							MeshBatch baseMeshBatch;
							if (getMeshElement(lodIndex, batchIndex, sectionIndex, primitiveDBG, bIsMeshElementSelected, true, baseMeshBatch))
							{
								MeshBatch meshBatch(baseMeshBatch);
								meshBatch.bCastShadow &= !bUseUnifiedMeshForShadow;
								meshBatch.bUseAsOccluder &= !bUseUnifiedMeshForDepth;
								meshBatch.bUseForDepthPass &= !bUseUnifiedMeshForDepth;
								PDI->drawMesh(meshBatch, sceneSize);
							}
						}
					}
				}
			}
		}
	}

	bool StaticMeshSceneProxy::getMeshElement(int32 lodIndex, int32 batchIndex, int32 sectionIndex, uint8 inDepthPriorityGroup, bool bUseSelectedMaterial, bool bAllowPreCulledIndices, MeshBatch& outMeshBatch) const
	{
		const ERHIFeatureLevel::Type featureLevel = getScene().getFeatureLevel();
		const StaticMeshLODResources& lod = mRenderData->mLODResources[lodIndex];
		const StaticMeshVertexFactories& vfs = mRenderData->mLODVertexFactories[lodIndex];
		const StaticMeshSection& section = lod.mSections[sectionIndex];
		const LODInfo& proxyLODInfo = mLODs[lodIndex];

		std::shared_ptr<MaterialInterface> materialInterface = proxyLODInfo.mSections[sectionIndex].mMaterial;
		MaterialRenderProxy* materialRenderProxy = materialInterface->getRenderProxy();
		const FMaterial* material = materialRenderProxy->getMaterial(featureLevel);

		const VertexFactory* vertexFactory = nullptr;

		MeshBatchElement& outMeshBatchElement = outMeshBatch.mElements[0];

		if (proxyLODInfo.mOverrideColorVertexBuffer != nullptr)
		{
			BOOST_ASSERT(section.mMaxVertexIndex < proxyLODInfo.mOverrideColorVertexBuffer->getNumVertices());
			vertexFactory = &vfs.mVertexFactoryOverrideColorVertexBuffer;

			outMeshBatchElement.mVertexFactoryUserData = proxyLODInfo.mOverrideColorVertexBuffer;
			outMeshBatchElement.mUserData = proxyLODInfo.mOverrideColorVertexBuffer;
			outMeshBatchElement.bUserDataIsColorVertexBuffer = true;
		}
		else
		{
			vertexFactory = &vfs.mVertexFactory;
			outMeshBatchElement.mVertexFactoryUserData = vfs.mVertexFactory.getConstantBuffer();
		}

		const bool bWireframe = false;
		const bool bRequiresAdjacencyInformation = requiresAdjacencyInformation(materialInterface, vertexFactory->getType(), featureLevel);
		
		const bool bUseReversedIndices = GUseReversedIndexBuffer && isLocalToWorldDeterminantNegative() && (lod.bHasReversedIndices != 0) && !bRequiresAdjacencyInformation && !material->isTwoSided();


		const bool bDitheredLODTransition = false;

		const int32 numPrimitives = setMeshElementGeometrySource(lodIndex, sectionIndex, bWireframe, bRequiresAdjacencyInformation, bUseReversedIndices, bAllowPreCulledIndices, vertexFactory, outMeshBatch);

		if (numPrimitives)
		{
			outMeshBatch.mSegmentIndex = sectionIndex;
			outMeshBatch.mLODIndex = lodIndex;

			outMeshBatch.bReverseCulling = isReversedCullingNeeded(bUseReversedIndices);
			outMeshBatch.bCastShadow = bCastShadow && section.bCastShadow;
#if RHI_RAYTRACING
			outMeshBatch.bCastRayTracedShadow = outMeshBatch.bCastShadow;
#endif
			outMeshBatch.mDepthPriorityGroup = (ESceneDepthPriorityGroup)inDepthPriorityGroup;
			outMeshBatch.LCI = &proxyLODInfo;
			outMeshBatch.mMaterialRenderProxy = materialRenderProxy;
			outMeshBatch.mMinVertexIndex = section.mMinVertexIndex;
			outMeshBatch.mMaxVertexIndex = section.mMaxVertexIndex;

			setMeshElementScreenSize(lodIndex, bDitheredLODTransition, outMeshBatch);

			return true;
		}
		else
		{
			return false;
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

	void StaticMeshSceneProxy::setMeshElementScreenSize(int32 lodIndex, bool bDitheredLODTransition, MeshBatch& outMeshBatch) const
	{
		MeshBatchElement& outBatchElement = outMeshBatch.mElements[0];
		if (mForcedLodModel > 0)
		{
			outMeshBatch.bDitheredLODTransition = false;
			outBatchElement.mMaxScreenSize = 0.0f;
			outBatchElement.mMinScreenSize = -1.0f;
		}
		else
		{
			outMeshBatch.bDitheredLODTransition = bDitheredLODTransition;
			outBatchElement.mMaxScreenSize = getScreenSize(lodIndex);
			outBatchElement.mMinScreenSize = 0.0f;
			if (lodIndex < MAX_STATIC_MESH_LODS - 1)
			{
				outBatchElement.mMinScreenSize = getScreenSize(lodIndex - 1);
			}
		}
	}

	uint32 StaticMeshSceneProxy::setMeshElementGeometrySource(
		int32 lodIndex,
		int32 elementIndex, 
		bool bWireframe, 
		bool bRequiresdAdjacencyInformation,
		bool bUseinversedIndices, 
		bool bAllowPreCulledIndices, 
		const VertexFactory* vertexFactory, 
		MeshBatch& outMeshElement) const
	{
		const StaticMeshLODResources& lodModel = mRenderData->mLODResources[lodIndex];
		const StaticMeshSection& section = lodModel.mSections[elementIndex];
		const LODInfo& lodInfo = mLODs[lodIndex];
		const LODInfo::SectionInfo& sectionInfo = lodInfo.mSections[elementIndex];
		
		MeshBatchElement& outMeshBatchElement = outMeshElement.mElements[0];
		uint32 numPrimitives = 0;

		const bool bHasPreculledTriangles = lodInfo.mSections[elementIndex].mNumPreCulledTriangles >= 0;
		const bool bUsePreculledIndices = bAllowPreCulledIndices && GUsePreCulledIndexBuffer && bHasPreculledTriangles;
		if (bWireframe)
		{
			const bool bSupportsTessellation = RHISupportsTessellation(getScene().getShaderPlatform()) && vertexFactory->getType()->supportsTessellationShaders();
			outMeshElement.mType = PT_TriangleList;

			if (bUsePreculledIndices)
			{
				outMeshBatchElement.mIndexBuffer = lodInfo.mPreCulledIndexBuffer;
				outMeshBatchElement.mFirstIndex = 0;
				numPrimitives = lodInfo.mPreCulledIndexBuffer->getNumIndices() / 3;
			}
			else
			{
				outMeshBatchElement.mFirstIndex = 0;
				outMeshBatchElement.mIndexBuffer = &lodModel.mIndexBuffer;
				numPrimitives = lodModel.mIndexBuffer.getNumIndices() / 3;
			}
			outMeshElement.bWireframe = true;
			outMeshElement.bDisableBackfaceCulling = true;
		}
		else
		{
			outMeshElement.mType = PT_TriangleList;
			if (bUsePreculledIndices)
			{
				outMeshBatchElement.mIndexBuffer = lodInfo.mPreCulledIndexBuffer;
				outMeshBatchElement.mFirstIndex = sectionInfo.mFirstPreCulledIndex;
				numPrimitives = sectionInfo.mNumPreCulledTriangles;
			}
			else
			{
				outMeshBatchElement.mIndexBuffer = &lodModel.mIndexBuffer;
				outMeshBatchElement.mFirstIndex = section.mFirstIndex;
				numPrimitives = section.mNumTriangles;
			}
		}

		if (bRequiresdAdjacencyInformation)
		{
			BOOST_ASSERT(lodModel.bHasAdjacencyInfo);
			BOOST_ASSERT(lodModel.mAdditionalIndexBuffer);
			outMeshBatchElement.mIndexBuffer = &lodModel.mAdditionalIndexBuffer->mAdjacencyIndexBuffer;
			outMeshElement.mType = PT_12_ControlPointPatchList;
			outMeshBatchElement.mFirstIndex = 4;
		}

		outMeshBatchElement.mNumPrimitives = numPrimitives;
		outMeshElement.mVertexFactory = vertexFactory;
		return numPrimitives;
	}
}