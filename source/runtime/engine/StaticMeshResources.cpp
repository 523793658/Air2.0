#include "StaticMeshResources.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "StaticMeshVertexData.h"
#include "Math/Color.h"
namespace Air
{
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
		uint32 tangentZOffset = 0;
		uint32 uvBaseOffset = 0;
		SELECT_STATIC_MESH_VERTEX_TYPE(
			params.mLODResource->mVertexBuffer.getUseHighPrecisionTangentBasis(),
			params.mLODResource->mVertexBuffer.getUseFullPrecisionUVs(),
			params.mLODResource->mVertexBuffer.getNumTexCoords(),
			{
				tangentXOffset = STRUCT_OFFSET(VertexType, mTangentX);
				tangentZOffset = STRUCT_OFFSET(VertexType, mTangentZ);
				uvBaseOffset = STRUCT_OFFSET(VertexType, mUVs);
			});

		ENQUEUE_UNIQUE_RENDER_COMMAND_FOURPARAMETER(
			InitStaticMeshVertexFactory,
			InitStaticMeshVertexFactoryParams, params, params,
			uint32, tangentXOffset, tangentXOffset,
			uint32, tangentZOffset, tangentZOffset,
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
				if (params.bOverrideColorVertexBuffer)
				{
					data.mColorComponent = VertexStreamComponent(&GNullColorVertexBuffer, 0, sizeof(Color), VET_Color, false, true);
				}
				else
				{
					ColorVertexBuffer* lodColorVertexBuffer = &params.mLODResource->mColorVertexBuffer;
					if (lodColorVertexBuffer)
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

	void StaticMeshVertexBuffer::cleanUp()
	{
		delete mVertexData;
		mVertexData = nullptr;
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

	void StaticMeshRenderData::initResource(RStaticMesh* owner)
	{
#if WITH_EDITOR
#endif
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
}