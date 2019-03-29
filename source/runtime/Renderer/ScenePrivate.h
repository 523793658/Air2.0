#pragma once
#include "RendererMininal.h"
#include "SceneView.h"
#include "RenderingThread.h"
#include "LightMapRendering.h"
#include "BasePassRendering.h"
#include "LightSceneInfo.h"
#include "StaticMeshDrawList.h"
#include "SceneInterface.h"
#include "LightSceneInfo.h"
namespace Air
{
	class SceneViewState : public SceneViewStateInterface, public DeferredCleanupInterface, public RenderResource
	{
	public:
		void destroy() override;
		void finishCleanup() override;
		
	private:
	};

	class BlockUpdateInfo
	{
	public:
	};

	struct FILCUpdatePrimTaskData
	{
	};

	struct PrimitiveBounds
	{
		float3 mOrigin;
		float mSphereRadius;
		float3 mBoxExtent;
		float mMinDrawDistanceSq;
		float mMaxDrawDistance;
	};

	struct PrimitiveVisiblilityId
	{
		int32 mByteIndex;
		uint8 mBitMask;
	};

	namespace EOcclusionFlags
	{
		enum Type
		{
			None = 0x0,
			CanBeOccluded = 0x1,
			AllowApproximateOcclusion = 0x4,
			HasPrecomputeVisibility = 0x8,
			HasSubPrimitiveQueries = 0x10,
		};
	}


	class Scene : public SceneInterface
	{
	public:
		Scene(World* inWorld, bool bInRequiresHitProxies, bool bInIsEditorScene, bool bCreateFXSystem, ERHIFeatureLevel::Type inFeatureLevel);

		virtual World* getWorld() const override;

		virtual Scene* getRenderScene() override;

		virtual void addPrimitive(PrimitiveComponent* primitive) override;

		virtual void addLight(LightComponent* light) override;

		template<typename LightMapPolicyType>
		TStaticMeshDrawList<TBasePassDrawingPolicy<LightMapPolicyType>>& getBasePassDrawList(EBasePassDrawListType drawType);

		ConstantBufferRHIParamRef getParameterCollectionBuffer(const Guid& inId) const
		{
			{
				auto r = mParameterCollections.find(inId);
				if (r != mParameterCollections.end())
				{
					return r->second;
				}
				return ConstantBufferRHIParamRef();
			}
		}

		virtual void updateLightColorAndBrightness(LightComponent* light) override;
	
		virtual void updateParameterCollections(const TArray<class MaterialParameterCollectionInstanceResource *>& inParameterCollections) override;
	private:
		void addLightSceneInfo_RenderThread(LightSceneInfo* lightSceneInfo);

		void addPrimitiveSceneInfo_RenderThread(RHICommandListImmediate& RHICmdList, PrimitiveSceneInfo* primitiveSceneInfo);

		void checkPrimitiveArrays();

		virtual void updatePrimitiveTransform(PrimitiveComponent* primitive) override;

		void updatePrimitiveTransform_RenderThread(RHICommandListImmediate& RHICmdList, PrimitiveSceneProxy* primitiveSceneProxy, const BoxSphereBounds& worldBounds, const BoxSphereBounds& localBounds, const Matrix& localToWorld, const float3& ownerPosition);

	public:
		LightSceneInfo* mSimpleDirectionalLight{ nullptr };
		TStaticMeshDrawList<TBasePassDrawingPolicy<ConstantLightMapPolicy>> mBasePassConstantLightMapPolicyDrawList[EBAsePass_MAX];

		bool mAtmosphericFog{ false };

		void* Skylight;

		TSparseArray<StaticMesh*> mStaticMeshes;

		TMap<Guid, ConstantBufferRHIRef> mParameterCollections;

		TArray<PrimitiveSceneInfo*> mPrimitives;
		TArray<PrimitiveBounds> mPrimitiveBounds;
		TArray<PrimitiveVisiblilityId>	mPrimitiveVisibilityIds;
		TArray<uint8>	mPrimitiveOcclusionFlags;
		TArray<BoxSphereBounds> mPrimitiveOcclusionBounds;
		TArray<PrimitiveComponentId> mPrimitiveComponentIds;
		TSparseArray<LightSceneInfoCompact> mLights;
		
		LightSceneInfo* mSunLight;

		SceneLightOctree mLightOctree;

		ScenePrimitiveOctree mPrimitiveOctree;

	protected:
		World* mWorld{ nullptr };

	private:
		int32 mNumVisibleLights_GameThread{ 0 };
		ERHIFeatureLevel::Type mFeatureLevel;
	};
}