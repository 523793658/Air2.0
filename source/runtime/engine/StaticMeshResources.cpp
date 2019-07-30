#include "StaticMeshResources.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "StaticMeshVertexData.h"
#include "MeshUtilities/MeshUtilities.h"
#include "Modules/ModuleManager.h"
#include "Math/Color.h"
#include "Serialization/MemoryWriter.h"
#include "Classes/Materials/Material.h"
namespace Air
{

	bool GForceDefaultMaterial = false;

	Archive& operator<<(Archive& ar, StaticMeshSection& section)
	{
		ar << section.mMaterialIndex;
		ar << section.mFirstIndex;
		ar << section.mNumTriangles;
		ar << section.mMinVertexIndex;
		ar << section.mMaxVertexIndex;
		ar << section.bEnableCollision;
		ar << section.bCastShadow;
		return ar;
	}

#if WITH_EDITOR
	static float calculateViewDistance(float maxDeviation, float allowedPixelError)
	{
		const float viewDistance = (maxDeviation* 960.0f) / Math::max(allowedPixelError, RStaticMesh::mMinimumAutoLODPixelError);
		return viewDistance;
	}
#endif



	StaticMeshSceneProxy::StaticMeshSceneProxy(StaticMeshComponent* component, bool bCanLODsShareStaticLighting)
		:PrimitiveSceneProxy(component, component->getStaticMesh()->getName())
		,mOwner(component->getOwner())
		,mStaticMesh(component->getStaticMesh())
		,mRenderData(component->getStaticMesh()->mRenderData.get())
		,bCastShadow(component->bCastShadow)
	{
		bool bAnySectionCastShadows = false;
		mLODs.empty(mRenderData->mLODResources.size());

		const auto featureLevel = getScene().getFeatureLevel();


		for (int32 lodIndex = 0; lodIndex < mRenderData->mLODResources.size(); lodIndex++)
		{
			LODInfo* newLodInfo = new (mLODs)LODInfo(component, lodIndex, bCanLODsShareStaticLighting);
			const int32 numSections = newLodInfo->mSections.size();
			for (int32 sectionIndex = 0; sectionIndex < numSections; ++sectionIndex)
			{
				const LODInfo::SectionInfo& sectionInfo = newLodInfo->mSections[sectionIndex];
				bAnySectionCastShadows |= mRenderData->mLODResources[lodIndex].mSections[sectionIndex].bCastShadow;
				if (sectionInfo.mMaterial == RMaterial::getDefaultMaterial(MD_Surface))
				{
					mMaterialRelevance |= RMaterial::getDefaultMaterial(MD_Surface)->getRelevance(featureLevel);
				}
			}
		}


	

	}



	StaticMeshSceneProxy::LODInfo::LODInfo(const StaticMeshComponent* inComponent, int32 lodIndex, bool bCanLODsShareStaticLighting)
		:LightCacheInterface(nullptr, nullptr)
		,mOverrideColorVertexBuffer(nullptr)
		,mPreCulledIndexBuffer(nullptr)
		,bUsesMeshModifyingMaterials(false)
	{
		const auto featureLevel = inComponent->getWorld()->mFeatureLevel;
		StaticMeshRenderData* meshRenderData = inComponent->getStaticMesh()->mRenderData.get();
		StaticMeshLODResources& lodModel = meshRenderData->mLODResources[lodIndex];

		mSections.empty(meshRenderData->mLODResources[lodIndex].mSections.size());
		for (int32 sectionIndex = 0; sectionIndex < lodModel.mSections.size(); sectionIndex++)
		{
			const StaticMeshSection& section = lodModel.mSections[sectionIndex];
			SectionInfo sectionInfo;
			sectionInfo.mMaterial = inComponent->getMaterial(section.mMaterialIndex);
#if WITH_EDITORONLY_DATA
			sectionInfo.mMaterialIndex = section.mMaterialIndex;
#endif
			if (GForceDefaultMaterial && sectionInfo.mMaterial && !isTranslucentBlendMode(sectionInfo.mMaterial->getBlendMode()))
			{
				sectionInfo.mMaterial = RMaterial::getDefaultMaterial(MD_Surface);
			}

			if (!sectionInfo.mMaterial)
			{
				sectionInfo.mMaterial = RMaterial::getDefaultMaterial(MD_Surface);
			}

			const bool bRequiresAdjacencyInformation = false;

			mSections.add(sectionInfo);

		}
	}


