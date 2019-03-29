#include "HAL/PlatformMisc.h"
#include "SceneUtils.h"
namespace Air
{
#define PRE_MESH_DRAW_STATS	0

	template<typename DrawingPolicyType>
	void TStaticMeshDrawList<DrawingPolicyType>::addMesh(StaticMesh* mesh, const ElementPolicyDataType& policyData, const DrawingPolicyType& inDrawingPolicy, ERHIFeatureLevel::Type inFeatureLevel)
	{
		BOOST_ASSERT(!GDrawListsLocked);
		DrawingPolicyLink* drawingPolicyLink = mDrawingPolicySet.find(inDrawingPolicy);
		if (!drawingPolicyLink)
		{
			const SetElementId drawingPolicyLinkId = mDrawingPolicySet.add(DrawingPolicyLink(this, inDrawingPolicy, inFeatureLevel));
			drawingPolicyLink = &mDrawingPolicySet[drawingPolicyLinkId];
			drawingPolicyLink->mSetId = drawingPolicyLinkId;


			mTotalBytesUsed += drawingPolicyLink->getSizeBytes();
			int32 minIndex = 0;
			int32 maxIndex = mOrderedDrawingPolicies.size() - 1;
			while (minIndex < maxIndex)
			{
				int32 pivotIndex = (maxIndex + minIndex) / 2;
				int32 compareResult = compareDrawingPolicy(mDrawingPolicySet[mOrderedDrawingPolicies[pivotIndex]].mDrawingPolicy, drawingPolicyLink->mDrawingPolicy);
				if (compareResult < 0)
				{
					minIndex = pivotIndex + 1;
				}
				else if (compareResult > 0)
				{
					minIndex = pivotIndex;
				}
				else
				{
					minIndex = maxIndex = pivotIndex;
				}
			}
			BOOST_ASSERT(minIndex >= maxIndex);
			mOrderedDrawingPolicies.insert(drawingPolicyLinkId, minIndex);
		}
		const int32 elementIndex = drawingPolicyLink->mElements.size();
		const SIZE_T previousElementsSize = drawingPolicyLink->mElements.getAllocatedSize();
		const SIZE_T previousCompactElementsSize = drawingPolicyLink->mCompactElements.getAllocatedSize();
	
		Element* element = new (drawingPolicyLink->mElements)Element(mesh, policyData, this, drawingPolicyLink->mSetId, elementIndex);
		new(drawingPolicyLink->mCompactElements)ElementCompact(mesh->mId);
		mTotalBytesUsed += drawingPolicyLink->mElements.getAllocatedSize() - previousElementsSize + drawingPolicyLink->mCompactElements.getAllocatedSize() - previousCompactElementsSize;
		mesh->linkDrawList(element->mHandle);
		BOOST_ASSERT(element->mMesh->mMaterialRenderProxy);
		element->mMesh->mMaterialRenderProxy->setReferencedInDrawList();
	}

	template<typename DrawingPolicyType>
	bool TStaticMeshDrawList<DrawingPolicyType>::drawVisible(RHICommandList& RHICmdList, const ViewInfo& view, const typename DrawingPolicyType::ContextDataType policyContext, const DrawingPolicyRenderState& drawRenderState, const TBitArray<SceneRenderingBitArrayAllocator>& staticMeshVisibilityMap, const TArray<uint64>& batchVisibilityArray)
	{
		DrawingPolicyRenderState drawRenderStateLocal(&RHICmdList, drawRenderState);
		return drawVisibleInner<InstancedStereoPolicy::Disabled>(RHICmdList, view, policyContext, drawRenderStateLocal, &staticMeshVisibilityMap, &batchVisibilityArray, 0, mOrderedDrawingPolicies.size() - 1, false);
	}

	template<typename DrawingPolicyType>
	template<InstancedStereoPolicy instancedStereo>
	bool TStaticMeshDrawList<DrawingPolicyType>::drawVisibleInner(RHICommandList& rhiCmdList, const ViewInfo& view, const typename DrawingPolicyType::ContextDataType policyContext, DrawingPolicyRenderState& drawRenderState, const TBitArray<SceneRenderingBitArrayAllocator>* const staticMeshVisibilityMap, const TArray<uint64>* const batchVisibilityArray, int32 firstPolicy, int32 lastPolicy, bool bUpdateCounts)
	{
		BOOST_ASSERT(staticMeshVisibilityMap != nullptr && batchVisibilityArray != nullptr);
		BOOST_ASSERT((instancedStereo != InstancedStereoPolicy::Disabled) == false);
		bool bDirty = false;
		for (int32 index = firstPolicy; index <= lastPolicy; index++)
		{
			DrawingPolicyLink* drawingPolicyLink = &mDrawingPolicySet[mOrderedDrawingPolicies[index]];
			bool bDrawnShared = false;
			PlatformMisc::prefetch(drawingPolicyLink->mCompactElements.data());
			const int32 numElements = drawingPolicyLink->mElements.size();
			PlatformMisc::prefetch(&drawingPolicyLink->mCompactElements.data()->mMeshId);
			const ElementCompact* compactElementPtr = drawingPolicyLink->mCompactElements.data();
			uint32 count = 0;
			for (int32 elementIndex = 0; elementIndex < numElements; elementIndex++, compactElementPtr++)
			{
				if (instancedStereo == InstancedStereoPolicy::Disabled)
				{
					if (staticMeshVisibilityMap->accessCorrespondingBit(RelativeBitReference(compactElementPtr->mMeshId)))
					{
						const Element& element = drawingPolicyLink->mElements[elementIndex];
						int32 subCount = element.mMesh->mElements.size();
						uint64 batchElementMask = element.mMesh->bRequiresPerElementVisibility ? (*batchVisibilityArray)[element.mMesh->mId] : ((1ull << subCount) - 1);
						count += drawElement<InstancedStereoPolicy::Disabled>(rhiCmdList, view, policyContext, drawRenderState, element, batchElementMask, drawingPolicyLink, bDrawnShared);
					}
				}
				else
				{
					const TArray<uint64>* resolvedVisibilityArray = nullptr;

				}
			}
			bDirty = bDirty || !!count;
			if (bUpdateCounts)
			{
				drawingPolicyLink->mVisibleCount = count;
			}
		}
		return bDirty;
	}

