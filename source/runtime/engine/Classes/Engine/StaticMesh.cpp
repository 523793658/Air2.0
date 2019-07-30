#include "Classes/Engine/StaticMesh.h"
#include "StaticMeshResources.h"
#include "SimpleReflection.h"
#include "Misc/App.h"
#include "Classes/Engine/TextureStreamingTypes.h"
#include "Interface/ITargetPlatform.h"
namespace Air
{
#if WITH_EDITORONLY_DATA
	const float RStaticMesh::mMinimumAutoLODPixelError = SMALL_NUMBER;
#endif

#if WITH_EDITOR
	static StaticMeshRenderData& getPlatformStaticMeshRenderData(RStaticMesh* mesh, const ITargetPlatform* platform)
	{
		BOOST_ASSERT(mesh && mesh->mRenderData);
		const StaticMeshLODSettings& platformLODSettings = platform->getStaticMeshLODSettings();
		StaticMeshRenderData* platformRenderData = mesh->mRenderData.get();
		if (platformRenderData == nullptr)
		{
			platformRenderData = new StaticMeshRenderData();
			platformRenderData->cache(mesh, platformLODSettings);
		}
		return *platformRenderData;
	}
#endif


	RStaticMesh::RStaticMesh(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{

	}

	bool operator == (const MeshSectionInfo& a, const MeshSectionInfo& b)
	{
		return a.mMaterialIndex == b.mMaterialIndex
			&& a.bEnableCollision == b.bEnableCollision
			&& a.bCastShadow == b.bCastShadow;
	}

	bool operator !=(const MeshSectionInfo& a, const MeshSectionInfo& b)
	{
		return !(a == b);
	}

	Archive& operator <<(Archive& ar, StaticMaterial& elem)
	{
		ar << elem.mMaterialInterface;
		ar << elem.mMaterialSlotName;
#if WITH_EDITORONLY_DATA
		ar << elem.mImportedMaterialSlotName;
#endif
		if (!ar.isLoading())
		{
			ar << elem.mUVChannelData;
		}
		return ar;
	}

	static uint32 getMeshMaterialKey(int32 LODIndex, int32 sectionIndex)
	{
		return ((LODIndex & 0xffff) << 16) | (sectionIndex & 0xffff);
	}

	MeshSectionInfo MeshSectionInfoMap::get(int32 LODIndex, int32 sectionIndex) const
	{
		uint32 key = getMeshMaterialKey(LODIndex, sectionIndex);
		auto &infoIt = mMap.find(key);
		if (infoIt == mMap.end())
		{
			key = getMeshMaterialKey(0, sectionIndex);
			infoIt = mMap.find(key);
		}
		if (infoIt == mMap.end())
		{
			return MeshSectionInfo(sectionIndex);
		}
		return infoIt->second;
	}

	void MeshSectionInfoMap::set(int32 LODIndex, int32 sectionIndex, MeshSectionInfo info)
	{
		uint32 key = getMeshMaterialKey(LODIndex, sectionIndex);
		mMap.emplace(key, info);
	}

	void MeshSectionInfoMap::serialize(Archive& ar)
	{
		ar << mMap;
	}

	bool operator == (const StaticMaterial& lhs, const StaticMaterial& rhs)
	{
		return (lhs.mMaterialInterface == rhs.mMaterialInterface &&
			lhs.mMaterialSlotName == rhs.mMaterialSlotName
#if WITH_EDITORONLY_DATA
			&& lhs.mImportedMaterialSlotName == rhs.mImportedMaterialSlotName
#endif
			);
	}

	bool operator == (const StaticMaterial& lhs, const MaterialInterface& rhs)
	{
		return (lhs.mMaterialInterface.get() == &rhs);
	}

	bool operator == (const MaterialInterface& lhs, const StaticMaterial& rhs)
	{
		return (&lhs == rhs.mMaterialInterface.get());
	}

	void MeshSectionInfoMap::remove(int32 LODIndex, int32 sectionIndex)
	{
		uint32 key = getMeshMaterialKey(LODIndex, sectionIndex);
		mMap.erase(key);
	}

	void StaticMeshLODResources::initResources(RStaticMesh* parent)
	{
		const auto maxShaderPlatform = GShaderPlatformForFeatureLevel[GMaxRHIFeatureLevel];
		if (isES2Platform(maxShaderPlatform) && !isMetalPlatform(maxShaderPlatform))
		{
			if (mIndexBuffer.is32Bit())
			{
				TArray<uint32> indices;
				mIndexBuffer.getCopy(indices);
				mIndexBuffer.setIndices(indices, EIndexBufferStride::Force16Bit);
			}
		}
		beginInitResource(&mIndexBuffer);
		if (mWireframeIndexBuffer.getNumIndices())
		{
			beginInitResource(&mWireframeIndexBuffer);
		}
		beginInitResource(&mVertexBuffer);
		beginInitResource(&mPositionVertexBuffer);
		if (mColorVertexBuffer.getNumVertices() > 0)
		{
			beginInitResource(&mColorVertexBuffer);
		}
		if (mReversedIndexBuffer.getNumIndices() > 0)
		{
			beginInitResource(&mReversedIndexBuffer);
		}
		if (mDepthOnlyIndexBuffer.getNumIndices() > 0)
		{
			beginInitResource(&mDepthOnlyIndexBuffer);
		}
		if (mReversedDepthOnlyIndexBuffer.getNumIndices() > 0)
		{
			beginInitResource(&mReversedDepthOnlyIndexBuffer);
		}
		if (RHISupportsTessellation(maxShaderPlatform))
		{
			beginInitResource(&mAdjacencyIndexBuffer);
		}

		initVertexFactory(mVertexFactory, parent, false);
		beginInitResource(&mVertexFactory);
		initVertexFactory(mVertexFactoryOverrideColorVertexBuffer, parent, true);
		beginInitResource(&mVertexFactoryOverrideColorVertexBuffer);
	}

	void StaticMeshLODResources::releaseResource()
	{
		beginReleaseResource(&mAdjacencyIndexBuffer);
		beginReleaseResource(&mIndexBuffer);
		beginReleaseResource(&mWireframeIndexBuffer);
		beginReleaseResource(&mVertexBuffer);
		beginReleaseResource(&mPositionVertexBuffer);
		beginReleaseResource(&mColorVertexBuffer);
		beginReleaseResource(&mReversedIndexBuffer);
		beginReleaseResource(&mDepthOnlyIndexBuffer);
		beginReleaseResource(&mReversedDepthOnlyIndexBuffer);
		beginReleaseResource(&mVertexFactory);
		beginReleaseResource(&mVertexFactoryOverrideColorVertexBuffer);
	}

	int32 StaticMeshLODResources::getNumTriangles() const
	{
		int32 numTriangles = 0;
		for (int32 sectionIndex = 0; sectionIndex < mSections.size(); sectionIndex++)
		{
			numTriangles += mSections[sectionIndex].mNumTriangles;
		}
		return numTriangles;
	}

	int32 StaticMeshLODResources::getNumVertices() const
	{
		return mVertexBuffer.getNumVertices();
	}

	int32 StaticMeshLODResources::getNumTexCoords() const
	{
		return mVertexBuffer.getNumVertices();
	}

	Archive& operator << (Archive& ar, MeshUVChannelInfo& channelInfo)
	{
		ar << channelInfo.bInitialized;
		ar << channelInfo.bOverrideDensities;
		for (int32 coordIndex = 0; coordIndex < TEXSTREAM_MAX_UVCHANNELS; coordIndex++)
		{
			ar << channelInfo.mLocalDensities[coordIndex];
		}
		return ar;
	}


	StaticMeshSourceModel::StaticMeshSourceModel()
	{
#if WITH_EDITOR
		mRawMeshBulkData = new RawMeshBulkData();
#endif
	}

	StaticMeshSourceModel::~StaticMeshSourceModel()
	{
#if WITH_EDITOR
		if (mRawMeshBulkData)
		{
			delete mRawMeshBulkData;
			mRawMeshBulkData = nullptr;
		}
#endif

		
	}

	void RStaticMesh::postLoad()
	{
		ParentType::postLoad();

#if WITH_EDITOR
		cacheDerivedData();


#endif


		if (App::canEverRender() && !hasAnyFlags(RF_ClassDefaultObject))
		{
			initResource();
		}
	}

	void RStaticMesh::initResource()
	{
		updateUVChannelData(false);
		if (mRenderData)
		{
			mRenderData->initResource(this);
		}
	}

	void RStaticMesh::updateUVChannelData(bool bRebuildAll)
	{

	}

	void RStaticMesh::cacheDerivedData()
	{
		ITargetPlatformManagerModule& targetPlatformManager = getTargetPlatformManagerRef();
		ITargetPlatform* runningPlatform = targetPlatformManager.getRunningTargetPlatform();

		BOOST_ASSERT(runningPlatform);

		const StaticMeshLODSettings& LODSettings = runningPlatform->getStaticMeshLODSettings();



		mRenderData = makeUniquePtr<StaticMeshRenderData>();
		mRenderData->cache(this, LODSettings);
		const TArray<ITargetPlatform*>& targetPlatforms = targetPlatformManager.getActiveTargetPlatforms();
		for (int32 platformIndex = 0; platformIndex < targetPlatforms.size(); platformIndex++)
		{
			ITargetPlatform* platform = targetPlatforms[platformIndex];
			if (platform != runningPlatform)
			{
				getPlatformStaticMeshRenderData(this, platform);
			}
		}
	}

	DECLARE_SIMPLER_REFLECTION(RStaticMesh);
}