	StaticMeshLODResources::StaticMeshLODResources()
		:bHasAdjacencyInfo(false)
		,bHasDepthOnlyIndices(false)
		,bHasReversedIndices(false)
		,bHasreversedDepthOnlyIndices(false)
		,mDepthOnlyNumTriangles(0)
	{

	}



	StaticMeshLODResources::~StaticMeshLODResources()
	{
		
	}

	void StaticMeshLODResources::serialize(Archive& ar, Object* owner, int32 idx)
	{
		RStaticMesh* ownerStaticMesh = check_cast<RStaticMesh*>(owner);
		bool bMeshCPUAccess = ownerStaticMesh ? ownerStaticMesh->bAllowCPUAccess : false;
		bool bNeedsCPUAccess = bMeshCPUAccess;

		bHasAdjacencyInfo = false;
		bHasDepthOnlyIndices = false;
		bHasReversedIndices = false;
		bHasreversedDepthOnlyIndices = false;
		mDepthOnlyNumTriangles = 0;
		
		const uint8 adjacencyDataStripFlag = 1;
		mPositionVertexBuffer.serialize(ar, bNeedsCPUAccess);

		mVertexBuffer.serialize(ar, bNeedsCPUAccess);
		mIndexBuffer.serialize(ar, bNeedsCPUAccess);
		mColorVertexBuffer.serialize(ar, bNeedsCPUAccess);
		mReversedIndexBuffer.serialize(ar, bNeedsCPUAccess);
		mDepthOnlyIndexBuffer.serialize(ar, bNeedsCPUAccess);
		mReversedDepthOnlyIndexBuffer.serialize(ar, bNeedsCPUAccess);

		bHasDepthOnlyIndices = mDepthOnlyIndexBuffer.getNumIndices() != 0;
		bHasReversedIndices = mReversedIndexBuffer.getNumIndices() != 0;
		bHasreversedDepthOnlyIndices = mReversedDepthOnlyIndexBuffer.getNumIndices() != 0;
		mDepthOnlyNumTriangles = mDepthOnlyIndexBuffer.getNumIndices() / 3;
	}

