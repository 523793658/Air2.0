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
#include "Classes/Engine/MeshMerging.h"
#include "RenderUtils.h"
#include "Classes/Materials/MaterialInterface.h"
#define  MAX_STATIC_MESH_LODS	8
namespace Air
{
	class RStaticMesh;
	class Object;


	

	
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



	class StaticMeshLODGroup
	{
	public:
		StaticMeshLODGroup()
			:mDefaultNumLODs(1)
			, mDefaultLightMapResolution(64)
			, mBasePercentTrianglesMult(1.0f)
			, mDisplayName(TEXT("None"))
		{
			Memory::memzero(mSettingsBias);
			mSettingsBias.mPercentTriangles = 1.0f;
		}

		int32 getDefaultNumLODs() const
		{
			return mDefaultNumLODs;
		}

		int32 getDefaultLightMapResolution()
		{
			return mDefaultLightMapResolution;
		}

		MeshReductionSettings getDefaultSettings(int32 LODIndex) const
		{
			BOOST_ASSERT(LODIndex >= 0 && LODIndex < MAX_STATIC_MESH_LODS);
			return mDefualtSettings[LODIndex];
		}

		ENGINE_API MeshReductionSettings getSettings(const MeshReductionSettings& inSettings, int32 LODIndex) const;




	private:
		friend class StaticMeshLODSettings;

		int32 mDefaultNumLODs;

		int32 mDefaultLightMapResolution;

		float mBasePercentTrianglesMult;

		wstring mDisplayName;

		MeshReductionSettings mDefualtSettings[MAX_STATIC_MESH_LODS];

		MeshReductionSettings mSettingsBias;
	};


	class StaticMeshLODSettings
	{
	public:
		ENGINE_API void initialize(const ConfigFile& iniFIle);

		const StaticMeshLODGroup& getLODGroup(wstring name) const
		{
			auto& it = mGroups.find(name);

			if (it == mGroups.end())
			{
				it = mGroups.find(Name_None);
			}
			BOOST_ASSERT(it != mGroups.end());
			return it->second;
		}


		void getLODGroupNames(TArray<wstring>& outNames) const;

		void getLODGroupDisplayNames(TArray<wstring>& outDisplayNames) const;
	private:

		void readEntry(StaticMeshLODGroup& group, wstring entry);
		TMap<wstring, StaticMeshLODGroup> mGroups;
	};

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

		virtual bool getAllowCPUAccess() const = 0;

