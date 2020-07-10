#pragma once
#include "EngineMininal.h"
#include "Classes/Components/PrimitiveComponent.h"
#include "PrimitiveConstantShaderParameters.h"
#include "SceneView.h"
#include "PrimitiveViewRelevance.h"
namespace Air
{
	class PrimitiveSceneInfo;
	class SceneInterface;
	class AActor;
	class SceneView;
	class StaticPrimitiveDrawInterface;
	class PrimitiveSceneProxy
	{
	public:	
		PrimitiveSceneProxy(const PrimitiveComponent* inComponent, wstring inResourceName = TEXT(""));

		inline PrimitiveSceneInfo* getPrimitiveSceneInfo() const { return mPrimitiveSceneInfo; }

		ENGINE_API void setTransform(const Matrix& inLocalToWorld, const BoxSphereBounds& inBounds, const BoxSphereBounds& inLocalBounds, float3 inActorPosition);

		ENGINE_API virtual size_t getTypeHash() const = 0;

		virtual void createRenderThreadResources() {}

		inline const BoxSphereBounds& getBounds() const { return mBounds; }

		inline float getMinDrawDistance() const { return mMinDrawDistance; }

		inline float getMaxDrawDistance() const { return mMaxDrawDistance; }

		inline const Matrix& getLocalToWorld() const { return mLocalToWorld; }

		inline const BoxSphereBounds& getBounds() { return mBounds; }

		inline const BoxSphereBounds& getLocalBounds() const { return mLocalBounds; }

		inline SceneInterface& getScene() const { return *mScene; }

		inline int32 getVisibilityId() const { return mVisibilityId; }

		ENGINE_API void updateConstantBufferMaybeLazy();

		virtual void OnTransformChanged(){}

		inline bool hasViewDependentPDG() const { return bUseViewOwnerDepthPriorityGroup; }

		inline bool shouldRenderInMainPass()const { return bRenderInMainPass; }

		ENGINE_API void updateConstantBuffer();

		ENGINE_API void verifyUsedMaterial(const class MaterialRenderProxy* materialRenderProxy) const;

		inline RHIConstantBuffer* getConstantBuffer() const
		{
			return mConstantBuffer.getReference();
		}

		virtual void getDynamicMeshElements(const TArray<const SceneView*>& views, const SceneViewFamily& viewFamily, uint32 VisibilityMap, class MeshElementCollector& collector) const {}

		virtual void drawStaticElements(StaticPrimitiveDrawInterface* PDI) {}

		inline bool staticElementsAlwaysUseProxyPrimitiveUniformBuffer() const {
			return bStaticElementsAlwaysUseProxyPrimitiveConstantBuffer;
		}

		inline bool isLocalToWorldDeterminantNegative() const { return bIsLocalToWorldDeterminatantNegative; }

		inline bool isMovable() const
		{
			return mMobility == EComponentMobility::Movable || mMobility == EComponentMobility::Stationary;
		}

		ENGINE_API virtual PrimitiveViewRelevance getViewRelevance(const SceneView* view) const;

		ENGINE_API bool isShown(const SceneView* view) const;

		inline bool isStaticPathAvailable() const { return !bDisableStaticPath; }

		inline bool hasStaticLighting() const { return bStaticLighting; }

		inline bool castsDynamicShadow() const { return bCastDynamicShadow; }
		inline bool castsStaticShadow() const { return bCastStaticShadow; }

		inline bool castsVolumetricTranslucentShadow() const { return bCastVolumetricTranslucentShadow; }

		inline bool doesVFRequirePrimitiveConstantBuffer() const {
			return bVFRequiresPrimitiveConstantBuffer;
		}

		virtual uint8 getStaticDepthPriorityGroup() const
		{
			BOOST_ASSERT(!hasViewDependentPDG());
			return mStaticDepthPriorityGroup;
		}
	private:
		friend class Scene;
		Matrix mLocalToWorld;
		BoxSphereBounds mBounds;
		BoxSphereBounds mLocalBounds;
		float3 mActorPosition;
		TArray<const AActor*> mOwners;
		SceneInterface* mScene;
		
		EComponentMobility::Type mMobility;
		PrimitiveComponentId mPrimitiveComponentId;
		PrimitiveSceneInfo* mPrimitiveSceneInfo;
		
		TConstantBufferRef<PrimitiveConstantShaderParameters> mConstantBuffer;


		float mMinDrawDistance;
		float mMaxDrawDistance;
		int32 mVisibilityId;
		uint32 bRenderInMainPass : 1;
		uint32 bIsLocalToWorldDeterminatantNegative : 1;
		uint32 bDrawInGame : 1;
		uint32 bStaticElementsAlwaysUseProxyPrimitiveConstantBuffer : 1;
		uint32 bDisableStaticPath : 1;
		uint32 bCastDynamicShadow : 1;
		uint32 bCastVolumetricTranslucentShadow : 1;
		uint32 bCastStaticShadow : 1;
		uint32 bStaticLighting : 1;
		uint32 bUseViewOwnerDepthPriorityGroup : 1;
		uint32 bReceivesDecals : 1;
		
		uint8 mStaticDepthPriorityGroup : SDPG_NumBits;
		uint8 mViewOwnerDepthPriorityGroup : SDPG_NumBits;
	protected:
		uint32 bVFRequiresPrimitiveConstantBuffer : 1;
	};

	class SimpleLightEntry
	{
	public:	 
		float3 mColor;
		float mRadius;
		float mExponent;
		bool bAffectTranslucency;
	};

	class SimpleLightPerViewEntry
	{
	public:
		float3 mPosition;
	};

	class SimpleLightInstancePerViewIndexData
	{
	public:
		uint32 mPerViewIndex : 31;
		uint32 bHasPerViewData : 1;
	};

	class SimpleLightArray
	{
	public:
		TArray<SimpleLightEntry, TMemStackAllocator<>> mInstanceData;
		TArray<SimpleLightPerViewEntry, TMemStackAllocator<>> mPerViewData;
		TArray<SimpleLightInstancePerViewIndexData, TMemStackAllocator<>> mInstancePerViewDataIndices;
	};

	ENGINE_API extern bool supportsCachingMeshDrawCommands(const PrimitiveSceneProxy* RESTRICT primitiveSceneProxy, const MeshBatch& meshBatch, ERHIFeatureLevel::Type featureLevel);

	ENGINE_API extern bool supportsCachingMeshDrawCommands(const PrimitiveSceneProxy* RESTRICT primitiveSceneProxy, const MeshBatch& meshBatch);
}