	template<typename DrawingPolicyType>
	void TStaticMeshDrawList<DrawingPolicyType>::ElementHandle::remove(const bool bUnlinkMesh /* = true */)
	{
		BOOST_ASSERT(!GDrawListsLocked);
		TStaticMeshDrawList* const localDrawList = mStaticMeshDrawList;
		DrawingPolicyLink* const localDrawingPolicyLink = &localDrawList->mDrawingPolicySet[mSetId];
		const int32 localElementIndex = mElementIndex;
		BOOST_ASSERT(localDrawingPolicyLink->mSetId == mSetId);
		BOOST_ASSERT(localDrawingPolicyLink->mElements[mElementIndex].mMesh->mMaterialRenderProxy);
		localDrawingPolicyLink->mElements[mElementIndex].mMesh->mMaterialRenderProxy->setUnreferencedInDrawList();
		if (bUnlinkMesh)
		{
			localDrawingPolicyLink->mElements[mElementIndex].mMesh->unlinkDrawList(this);
		}
		localDrawingPolicyLink->mElements[mElementIndex].mMesh = nullptr;
		BOOST_ASSERT(localDrawingPolicyLink->mElements.size() == localDrawingPolicyLink->mCompactElements.size());
		const uint32 lastDrawingPolicySize = localDrawingPolicyLink->getSizeBytes();
		localDrawingPolicyLink->mElements.removeAtSwap(localElementIndex);
		localDrawingPolicyLink->mCompactElements.removeAtSwap(localElementIndex);

		const uint32 currentDrawingPolicySize = localDrawingPolicyLink->getSizeBytes();
		const uint32 drawingPolicySizeDiff = lastDrawingPolicySize - currentDrawingPolicySize;
		localDrawList->mTotalBytesUsed -= drawingPolicySizeDiff;
		if (localElementIndex < localDrawingPolicyLink->mElements.size())
		{
			localDrawingPolicyLink->mElements[localElementIndex].mHandle->mElementIndex = localElementIndex;
		}
		if (!localDrawingPolicyLink->mElements.size())
		{
			localDrawList->mTotalBytesUsed -= localDrawingPolicyLink->getSizeBytes();
			localDrawList->mOrderedDrawingPolicies.removeSingle(localDrawingPolicyLink->mSetId);
			localDrawList->mDrawingPolicySet.remove(localDrawingPolicyLink->mSetId);
		}
	}

	template<typename DrawingPolicyType>
	template<InstancedStereoPolicy InstancedStereo>
	int32 TStaticMeshDrawList<DrawingPolicyType>::drawElement(RHICommandList& RHICmdList, const ViewInfo& view, const typename DrawingPolicyType::ContextDataType policyContext, DrawingPolicyRenderState& drawRenderState, const Element& element, uint64 batchElementMask, DrawingPolicyLink* drawingPolicyLink, bool bDrawnShared)
	{
#if PRE_MESH_DRAW_STATS

#endif
		if (!bDrawnShared)
		{
			if (isValidRef(drawingPolicyLink->mBoundShaderState))
			{
				RHICmdList.setBoundShaderState(drawingPolicyLink->mBoundShaderState);
			}
			else
			{
				RHICmdList.buildAndSetLocalBoundShaderState(drawingPolicyLink->mDrawingPolicy.getBoundShaderStateInput(view.getFeatureLevel()));
			}
			drawingPolicyLink->mDrawingPolicy.setSharedState(RHICmdList, &view, policyContext, drawRenderState);
			bDrawnShared = true;
		}
		DrawingPolicyRenderState drawRenderStateLocal(&RHICmdList, drawRenderState);
		drawingPolicyLink->mDrawingPolicy.applyDitheredLODTransitionState(RHICmdList, drawRenderStateLocal, view, *element.mMesh, view.bAllowStencilDither);
		int32 drawCount = 0;
		int32 batchElementIndex = 0;
		do 
		{
			if (batchElementMask & 1)
			{
				const PrimitiveSceneProxy* proxy = element.mMesh->mPrimitiveSceneInfo->mProxy;
				if (InstancedStereo == InstancedStereoPolicy::Enabled)
				{

				}
				else
				{
					drawCount++;
					TDrawEvent<RHICommandList> meshEvent;
					beginMeshDrawEvent(RHICmdList, proxy, *element.mMesh, meshEvent);
					drawingPolicyLink->mDrawingPolicy.setMeshRenderState(
						RHICmdList,
						view,
						proxy,
						*element.mMesh,
						batchElementIndex,
						drawRenderStateLocal,
						element.mPolicyData,
						policyContext
					);
					drawingPolicyLink->mDrawingPolicy.drawMesh(RHICmdList, *element.mMesh, batchElementIndex, false);
				}
			}
			batchElementMask >>= 1;
			batchElementIndex++;
		} while (batchElementMask);
		return drawCount;
	}
}