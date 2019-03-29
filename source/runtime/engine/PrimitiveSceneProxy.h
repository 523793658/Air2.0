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

		virtual void createRenderThreadResources() {}

		inline const BoxSphereBounds& getBounds() const { return mBounds; }

		inline float getMinDrawDistance() const { return mMinDrawDistance; }

		inline float getMaxDrawDistance() const { return mMaxDrawDistance; }

		inline const Matrix& getLocalToWorld() const { return mLocalToWorld; }

		inline const BoxSphereBounds& getBounds() { return mBounds; }

		inline const BoxSphereBounds& getLocalBounds() const { return mLocalBounds; }

		inline SceneInterface& getScene() { return *mScene; }

		inline int32 getVisibilityId() const { return mVisibilityId; }

		ENGINE_API void updateConstantBufferMaybeLazy();

		virtual void OnTransformChanged(){}

		inline bool shouldRenderInMainPass()const { return bRenderInMainPass; }

		ENGINE_API void updateConstantBuffer();

		ENGINE_API void verifyUsedMaterial(const class MaterialRenderProxy* materialRenderProxy) const;

		virtual void getDynamicMeshElements(const TArray<const SceneView*>& views, const SceneViewFamily& viewFamily, uint32 VisibilityMap, class MeshElementCollector& collector) const {}

		virtual void drawStaticElements(StaticPrimitiveDrawInterface* PDI) {}

		inline bool staticElementsAlwaysUseProxyPrimitiveUniformBuffer() const {
			return bStaticElementsAlwaysUseProxyPrimitiveConstantBuffer;
		}

		ENGINE_API virtual PrimitiveViewRelevance getViewRelevance(const SceneView* view) const;
	private:
		friend class Scene;
		Matrix mLocalToWorld;
		BoxSphereBounds mBounds;
		BoxSphereBounds mLocalBounds;
		float3 mActorPosition;
		TArray<const AActor*> mOwners;
		SceneInterface* mScene;
		

		PrimitiveComponentId mPrimitiveComponentId;
		PrimitiveSceneInfo* mPrimitiveSceneInfo;
		
		TConstantBuffer<PrimitiveConstantShaderParameters> mConstantBuffer;


		float mMinDrawDistance;
		float mMaxDrawDistance;
		int32 mVisibilityId;
		uint32 bRenderInMainPass : 1;

		uint32 bStaticElementsAlwaysUseProxyPrimitiveConstantBuffer : 1;
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
}