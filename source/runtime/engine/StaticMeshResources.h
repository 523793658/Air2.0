#pragma once
#include "EngineMininal.h"
#include "RenderResource.h"
#include "Rendering/ColorVertexBuffer.h"
#include "RawIndexBuffer.h"
#include "LocalVertexFactory.h"
#include "Containers/IndirectArray.h"
#include "PrimitiveSceneProxy.h"
#include "Classes/Engine/StaticMesh.h"
#include "PackedNormal.h"
#define  MAX_STATIC_MESH_LODS	8
namespace Air
{
	class RStaticMesh;
	class Object;

	struct PositionVertex
	{
		float3 mPosition;
		friend Archive& operator << (Archive& ar, PositionVertex& v)
		{
			ar << v.mPosition;
			return ar;
		}
	};



	class StaticMeshVertexDataInterface
	{
	public:
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

		FORCEINLINE uint32 getNumTexCoords() const
		{
			return mNumTexcoords;
		}
		ENGINE_API void cleanUp();

		FORCEINLINE bool getUseHighPrecisionTangentBasis() const
		{
			return bUseHighPrecisionTangentBasis;
		}
		FORCEINLINE void setUseHighPrecisionTangentBasis(bool bUseHighPrecision)
		{
			bUseHighPrecisionTangentBasis = bUseHighPrecision;
		}

