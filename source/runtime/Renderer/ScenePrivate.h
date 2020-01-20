#pragma once
#include "RendererMininal.h"
#include "SceneView.h"
#include "RenderingThread.h"
#include "LightMapRendering.h"
#include "LightSceneInfo.h"
#include "SceneInterface.h"
#include "Rendering/SceneRenderTargetParameters.h"
#include "GPUScene.h"
namespace Air
{
	class SceneViewState : public SceneViewStateInterface, public DeferredCleanupInterface, public RenderResource
	{
	public:
		void destroy() override;
		void finishCleanup() override;
		
		inline uint32 getFrameIndex(uint32 pow2Modulus) const
		{
			BOOST_ASSERT(Math::isPowerOfTwo(pow2Modulus));
			return mFrameIndex % (pow2Modulus - 1);
		}

		inline uint32 getFrameIndex() const
		{
			return mFrameIndex;
		}
	public:
		RWBufferStructured mPrimitiveShaderDataBuffer;

	private:
		uint32 mFrameIndex;
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
	class OpaqueBasePassConstantParameters;
	class TranslucentBasePassConstantParameters;

	class PersistentConstantBuffers
	{
	public:
		bool updateViewConstantBuffer(const ViewInfo& view);

		const ViewInfo& getInstancedView(const ViewInfo& view)
		{
			const EStereoscopicPass steredPassIndex = (view.mStereoPass != eSSP_FULL) ? eSSP_RIGHT_EYE : eSSP_FULL;

			return static_cast<const ViewInfo&>(view.mFamily->getSteredEyeView(steredPassIndex));
		}

		TConstantBufferRef<ViewConstantShaderParameters> mViewConstantBuffer;
		TConstantBufferRef<InstancedViewConstantShaderParameters> mInstancedViewConstantBuffer;
		TConstantBufferRef<SceneTextureConstantParameters> mDepthPassConstantBuffer;
		TConstantBufferRef<OpaqueBasePassConstantParameters> mOpaqueBasePassConstantBuffer;
		TConstantBufferRef<TranslucentBasePassConstantParameters> mTranslucentBasePassConstantBuffer;




		const ViewInfo* mCachedView;
	};


	class Scene : public SceneInterface
	{
	public:
		Scene(World* inWorld, bool bInRequiresHitProxies, bool bInIsEditorScene, bool bCreateFXSystem, ERHIFeatureLevel::Type inFeatureLevel);

		virtual World* getWorld() const override;

		virtual Scene* getRenderScene() override;

		virtual void addPrimitive(PrimitiveComponent* primitive) override;

		virtual void addLight(LightComponent* light) override;

		virtual void setSkyLight(SkyLightSceneProxy* light) override;

		virtual void disableSkyLight(SkyLightSceneProxy* light) override;

		virtual void updateSkyCaptureContents(const SkyLightComponent* captureComponent, bool bCaptureEmissiveOnly, std::shared_ptr<RTextureCube> sourceCubemap, Texture* outProcessedTexture, float& outAverageBrightness, SHVectorRGB3& outIrradianceEnvironmentMap) override;

		bool shouldRenderSkyLight(EBlendMode blendMode) const
		{
			return shouldRenderSkyLight_Internal(blendMode);
		}

		bool shouldRenderSkyLight_Internal(EBlendMode blendMode) const
		{
			if (isTranslucentBlendMode(blendMode))
			{
				return mSkyLight && !mSkyLight->bHasStaticLighting;
			}
			else
			{
				const bool bRenderSkyLight = mSkyLight && !mSkyLight->bHasStaticLighting && (mSkyLight->bWantsStaticShadowing || isAnyForwardShadingEnabled(getShaderPlatform()));
				return bRenderSkyLight;
			}
		}

		

		RHIConstantBuffer* getParameterCollectionBuffer(const Guid& inId) const
		{
			{
				auto r = mParameterCollections.find(inId);
				if (r != mParameterCollections.end())
				{
					return r->second;
				}
				return nullptr;
			}
		}

		virtual void updateLightColorAndBrightness(LightComponent* light) override;
	
		virtual void updateParameterCollections(const TArray<class MaterialParameterCollectionInstanceResource *>& inParameterCollections) override;

		bool hasAtmosphericFog()
		{
			return (mAtmosphericFog != nullptr);
		}
	private:
		void addLightSceneInfo_RenderThread(LightSceneInfo* lightSceneInfo);

		void addPrimitiveSceneInfo_RenderThread(RHICommandListImmediate& RHICmdList, PrimitiveSceneInfo* primitiveSceneInfo);

		void checkPrimitiveArrays();

		virtual void updatePrimitiveTransform(PrimitiveComponent* primitive) override;

		void updatePrimitiveTransform_RenderThread(RHICommandListImmediate& RHICmdList, PrimitiveSceneProxy* primitiveSceneProxy, const BoxSphereBounds& worldBounds, const BoxSphereBounds& localBounds, const Matrix& localToWorld, const float3& ownerPosition);

	public:
		LightSceneInfo* mSimpleDirectionalLight{ nullptr };

		void* Skylight;

		TSparseArray<StaticMeshBatch*> mStaticMeshes;

		TSparseArray<bool> mStaticMeshBatchVisibility;

		TMap<Guid, ConstantBufferRHIRef> mParameterCollections;

		TArray<PrimitiveSceneInfo*> mPrimitives;
		TArray<Matrix> mPrimitiveTransforms;
		TArray<PrimitiveBounds> mPrimitiveBounds;
		TArray<PrimitiveFlagsCompact> mPrimitiveFlagsCompact;
		TArray<class PrimitiveSceneProxy*> mPrimitiveSceneProxies;
		TArray<PrimitiveVisiblilityId>	mPrimitiveVisibilityIds;
		TArray<uint8>	mPrimitiveOcclusionFlags;
		TArray<BoxSphereBounds> mPrimitiveOcclusionBounds;
		TArray<PrimitiveComponentId> mPrimitiveComponentIds;
		TSparseArray<LightSceneInfoCompact> mLights;
		TBitArray<> mPrimitivesNeedingStaticMeshUpdate;
		TSet<PrimitiveSceneInfo*> mPrimitivesNeedingStaticMeshUpdateWithoutVisibilityCheck;
		LightSceneInfo* mSunLight{ nullptr };

		class AtmosphericFogSceneInfo* mAtmosphericFog;

		TArray<SkyLightSceneProxy*> mSkyLightStack;

		SkyLightSceneProxy* mSkyLight{ nullptr };

		SceneLightOctree mLightOctree;

		ScenePrimitiveOctree mPrimitiveOctree;

		PersistentConstantBuffers mConstantBuffers;

		bool bScenesPrimitivesNeedStaticMeshElementUpdate{ true };

		bool bPathTracingNeedsInvalidation;

		GPUScene mGPUScene;

		CachedPassMeshDrawList mCachedDrawLists[EMeshPass::Num];

		struct TypeOffsetTableEntry
		{
			TypeOffsetTableEntry(size_t inPrimitiveSceneProxyType, uint32 inOffset)
				: mPrimitiveSceneProxyType(inPrimitiveSceneProxyType)
				, mOffset(inOffset)
			{}

			size_t mPrimitiveSceneProxyType;
			uint32 mOffset;
		};

		TArray<TypeOffsetTableEntry> mTypeOffsetTable;

	protected:
		World* mWorld{ nullptr };

	private:
		int32 mNumVisibleLights_GameThread{ 0 };

		int32 mNumEnabledSkyLights_GameThread{ 0 };
		ERHIFeatureLevel::Type mFeatureLevel;

	};

	
}