	void StaticMeshLODResources::initVertexFactory(LocalVertexFactory& inOutVertexFactory, RStaticMesh* inParentMesh, bool bInOverrideColorVertexBuffer)
	{
		BOOST_ASSERT(inParentMesh != nullptr);
		struct InitStaticMeshVertexFactoryParams
		{
			LocalVertexFactory* mVertexFactory;
			StaticMeshLODResources* mLODResource;
			bool bOverrideColorVertexBuffer;
			RStaticMesh* mParent;
		}params;
		params.mVertexFactory = &inOutVertexFactory;
		params.mLODResource = this;
		params.bOverrideColorVertexBuffer = bInOverrideColorVertexBuffer;
		params.mParent = inParentMesh;

		uint32 tangentXOffset = 0;
		uint32 tangentYOffset = 0;
		uint32 uvBaseOffset = 0;
		SELECT_STATIC_MESH_VERTEX_TYPE(
			params.mLODResource->mVertexBuffer.getUseHighPrecisionTangentBasis(),
			params.mLODResource->mVertexBuffer.getUseFullPrecisionUVs(),
			params.mLODResource->mVertexBuffer.getNumTexCoords(),
			{
				tangentXOffset = STRUCT_OFFSET(VertexType, mTangentX);
				tangentYOffset = STRUCT_OFFSET(VertexType, mTangentY);
				uvBaseOffset = STRUCT_OFFSET(VertexType, mUVs);
			});

		ENQUEUE_UNIQUE_RENDER_COMMAND_FOURPARAMETER(
			InitStaticMeshVertexFactory,
			InitStaticMeshVertexFactoryParams, params, params,
			uint32, tangentXOffset, tangentXOffset,
			uint32, tangentYOffset, tangentYOffset,
			uint32, uvBaseOffset, uvBaseOffset,
			{
				LocalVertexFactory::DataType data;
				data.mPositionComponents = VertexStreamComponent(
			&params.mLODResource->mPositionVertexBuffer,
			STRUCT_OFFSET(PositionVertex, mPosition),
			params.mLODResource->mPositionVertexBuffer.getStride(),
			VET_Float3);
				data.mTangentBasisComponents[0] = VertexStreamComponent(
					&params.mLODResource->mVertexBuffer,
					tangentXOffset,
					params.mLODResource->mVertexBuffer.getStride(),
					params.mLODResource->mVertexBuffer.getUseHighPrecisionTangentBasis() ? TStaticMeshVertexTangentTypeSelector<EStaticMeshVertexTangentBasisType::HighPrecision>::VertexElementType : TStaticMeshVertexTangentTypeSelector<EStaticMeshVertexTangentBasisType::Default>::VertexElementType
				);
				data.mTangentBasisComponents[1] = VertexStreamComponent(
					&params.mLODResource->mVertexBuffer,
					tangentYOffset,
					params.mLODResource->mVertexBuffer.getStride(),
					params.mLODResource->mVertexBuffer.getUseHighPrecisionTangentBasis() ? TStaticMeshVertexTangentTypeSelector<EStaticMeshVertexTangentBasisType::HighPrecision>::VertexElementType : TStaticMeshVertexTangentTypeSelector<EStaticMeshVertexTangentBasisType::Default>::VertexElementType);




				if (params.bOverrideColorVertexBuffer)
				{
					data.mColorComponent = VertexStreamComponent(&GNullColorVertexBuffer, 0, sizeof(Color), VET_Color, false, true);
				}
				else
				{
					ColorVertexBuffer* lodColorVertexBuffer = &params.mLODResource->mColorVertexBuffer;
					if (lodColorVertexBuffer->getNumVertices() > 0)
					{
						data.mColorComponent = VertexStreamComponent(
							lodColorVertexBuffer,
							0,
							lodColorVertexBuffer->getStride()
							, VET_Color
						);
					}
				}

				data.mTextureCoordinates.empty();
				uint32 uvSizeInBytes = params.mLODResource->mVertexBuffer.getUseFullPrecisionUVs() ? sizeof(TStaticMeshVertexUVsTypeSelector<EStaticMeshVertexUVType::HighPrecision>::UVsTypeT) : sizeof(TStaticMeshVertexUVsTypeSelector<EStaticMeshVertexUVType::Default>::UVsTypeT);
				EVertexElementType uvDoubleWideVertexElementType = params.mLODResource->mVertexBuffer.getUseFullPrecisionUVs() ? VET_Float4 : VET_Half4;
				EVertexElementType uvVertexElementType = params.mLODResource->mVertexBuffer.getUseFullPrecisionUVs() ? VET_Float2 : VET_Half2;
				uint32 uvIndex;
				for (uvIndex = 0; uvIndex < (uint32)params.mLODResource->mVertexBuffer.getNumTexCoords() - 1; uvIndex += 2)
				{
					data.mTextureCoordinates.add(VertexStreamComponent(
						&params.mLODResource->mVertexBuffer,
						uvBaseOffset + uvSizeInBytes * uvIndex,
						params.mLODResource->mVertexBuffer.getStride(),
						uvDoubleWideVertexElementType));
				}
				if (uvIndex < (int32)params.mLODResource->mVertexBuffer.getNumTexCoords())
				{
					data.mTextureCoordinates.add(VertexStreamComponent(
						&params.mLODResource->mVertexBuffer,
						uvBaseOffset + uvSizeInBytes * uvIndex,
						params.mLODResource->mVertexBuffer.getStride(),
						uvVertexElementType
					));
				}
				params.mVertexFactory->setData(data);
			}
		);
	}