		FORCEINLINE bool getUseFullPrecisionUVs() const
		{
			return bUseFullPrecisionUVs;
		}
		void serialize(Archive& ar, bool bNeedsCPUAccess);
		FORCEINLINE int32 getStride() const
		{
			return mStride;
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
		void allocateData(bool bNeedsCPUAccess = true);

		void serialize(Archive& ar, bool bNeedsCPUAccess);

		void cleanUp();

		FORCEINLINE uint32 getStride() const
		{
			return mStride;
		}

		FORCEINLINE uint32 getNumVertices() const
		{
			return mNumVertices;
		}

	private:
		class PositionVertexData* mVertexData;

		uint8* mData;

		uint32 mStride;

		uint32 mNumVertices;


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

#if WITH_EDITORONLY_DATA
		float mUVDensities[MAX_STATIC_TEXCOORDS];
		float mWeights[MAX_STATIC_TEXCOORDS];
#endif

		StaticMeshSection()
			:mMaterialIndex(0)
			, mFirstIndex(0)
			, mNumTriangles(0)
			, mMinVertexIndex(0)
			, mMaxVertexIndex(0)
			, bEnableCollision(false)
			, bCastShadow(true)
		{
#if WITH_EDITORONLY_DATA
			Memory::memzero(mUVDensities);
			Memory::memzero(mWeights);
#endif
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

		float mMaxDeviation;

		uint32 mDepthOnlyNumTriangles;

		StaticMeshLODResources();
		~StaticMeshLODResources();

		void initResources(RStaticMesh* parent);

		void releaseResource();

		void serialize(Archive& ar, Object* owner, int32 idx);

		ENGINE_API int32 getNumTriangles() const;

		ENGINE_API int32 getNumVertices() const;

		ENGINE_API int32 getNumTexCoords() const;

		void initVertexFactory(LocalVertexFactory& inOutVertexFactory, RStaticMesh* inParentMesh, bool bInOverrideColorVertexBuffer);

	private:
#if WITH_EDITOR
		ENGINE_API void resolveSectionInfo(RStaticMesh* owner);
#endif
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

	enum class EStaticMeshVertexTangentBasisType
	{
		Default,
		HighPrecision
	};

	enum class EStaticMeshVertexUVType
	{
		Default,
		HighPrecision
	};

	template<typename TangentTypeT>
	struct TStaticMeshVertexTangentDatum
	{
		TangentTypeT mTangentX;
		TangentTypeT mTangentZ;

		FORCEINLINE float3 getTangentX() const
		{
			return mTangentX;
		}

		FORCEINLINE float3 getTangentZ() const
		{
			return mTangentX;
		}

		FORCEINLINE float3 getTangentY() const
		{
			float3 tanX = getTangentX();
			float3 tanZ = getTangentZ();
			return (float3(tanZ) ^ tanX) * tanZ.w;
		}

		FORCEINLINE void setTangents(float3 x, float3 y, float3 z)
		{
			mTangentX = x;
			mTangentZ = float4(z, getBasisDeterminantSign(x, y, z));
		}
	};

	template<typename UVTypeT, uint32 NumTexCoords>
	struct TStaticMeshVertexUVsDatum
	{
		UVTypeT mUVs[NumTexCoords];
		FORCEINLINE float2 getUV(uint32 texCoordIndex) const
		{
			BOOST_ASSERT(texCoordIndex < NumTexCoords);
			return mUVs[texCoordIndex];
		}

		FORCEINLINE void setUV(uint32 texCoordIndex, float2 uv)
		{
			BOOST_ASSERT(texCoordIndex < NumTexCoords);
			mUVs[texCoordIndex] = uv;
		}
	};

	template<EStaticMeshVertexTangentBasisType TangentBasisType>
	struct TStaticMeshVertexTangentTypeSelector
	{

	};

	template<>
	struct TStaticMeshVertexTangentTypeSelector<EStaticMeshVertexTangentBasisType::Default>
	{
		typedef PackedNormal TangentTypeT;
		static const EVertexElementType VertexElementType = VET_PackedNormal;
	};

	template<>
	struct TStaticMeshVertexTangentTypeSelector<EStaticMeshVertexTangentBasisType::HighPrecision>
	{
		typedef PackagedRGBA16N TangentTypeT;
		static const EVertexElementType VertexElementType = VET_UShort4N;
	};

	template<EStaticMeshVertexUVType UVType>
	struct TStaticMeshVertexUVsTypeSelector
	{};

	template<>
	struct TStaticMeshVertexUVsTypeSelector<EStaticMeshVertexUVType::Default>
	{
		typedef half2 UVsTypeT;
	};

	template<>
	struct TStaticMeshVertexUVsTypeSelector<EStaticMeshVertexUVType::HighPrecision>
	{
		typedef float2 UVsTypeT;
	};

#define ConstExprStaticMeshVertexTypeID(bUseHighPrecisionTangentBasis, bUseFullPrecisionUVs, texCoordCount) \
	(((((bUseHighPrecisionTangentBasis) ? 1: 0) * 2) + ((bUseFullPrecisionUVs) ? 1:0)) * MAX_STATIC_TEXCOORDS) + ((texCoordCount) -1)

	FORCEINLINE uint32 computeStaticMeshVertexTypeID(bool bUseHighPrecisionTangentBasis, bool bUseFullPrecisionUVs, uint32 texCoordCount)
	{
		return ConstExprStaticMeshVertexTypeID(bUseHighPrecisionTangentBasis, bUseFullPrecisionUVs, texCoordCount);
	}

	template<EStaticMeshVertexTangentBasisType TangentBasisTypeT, EStaticMeshVertexUVType UVTypeT, uint32 NumTexCoordsT>
	struct TStaticMeshFullVertex : public TStaticMeshVertexTangentDatum<typename TStaticMeshVertexTangentTypeSelector<TangentBasisTypeT>::TangentTypeT>, public TStaticMeshVertexUVsDatum<typename TStaticMeshVertexUVsTypeSelector<UVTypeT>::UVsTypeT, NumTexCoordsT>
	{
		static_assert(NumTexCoordsT > 0, "Must have at least 1 texCoord.");
		static const EStaticMeshVertexTangentBasisType tangentBasisType = TangentBasisTypeT;
		static const EStaticMeshVertexUVType UVType = UVTypeT;
		static const uint32 NumTexCoords = NumTexCoordsT;
		static const uint32 VertexTypeID = ConstExprStaticMeshVertexTypeID(TangentBasisTypeT == EStaticMeshVertexTangentBasisType::HighPrecision, UVTypeT == EStaticMeshVertexUVType::HighPrecision, NumTexCoordsT);

		friend Archive& operator <<(Archive& ar, TStaticMeshFullVertex& vertex)
		{
			ar << vertex.mTangentX;
			ar << vertex.mTangentZ;
			for (uint32 uvIndex = 0; uvIndex < NumTexCoordsT; uvIndex++)
			{
				ar << vertex.mUVs[uvIndex];
			}
			return ar;
		}
	};

#define APPLY_TO_STATIC_MESH_VERTEX(tangentBasisType, UVType, UVCount, ...) \
	{\
		typedef TStaticMeshFullVertex<tangentBasisType, UVType, UVCount> VertexType;\
		case VertexType::VertexTypeID: {__VA_ARGS__}break;\
	}

#define SELECT_STATIC_MESH_VERTEX_TYPE_WITH_TEX_COORDS(TangentBasisType, UVType, ...) \
	APPLY_TO_STATIC_MESH_VERTEX(TangentBasisType, UVType, 1, __VA_ARGS__); \
	APPLY_TO_STATIC_MESH_VERTEX(TangentBasisType, UVType, 2, __VA_ARGS__); \
	APPLY_TO_STATIC_MESH_VERTEX(TangentBasisType, UVType, 3, __VA_ARGS__); \
	APPLY_TO_STATIC_MESH_VERTEX(TangentBasisType, UVType, 4, __VA_ARGS__); \
	APPLY_TO_STATIC_MESH_VERTEX(TangentBasisType, UVType, 5, __VA_ARGS__); \
	APPLY_TO_STATIC_MESH_VERTEX(TangentBasisType, UVType, 6, __VA_ARGS__); \
	APPLY_TO_STATIC_MESH_VERTEX(TangentBasisType, UVType, 7, __VA_ARGS__); \
	APPLY_TO_STATIC_MESH_VERTEX(TangentBasisType, UVType, 8, __VA_ARGS__); 

#define SELECT_STATIC_MESH_VERTEX_TYPE(bIsHighPrecisionTangentsBias, bIsHighPrecisionUVs, numTexCoords, ...)  \
	{\
		uint32 vertexTypeID = computeStaticMeshVertexTypeID(bIsHighPrecisionTangentsBias, bIsHighPrecisionUVs, numTexCoords);\
		switch (vertexTypeID)\
		{\
			SELECT_STATIC_MESH_VERTEX_TYPE_WITH_TEX_COORDS(EStaticMeshVertexTangentBasisType::Default, EStaticMeshVertexUVType::Default,  __VA_ARGS__);\
			SELECT_STATIC_MESH_VERTEX_TYPE_WITH_TEX_COORDS(EStaticMeshVertexTangentBasisType::Default, EStaticMeshVertexUVType::HighPrecision,  __VA_ARGS__);\
			SELECT_STATIC_MESH_VERTEX_TYPE_WITH_TEX_COORDS(EStaticMeshVertexTangentBasisType::HighPrecision, EStaticMeshVertexUVType::Default,  __VA_ARGS__);\
			SELECT_STATIC_MESH_VERTEX_TYPE_WITH_TEX_COORDS(EStaticMeshVertexTangentBasisType::HighPrecision, EStaticMeshVertexUVType::HighPrecision,  __VA_ARGS__);\
		};\
	}
}