		virtual uint32 size() const = 0;

	};

	

	class StaticMeshVertexBuffer : public VertexBuffer
	{

	public:
		class TangentsVertexBuffer : public VertexBuffer
		{
			virtual wstring getFriendlyName() const override { return TEXT("TangentsVertexBuffer"); }
		}_TangentsVertexBuffer;

		class TexcoordVertexBuffer : public VertexBuffer
		{
			virtual wstring getFriendlyName() const override { return TEXT("TexcoordVertexBuffer"); }
		}_TexcoordVertexBuffer;



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

		FORCEINLINE void setUseFullPrecisionUVs(bool useFull)
		{
			bUseFullPrecisionUVs = useFull;
		}

		FORCEINLINE bool getUseFullPrecisionUVs() const
		{
			return bUseFullPrecisionUVs;
		}
		void serialize(Archive& ar, bool bNeedsCPUAccess);

		void convertHalfTexcoordsToFloat(const uint8* inData);

		FORCEINLINE int32 getStride() const
		{
			return mStride;
		}

		ENGINE_API void init(const TArray<StaticMeshBuildVertex>& inVertices, uint32 inNumTexCoords, bool needCPUAccess);

		FORCEINLINE void setVertexUV(uint32 vertexIndex, uint32 uvIndex, const float2& vec2D);

		FORCEINLINE void setVertexTangents(uint32 vertexIndex, float3 x, float3 y, float3 z);

		virtual void initRHI() override;

		ENGINE_API void bindTangentVertexBuffer(const VertexFactory* vertexFactory, struct StaticMeshDataType& data) const;

		ENGINE_API void bindTexCoordVertexBuffer(const VertexFactory* vertexFactory, const StaticMeshDataType& data) const;

		ENGINE_API void bindPackedTexCoordVertexBuffer(const VertexFactory* vertexFactory, StaticMeshDataType& data) const;

		ENGINE_API void bindLightMapVertexBuffer(const VertexFactory* vertexFactory, struct StaticMeshDataType& data, int lightMapCoordinateIndex) const;

		VertexBufferRHIRef createTangentsRHIBuffer_RenderThread();
		VertexBufferRHIRef createTangentsRHIBuffer_Async();
		VertexBufferRHIRef createTexcoordRHIBuffer_RenderThread();
		VertexBufferRHIRef createTexcoordRHIBuffer_Async();

		void serializeMetaData(Archive& ar);

		void initTangentAndTexcoordStrides();

	private:
		template<bool bRenderThread>
		VertexBufferRHIRef createTangentsRHIBuffer_Interal();

		template<bool bRenderThread>
		VertexBufferRHIRef createTexcoordRHIBuffer_Internal();

	private:
		StaticMeshVertexDataInterface * mTangentsData{ nullptr };
		ShaderResourceViewRHIRef mTangentSRV{ nullptr };

		uint8* mTangentDataPtr;
		uint8* mTexcoordDataPtr;

		StaticMeshVertexDataInterface* mTexcoordData{ nullptr };
		ShaderResourceViewRHIRef mTextureCoordinateSRV{ nullptr };



		uint32 mNumTexcoords{ 0 };

		uint8* mData{ nullptr };

		uint32 mStride{ 0 };

		uint32 mNumVertices{ 0 };

		uint32 mTangentsStride{ 0 };

		uint32 mTexcoordStride{ 0 };

		bool bUseFullPrecisionUVs{ false };

		bool bUseHighPrecisionTangentBasis{ false };

		bool bNeedCPUAccess{ true };

		void allocateData(bool bNeedsCPUAccess = true);
	};

	
	class PositionVertexBuffer : public VertexBuffer
	{
	public:
		void allocateData(bool bNeedsCPUAccess = true);

		void serialize(Archive& ar, bool bNeedsCPUAccess);

		void cleanUp();

		ENGINE_API void init(const TArray<StaticMeshBuildVertex>& inVertices);

		FORCEINLINE uint32 getStride() const
		{
			return mStride;
		}

		FORCEINLINE uint32 getNumVertices() const
		{
			return mNumVertices;
		}

		FORCEINLINE float3& vertexPosition(uint32 vertexIndex)
		{
			BOOST_ASSERT(vertexIndex < getNumVertices());
			return ((PositionVertex*)(mData + vertexIndex * mStride))->mPosition;
		}

		virtual void initRHI() override;

		ENGINE_API void bindPositionVertexBuffer(const class VertexFactory* vertexFactory, struct StaticMeshDataType& data) const;

	private:
		ShaderResourceViewRHIRef mPositionComponentSRV;

		class PositionVertexData* mVertexData{ nullptr };

		uint8* mData{ nullptr };

		uint32 mStride{ 0 };

		uint32 mNumVertices{ 0 };


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

	struct StaticMeshVertexBuffers
	{
		StaticMeshVertexBuffer mStaticMeshVertexBuffer;
		PositionVertexBuffer mPositionVertexBuffer;
		ColorVertexBuffer mColorVertexBuffer;

		
	};

	struct AdditionalStaticMeshIndexBuffers
	{
		RawStaticIndexBuffer mReservedIndexBuffer;
		RawStaticIndexBuffer mRevervedDepthOnlyIndexBuffer;
		RawStaticIndexBuffer mWireframeIndexBuffer;
		RawStaticIndexBuffer mAdjacencyIndexBuffer;
	};


	struct StaticMeshLODResources
	{
		StaticMeshVertexBuffers mVertexBuffers;

		RawStaticIndexBuffer mIndexBuffer;

		RawStaticIndexBuffer mDepthOnlyIndexBuffer;

		TArray<StaticMeshSection> mSections;

		AdditionalStaticMeshIndexBuffers* mAdditionalIndexBuffer;

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

		
	private:
#if WITH_EDITOR
		ENGINE_API void resolveSectionInfo(RStaticMesh* owner);
#endif

		void conditionalForce16BitIndexBuffer(EShaderPlatform maxShaderPlatform, RStaticMesh* parent);
	};
	struct StaticMeshVertexFactories;

	class StaticMeshRenderData
	{
	public:
		StaticMeshRenderData();

		TindirectArray<StaticMeshLODResources> mLODResources;
		TindirectArray<StaticMeshVertexFactories> mLODVertexFactories;

		float mScreenSize[MAX_STATIC_MESH_LODS];
		BoxSphereBounds mBounds;

		void serialize(Archive& ar, RStaticMesh* owner, bool bCooked);

		void initResource(RStaticMesh* owner);

		ENGINE_API void releaseResources();

		SIZE_T getResourceSize() const;

		SIZE_T  getResourceSizeBytes() const;

		ENGINE_API void allocateLODResource(int32 numLODs);

		void computeUVDensities();

		void resolveSectionInfo(RStaticMesh* owner);

#if WITH_EDITORONLY_DATA
		void cache(RStaticMesh* owner, const StaticMeshLODSettings& LODSettings);

#endif

#if WITH_EDITORONLY_DATA
		TArray<int32> mWedgeMap;
#endif
	};

	class StaticMeshComponent;

	struct DynamicMeshVertex;

	



	class ENGINE_API StaticMeshSceneProxy : public PrimitiveSceneProxy
	{
	public:
		size_t getTypeHash() const override;

		StaticMeshSceneProxy(StaticMeshComponent* component, bool bCanLODsShareStaticLighting);

		virtual ~StaticMeshSceneProxy() {}

		virtual void drawStaticElements(StaticPrimitiveDrawInterface* PDI) override;

		virtual int32 getNumMeshBatches() const
		{
			return 1;
		}

		virtual bool getMeshElement(
			int32 lodIndex,
			int32 batchIndex,
			int32 elementIndex,
			uint8 inDepthPriorityGroup,
			bool bUseSelectedMaterial,
			bool bAllowPreCulledIndices,
			MeshBatch& outMeshBatch) const;

		virtual PrimitiveViewRelevance getViewRelevance(const SceneView* view) const override;

		void setMeshElementScreenSize(int32 lodIndex, bool bDitheredLODTransition, MeshBatch& outMeshBatch) const;
	protected:
		float getScreenSize(int32 lodIndex) const;

		
		uint32 setMeshElementGeometrySource(
			int32 lodIndex,
			int32 elementIndex,
			bool bWireframe,
			bool bRequiresdIndices,
			bool bUseinversedIndices,
			bool bAllowPreCulledIndices,
			const VertexFactory* vertexFactory,
			MeshBatch& outMeshElement) const;

		bool isReversedCullingNeeded(bool bUseReversedIndices) const;
	protected:
		class LODInfo : public LightCacheInterface
		{
		public:
			struct SectionInfo
			{
				std::shared_ptr<class MaterialInterface> mMaterial;
				bool bSelected;
#if WITH_EDITORONLY_DATA
				int32 mMaterialIndex;
#endif

				int32 mFirstPreCulledIndex;
				int32 mNumPreCulledTriangles;
			};

			TArray<SectionInfo> mSections;
			ColorVertexBuffer* mOverrideColorVertexBuffer;

			const RawStaticIndexBuffer* mPreCulledIndexBuffer;

			LODInfo(const StaticMeshComponent* inComponent, int32 inLodIndex, bool bCanLODsShareStaticLighting);

			bool usesMeshModifyingMaterials() const { return bUsesMeshModifyingMaterials; }


		private:
			TArray<Guid> mIrrelevantLights;
			bool bUsesMeshModifyingMaterials;
		};
	protected:
		AActor* mOwner;
		const RStaticMesh* mStaticMesh;

		StaticMeshRenderData* mRenderData;

		MaterialRelevance mMaterialRelevance;

		TindirectArray<LODInfo> mLODs;

		uint32 bCastShadow : 1;

		uint32 bReverseCulling : 1;

		int32 mForcedLodModel;

		int32 mClampedMinLOD;
	};


	template<typename TangentTypeT>
	struct TStaticMeshVertexTangentDatum
	{
		TangentTypeT mTangentX;
		TangentTypeT mTangentY;

		FORCEINLINE float3 getTangentX() const
		{
			return mTangentX;
		}

		

		FORCEINLINE float4 getTangentY() const
		{
			return mTangentY;
		}

		FORCEINLINE float3 getTangentZ() const
		{
			float3 tanX = getTangentX();
			float4 tanY = getTangentY();
			return (tanX ^ float3(tanY)) * tanY.w;
		}

		FORCEINLINE void setTangents(float3 x, float3 y, float3 z)
		{
			mTangentX = x;
			mTangentY = float4(y, getBasisDeterminantSign(x, y, z));
		}
	};

	template<typename UVTypeT>
	struct TStaticMeshVertexUVsDatum
	{
		UVTypeT mUVs;
		FORCEINLINE float2 getUV() const
		{
			return mUVs;
		}

		FORCEINLINE void setUV(float2 uv)
		{
			mUVs = uv;
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

	






	

	

	struct ENGINE_API StaticMeshVertexFactories
	{
		StaticMeshVertexFactories(ERHIFeatureLevel::Type inFeatureLevel)
			:mVertexFactory(inFeatureLevel, "StaticMeshVertexFactories")
			,mVertexFactoryOverrideColorVertexBuffer(inFeatureLevel, "StaticMeshVertexFactories_Override")
		{}

		LocalVertexFactory mVertexFactory;
		LocalVertexFactory mVertexFactoryOverrideColorVertexBuffer;
		void InitVertexFactory(const StaticMeshLODResources& lodResources, LocalVertexFactory& inOutVertexFactory, uint32 lodIndex, const RStaticMesh* inParentMesh, bool bInOverrideColorVertexBuffer);

		void InitResources(const StaticMeshLODResources& lodResources, uint32 lodIndex, const RStaticMesh* parent);

		void releaseResources();
	};
}