	void StaticMeshVertexBuffer::initRHI()
	{
		BOOST_ASSERT(mVertexData);
		ResourceArrayInterface* resourceArray = mVertexData->getResourceArray();
		if (resourceArray->getResourceDataSize())
		{
			RHIResourceCreateInfo info(resourceArray);
			mVertexBufferRHI = RHICreateVertexBuffer(resourceArray->getResourceDataSize(), BUF_Static, info);
		}
	}

	void StaticMeshVertexBuffer::cleanUp()
	{
		if (mVertexData)
		{
			delete mVertexData;
			mVertexData = nullptr;
		}
	}

	void StaticMeshVertexBuffer::allocateData(bool bNeedsCPUAccess /* = true */)
	{
		cleanUp();
		SELECT_STATIC_MESH_VERTEX_TYPE(
			getUseHighPrecisionTangentBasis(),
			getUseFullPrecisionUVs(),
			getNumTexCoords(),
			mVertexData = new TStaticMeshVertexData<VertexType>(bNeedsCPUAccess);
		);
		mStride = mVertexData->getStride();
	}

	void StaticMeshVertexBuffer::serialize(Archive& ar, bool bNeedsCPUAccess)
	{
		ar << mNumTexcoords << mStride << mNumVertices;
		ar << bUseFullPrecisionUVs;
		ar << bUseHighPrecisionTangentBasis;
		if (ar.isLoading())
		{
			allocateData(bNeedsCPUAccess);
		}
		if (mVertexData != nullptr)
		{
			mVertexData->serialize(ar);
			mData = mVertexData->getDataPointer();
		}
	}

	void StaticMeshVertexBuffer::init(const TArray<StaticMeshBuildVertex>& inVertices, uint32 inNumTexCoords)
	{
		mNumTexcoords = inNumTexCoords;
		mNumVertices = inVertices.size();
		allocateData();
		mVertexData->resizeBuffer(mNumVertices);
		mData = mVertexData->getDataPointer();

		for (int32 vertexIndex = 0; vertexIndex < inVertices.size(); vertexIndex++)
		{
			const StaticMeshBuildVertex& sourceVertex = inVertices[vertexIndex];
			const uint32 destVertexIndex = vertexIndex;
			setVertexTangents(destVertexIndex, sourceVertex.TangentX, sourceVertex.TangentY, sourceVertex.TangentZ);
			for (uint32 uvIndex = 0; uvIndex < mNumTexcoords; uvIndex++)
			{
				setVertexUV(destVertexIndex, uvIndex, sourceVertex.UVs[uvIndex]);
			}
		}
	}

	class PositionVertexData : public TStaticMeshVertexData<PositionVertex>
	{
	public:
		PositionVertexData(bool inNeedsCPUAccess = false)
			:TStaticMeshVertexData<PositionVertex>(inNeedsCPUAccess)
		{

		}
	};

	void PositionVertexBuffer::allocateData(bool bNeedsCPUAccess /* = true */)
	{
		cleanUp();
		mVertexData = new PositionVertexData(bNeedsCPUAccess);
		mStride = mVertexData->getStride();
	}

	void PositionVertexBuffer::cleanUp()
	{
		if (mVertexData)
		{
			delete mVertexData;
			mVertexData = nullptr;
		}
	}

	void PositionVertexBuffer::initRHI()
	{
		BOOST_ASSERT(mVertexData);
		ResourceArrayInterface* resourceArray = mVertexData->getResourceArray();
		if (resourceArray->getResourceDataSize())
		{
			RHIResourceCreateInfo info(resourceArray);
			mVertexBufferRHI = RHICreateVertexBuffer(resourceArray->getResourceDataSize(), BUF_Static, info);
		}
	}

	void PositionVertexBuffer::serialize(Archive& ar, bool bNeedsCPUAccess)
	{
		ar << mStride << mNumVertices;
		if (ar.isLoading())
		{
			allocateData(bNeedsCPUAccess);
		}
		if (mVertexData != nullptr)
		{
			mVertexData->serialize(ar);
			mData = mVertexData->getDataPointer();
		}
	}

