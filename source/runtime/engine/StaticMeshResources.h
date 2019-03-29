#pragma once
#include "EngineMininal.h"
#include "RenderResource.h"
#include "Rendering/ColorVertexBuffer.h"
#include "RawIndexBuffer.h"
#include "LocalVertexFactory.h"
#include "Containers/IndirectArray.h"
#include "Classes/Engine/StaticMesh.h"
#include "PrimitiveSceneProxy.h"
namespace Air
{
	class RStaticMesh;
	class Object;
	class StaticMeshVertexDataInterface
	{
		virtual ~StaticMeshVertexDataInterface() {}

		virtual void resizeBuffer(uint32 numVertices) = 0;

		virtual uint32 getStride() const = 0;

		virtual uint8* getDataPointer() = 0;

		virtual ResourceArrayInterface* getResourceArray() = 0;

		virtual void serialize(Archive& ar) = 0;
	};

	class StaticMeshVertexBuffer : public VertexBuffer
	{

	public:
		FORCEINLINE uint32 getNumVertices() const
		{
			return mNumVertices;
		}

	private:
		StaticMeshVertexDataInterface* mVertexData;

		uint32 mNumTexcoords;

		uint8* mData;

		uint32 mStride;

		uint32 mNumVertices;

		bool bUseFullPrecisionUVs;

		bool bUseHighPrecisionTangentBasis;

		void allocateData(bool bNeedsCPUAccess = true);
	};

	class PositionVertexBuffer : public VertexBuffer
	{
	public:
	private:
		class PositionVertexData* mVertexData;

		uint8* mData;

		uint32 mStride;

		uint32 mNumVertices;

		void allocateData(bool bNeedsCPUAccess = true);
	};

	struct StaticMeshSection
	{
		int32 mMaterialIndex;
		
		uint32 mFirstIndex;
		uint32 mNumTriangles;
		uint32 mMinVertexIndex;
		uint32 mMaxVertexIndex;

		bool bEnableCollision;
		bool bCastShadow;

		StaticMeshSection()
			:mMaterialIndex(0)
			, mFirstIndex(0)
			, mNumTriangles(0)
			, mMinVertexIndex(0)
			, mMaxVertexIndex(0)
			, bEnableCollision(false)
			, bCastShadow(true)
		{

		}

		friend Archive& operator << (Archive& ar, StaticMeshSection& section);
	};

	struct StaticMeshLODResources
	{
		StaticMeshVertexBuffer mVertexBuffer;

		PositionVertexBuffer mPositionVertexBuffer;

		ColorVertexBuffer mColorVertexBuffer;

		RawStaticIndexBuffer mIndexBuffer;

		RawStaticIndexBuffer mReversedIndexBuffer;

		RawStaticIndexBuffer mDepthOnlyIndexBuffer;

		RawStaticIndexBuffer mReversedDepthOnlyIndexBuffer;

		RawStaticIndexBuffer mWireframeIndexBuffer;

		RawStaticIndexBuffer mAdjacencyIndexBuffer;

		LocalVertexFactory mVertexFactory;

		LocalVertexFactory mVertexFactoryOverrideColorVertexBuffer;

		TArray<StaticMeshSection> mSections;

		uint32 bHasAdjacencyInfo : 1;
		uint32 bHasDepthOnlyIndices : 1;
		uint32 bHasReversedIndices : 1;
		uint32 bHasreversedDepthOnlyIndices : 1;

		uint32 mDepthOnlyNumTriangles;

		StaticMeshLODResources();
		~StaticMeshLODResources();

		void initResource(RStaticMesh* parent);

		void releaseResource();

		void serialize(Archive& ar, Object* owner, int32 idx);

		ENGINE_API int32 getNumTriangles() const;

		ENGINE_API int32 getNumVertices() const;

		ENGINE_API int32 getNumTexCoords() const;

		void initVertexFactory(LocalVertexFactory& inOutVertexFactory, RStaticMesh* inParentMesh, bool bInOverrideColorVertexBuffer);
	};

	class StaticMeshRenderData
	{
	public:
		StaticMeshRenderData();

		TindirectArray<StaticMeshLODResources> mLODResources;

		float mScreenSize[MAX_STATIC_MESH_LODS];
		BoxSphereBounds mBounds;

		void serialize(Archive& ar, RStaticMesh* owner, bool bCooked);

		void initResource(RStaticMesh* owner);

		ENGINE_API void releaseResources();

		SIZE_T getResourceSize() const;

		SIZE_T  getResourceSizeBytes() const;

		ENGINE_API void allocateLODResource(int32 numLODs);

		void computeUVDensities();


	};

	class StaticMeshComponent;

	class ENGINE_API StaticMeshSceneProxy : public PrimitiveSceneProxy
	{
	public:
		StaticMeshSceneProxy(StaticMeshComponent* component, bool bCanLODsShareStaticLighting);

		virtual ~StaticMeshSceneProxy() {}

	protected:
		AActor* mOwner;
		const RStaticMesh* mStaticMesh;

		StaticMeshRenderData* mRenderData;

		uint32 bCastShadow : 1;
	};
}