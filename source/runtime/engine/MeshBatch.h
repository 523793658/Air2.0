#pragma once
#include "CoreMinimal.h"
#include "PrimitiveConstantShaderParameters.h"
#include "RenderResource.h"
#include "scene.h"
#include "MaterialShared.h"
#include "HitProxies.h"
#include "VertexFactory.h"
namespace Air
{
	class LightCacheInterface;


	struct MeshBatchElement
	{
		const TConstantBuffer<PrimitiveConstantShaderParameters>* mPrimitiveConstantBufferResource;
		RHIConstantBuffer* mPrimitiveConstantBuffer;


		void* mVertexFactoryUserData;

		const IndexBuffer* mIndexBuffer;

		union
		{
			uint32* mInstantceRuns;
			class SplineMeshSceneProxy* mSplineMeshSceneProxy;
		};

		const void* mUserData{ nullptr };

		const void* mDynamicIndexData{ nullptr };
		uint32 mFirstIndex{ 0 };
		uint32 mNumPrimitives{ 0 };
		uint32 mNumInstances{ 0 };
		uint32 mMinVertexIndex{ 0 };
		uint32 mMaxVertexIndex{ 0 };

		int32 mUserIndex;
		float mMinScreenSize;
		float mMaxScreenSize;
		uint16 mDynamicIndexStride;
		uint8 mInstancedLODIndex : 4;
		uint8 mInstancedLODRange : 4;
		uint8 bUserDataIsColorVertexBuffer : 1;
		uint8 bIsInstancedMesh : 1;
		uint8 bIsSplineProxy : 1;
		uint8 bIsInstanceRuns : 1;
		uint8 mVisualizeElementIndex;

		MeshBatchElement()
			:mPrimitiveConstantBufferResource(nullptr)
			, mIndexBuffer(nullptr)
			, mInstantceRuns(nullptr)
			, mUserData(nullptr)
			, mDynamicIndexData(nullptr)
			, mNumInstances(1)
			, mUserIndex(-1)
			, mMinScreenSize(0.0f)
			, mMaxScreenSize(1.0f)
			, mInstancedLODIndex(0)
			, mInstancedLODRange(0)
			, bUserDataIsColorVertexBuffer(false)
			, bIsInstancedMesh(false)
			, bIsSplineProxy(false)
			, bIsInstanceRuns(false)
			, mVisualizeElementIndex(INDEX_NONE)
		{

		}
	};


	struct MeshBatch
	{
		TArray <MeshBatchElement, TInlineAllocator<1>> mElements;
		uint16 mDynamicVertexStride{ 0 };
		int8 mLODIndex{ INDEX_NONE };
		int8 mVisualizeLODIndex{ INDEX_NONE };
		uint8 mSegmentIndex;
		uint32 mMinVertexIndex;
		uint32 mMaxVertexIndex;

		uint32 bUseDynamicData : 1;
		uint32 bReverseCulling : 1;
		uint32 bDisableBackfaceCulling : 1;
		uint32 bCastShadow : 1;
#if RHI_RAYTRACING
		uint32 bCastRayTracedShadow : 1;
#endif
		uint32 bUseForMaterial : 1;
		uint32 bUseAsOccluder : 1;
		uint32 bWireframe : 1;
		uint32 bRequiresPerElementVisibility : 1;
		uint32 mType : PT_NumBits;
		uint32 mDepthPriorityGroup : SDPG_NumBits;
		uint32 bCanApplyViewModeOverrides : 1;
		uint32 bUseWireframeSelectionColoring : 1;
		uint32 bUseSelectionOutline : 1;
		uint32 bDitheredLODTransition : 1;
		uint32 bSelectable : 1;
		uint32 bUseForDepthPass : 1;
		float	DitheredLODTransitionAlpha{ 0.0f };
		HitProxyId mBatchHitPrixyId;


		const void* mDynamicVertexData = nullptr;

		const MaterialRenderProxy* mMaterialRenderProxy = nullptr;
		const VertexFactory* mVertexFactory = nullptr;
		const LightCacheInterface* LCI = nullptr;

		FORCEINLINE bool isMasked(ERHIFeatureLevel::Type inFeatureLevel) const
		{
			return mMaterialRenderProxy->getMaterial(inFeatureLevel)->isMasked();
		}

		FORCEINLINE void checkConstantBuffers() const
		{
			for (int32 elementIdx = 0; elementIdx < mElements.size(); elementIdx++)
			{
				BOOST_ASSERT(isValidRef(mElements[elementIdx].mPrimitiveConstantBuffer) || mElements[elementIdx].mPrimitiveConstantBufferResource != nullptr);
			}
		}

		FORCEINLINE bool isTranslucent(ERHIFeatureLevel::Type inFeatureLevel) const
		{
			return isTranslucentBlendMode(mMaterialRenderProxy->getMaterial(inFeatureLevel)->getBlendMode());
		}

		FORCEINLINE int32 getNumPrimitives() const
		{
			int32 count = 0;
			for (int32 elementIndex = 0; elementIndex < mElements.size(); elementIndex++)
			{
				if (mElements[elementIndex].bIsInstanceRuns && mElements[elementIndex].mInstantceRuns)
				{
					for (uint32 run = 0; run < mElements[elementIndex].mNumInstances; run++)
					{
						count += mElements[elementIndex].mNumPrimitives * (mElements[elementIndex].mInstantceRuns[run * 2 + 1] - mElements[elementIndex].mInstantceRuns[run * 2] + 1);
					}
				}
				else
				{
					const MeshBatchElement &element = mElements[elementIndex];
					count += mElements[elementIndex].mNumPrimitives * mElements[elementIndex].mNumInstances;
				}
			}
			return count;
		}

		MeshBatch()
			:bUseDynamicData(false),
			bReverseCulling(false),
			bDisableBackfaceCulling(false),
			bCastShadow(true),
			bUseForMaterial(true),
			bUseAsOccluder(false),
			bWireframe(false),
			bCanApplyViewModeOverrides(false),
			bUseWireframeSelectionColoring(false),
			bUseSelectionOutline(false),
			bDitheredLODTransition(false)
		{
			new(mElements)MeshBatchElement();
		}

		ENGINE_API void preparePrimitiveConstantBuffer(const PrimitiveSceneProxy* primitiveSceneProxy, ERHIFeatureLevel::Type featureLevel);
	};
}