	void PositionVertexBuffer::init(const TArray<StaticMeshBuildVertex>& inVertices)
	{
		mNumVertices = inVertices.size();
		allocateData();
		mVertexData->resizeBuffer(mNumVertices);
		mData = mVertexData->getDataPointer();
		for (int32 vertexIndex = 0; vertexIndex < inVertices.size(); vertexIndex++)
		{
			const StaticMeshBuildVertex& sourceVertex = inVertices[vertexIndex];
			const uint32 destVertexIndex = vertexIndex;
			vertexPosition(destVertexIndex) = sourceVertex.mPosition;
		}
	}

	StaticMeshRenderData::StaticMeshRenderData()
	{
		for (int32 lodIndex = 0; lodIndex < MAX_STATIC_MESH_LODS; lodIndex++)
		{
			mScreenSize[lodIndex] = 0.0f;
		}
	}

	void StaticMeshRenderData::serialize(Archive& ar, RStaticMesh* owner, bool bCooked)
	{

	}


	void StaticMeshRenderData::resolveSectionInfo(RStaticMesh* owner)
	{
		int32 lodIndex = 0;
		int32 maxLods = mLODResources.size();
		BOOST_ASSERT(maxLods <= MAX_STATIC_MESH_LODS);
		for (; lodIndex < maxLods; lodIndex++)
		{
			StaticMeshLODResources& lod = mLODResources[lodIndex];
			for (int32 sectionIndex = 0 ; sectionIndex < lod.mSections.size(); ++sectionIndex)
			{
				MeshSectionInfo info = owner->mSectionInfoMap.get(lodIndex, sectionIndex);
				StaticMeshSection& section = lod.mSections[sectionIndex];
				section.mMaterialIndex = info.mMaterialIndex;
				section.bEnableCollision = info.bEnableCollision;
				section.bCastShadow = info.bCastShadow;
			}

			const float autoComputeLODPowerBase = 0.75f;
			if (owner->bAutoComputeLODScreenSize)
			{
				if (lodIndex == 0)
				{
					mScreenSize[lodIndex] = 1.0f;
				}
				else if (lod.mMaxDeviation <= 0.0f)
				{
					mScreenSize[lodIndex] = Math::pow(autoComputeLODPowerBase, lodIndex);
				}
				else
				{
					const float viewDistance = calculateViewDistance(lod.mMaxDeviation, owner->mSourceModels[lodIndex].mReductionSettings.mPixelError);

					const float holfFOV = PI * 0.25f;
					const float screenWidth = 1920.0f;
					const float screenHeight = 1080.0f;

					const PerspectiveMatrix projMatrix(holfFOV, screenWidth, screenHeight, 1.0f);
					mScreenSize[lodIndex] = computeBoundsScreenSize(float3::Zero, mBounds.mSphereRadius, float3(0.0f, 0.0f, viewDistance + mBounds.mSphereRadius), projMatrix);
				}
			}
			else if (owner->mSourceModels.isValidIndex(lodIndex))
			{
				mScreenSize[lodIndex] = owner->mSourceModels[lodIndex].mScreenSize;
			}
			else
			{
				BOOST_ASSERT(lodIndex > 0);
				const float tolerance = 0.01f;
				float autoDisplayFactor = Math::pow(autoComputeLODPowerBase, lodIndex);

				mScreenSize[lodIndex] = Math::clamp(autoDisplayFactor, 0.0f, mScreenSize[lodIndex - 1] - tolerance);
			}
		}
		for (; lodIndex < MAX_STATIC_MESH_LODS; ++lodIndex)
		{
			mScreenSize[lodIndex] = 0.0f;
		}
	}

	void StaticMeshRenderData::initResource(RStaticMesh* owner)
	{
#if WITH_EDITOR
		resolveSectionInfo(owner);
#endif
		for (int32 lodIndex = 0; lodIndex < mLODResources.size(); ++lodIndex)
		{
			mLODResources[lodIndex].initResources(owner);
		}
	}

	void StaticMeshRenderData::releaseResources()
	{
		for (int32 lodIndex = 0; lodIndex < mLODResources.size(); lodIndex++)
		{
			mLODResources[lodIndex].releaseResource();
		}
	}

	void StaticMeshRenderData::allocateLODResource(int32 numLODs)
	{
		BOOST_ASSERT(mLODResources.size() == 0);
		while (mLODResources.size() < numLODs)
		{
			new(mLODResources)StaticMeshLODResources();
		}


	}

	SIZE_T StaticMeshRenderData::getResourceSize() const
	{
		return getResourceSizeBytes();
	}

	SIZE_T StaticMeshRenderData::getResourceSizeBytes() const
	{
		return 0;
	}

	void StaticMeshRenderData::computeUVDensities()
	{
#if WITH_EDITORONLY_DATA
		for (StaticMeshLODResources& lodModel : mLODResources)
		{
			const int32 numTexCoords = Math::min<int32>(lodModel.getNumTexCoords(), MAX_STATIC_TEXCOORDS);
			for (StaticMeshSection& sectionInfo : lodModel.mSections)
			{
				Memory::memzero(sectionInfo.mUVDensities);
				Memory::memzero(sectionInfo.mWeights);
			}
		}
#endif
	}

#if WITH_EDITORONLY_DATA
	void StaticMeshRenderData::cache(RStaticMesh* owner, const StaticMeshLODSettings& LODSettings)
	{
		{
			int32 numLODs = owner->mSourceModels.size();
			const StaticMeshLODGroup& lodGroup = LODSettings.getLODGroup(owner->mLODGroup);

			TArray<uint8> derivedData;

			IMeshUtilities& meshUtilities = ModuleManager::get().loadModuleChecked<IMeshUtilities>(TEXT("MeshUtilities"));
			meshUtilities.buildStaticMesh(*this, owner->mSourceModels, lodGroup);
			//MemoryWriter ar(derivedData, true);
			//serialize(ar, owner, false);
		}
	}
#endif


	void StaticMeshLODSettings::initialize(const ConfigFile& iniFIle)
	{
		mGroups.findOrAdd(Name_None);

		const TCHAR* iniSection = TEXT("StaticMeshLODSettings");
		const auto& it = iniFIle.find(iniSection);
		if (it != iniFIle.end())
		{
			for (auto& keyIt : it->second)
			{
				wstring groupName = keyIt.first;
				StaticMeshLODGroup& group = mGroups.findOrAdd(groupName);
				readEntry(group, keyIt.second.getValue());
			}
		}

		for (auto& it : mGroups)
		{
			StaticMeshLODGroup& group = it.second;
			float percentTrianglesPerLOD = group.mDefualtSettings[1].mPercentTriangles;
			for (int32 lodIndex = 1; lodIndex < MAX_STATIC_MESH_LODS; ++lodIndex)
			{
				float percentTriangles = group.mDefualtSettings[lodIndex++].mPercentTriangles;
				group.mDefualtSettings[lodIndex] = group.mDefualtSettings[lodIndex - 1];
				group.mDefualtSettings[lodIndex].mPercentTriangles = percentTriangles * percentTrianglesPerLOD;
			}
		}
	}

	void StaticMeshLODSettings::readEntry(StaticMeshLODGroup& group, wstring entry)
	{

	}

	MeshReductionSettings StaticMeshLODGroup::getSettings(const MeshReductionSettings& inSettings, int32 LODIndex) const
	{
		BOOST_ASSERT(LODIndex >= 0 && LODIndex < MAX_STATIC_MESH_LODS);
		MeshReductionSettings finalSettings = inSettings;
		float percentTrianglesMult = (LODIndex == 0) ? mBasePercentTrianglesMult : mSettingsBias.mPercentTriangles;
		finalSettings.mPercentTriangles = Math::clamp(inSettings.mPercentTriangles * percentTrianglesMult, 0.0f, 1.0f);
		finalSettings.mMaxDeviation = Math::max(inSettings.mMaxDeviation + mSettingsBias.mMaxDeviation, 0.0f);
		finalSettings.mPixelError = Math::max(inSettings.mPixelError + mSettingsBias.mPixelError, 1.0f);
		return finalSettings;
	}
}