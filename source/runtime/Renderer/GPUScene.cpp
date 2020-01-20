#include "GPUScene.h"
#include "RenderUtils.h"
#include "ScenePrivate.h"
#include "PrimitiveConstantShaderParameters.h"
#include "ByteBuffer.h"
namespace Air
{
	int32 GGPUSceneUploadEveryFrame = 0;

	int32 GGPUSceneMaxPooledUploadBufferSize = 256000;

	void updateGPUScene(RHICommandList& RHICmdList, Scene& scene)
	{
		if (useGPUScene(GMaxRHIShaderPlatform, scene.getFeatureLevel()))
		{
			if (GGPUSceneUploadEveryFrame || scene.mGPUScene.bUpdateAllPrimitives)
			{
				for (int32 index : scene.mGPUScene.mPrimitivesToUpdate)
				{
					scene.mGPUScene.mPrimitivesMarkedToUpdate[index] = false;
				}
				scene.mGPUScene.mPrimitivesToUpdate.reset();

				for (int32 i = 0; i < scene.mPrimitives.size(); i++)
				{
					scene.mGPUScene.mPrimitivesToUpdate.add(i);
				}
				scene.mGPUScene.bUpdateAllPrimitives = false;
			}

			bool bResizedPrimitiveData = false;
			bool bResizedLightmapData = false;
			{
				const int32 numPrimitiveEntries = scene.mPrimitives.size();
				const uint32 primitiveSceneNumFloat4s = numPrimitiveEntries * PrimitiveSceneShaderData::PrimitiveDataStrideInFloat4s;
				bResizedPrimitiveData = resizeBufferIfNeeded(RHICmdList, scene.mGPUScene.mPrimitiveBuffer, Math::roundUpToPowerOfTwo(primitiveSceneNumFloat4s));
			}
			{
				/*const int32 numLightmapDataEntries = scene.mGPUScene.mLightmapDataAllocator.getMaxSize();
				const uint32 lightmapDataNumFloat4s = numLightmapDataEntries * LightmapSceneShaderData*/
			}

			const int32 numPrimitiveDataUploads = scene.mGPUScene.mPrimitivesToUpdate.size();

			if (numPrimitiveDataUploads > 0)
			{
				ScatterUploadBuilder primitivesUpdateBuilder(numPrimitiveDataUploads, PrimitiveSceneShaderData::PrimitiveDataStrideInFloat4s, scene.mGPUScene.mPrimitivesUploadScatterBuffer, scene.mGPUScene.mPrimitivesUploadDataBuffer);

				int32 numLightmapDataUploads = 0;

				for (int32 index : scene.mGPUScene.mPrimitivesToUpdate)
				{
					if (index < scene.mPrimitiveSceneProxies.size())
					{
						PrimitiveSceneProxy* primitiveSceneProxy = scene.mPrimitiveSceneProxies[index];
						numLightmapDataUploads += primitiveSceneProxy->getPrimitiveSceneInfo()->getNumLightmapDataEntries();

						PrimitiveSceneShaderData primitiveSceneData(primitiveSceneProxy);
						primitivesUpdateBuilder.add(index, &primitiveSceneData.mData[0]);
					}
					scene.mGPUScene.mPrimitivesMarkedToUpdate[index] = false;
				}
				if (bResizedPrimitiveData)
				{
					RHICmdList.transitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EComputeToCompute, scene.mGPUScene.mPrimitiveBuffer.mUAV);
				}
				else
				{
					RHICmdList.transitionResource(EResourceTransitionAccess::EWritable, EResourceTransitionPipeline::EGfxToCompute, scene.mGPUScene.mPrimitiveBuffer.mUAV);
				}

				primitivesUpdateBuilder.uploadTo_Flush(RHICmdList, scene.mGPUScene.mPrimitiveBuffer);

				RHICmdList.transitionResource(EResourceTransitionAccess::EReadable, EResourceTransitionPipeline::EComputeToGfx, scene.mGPUScene.mPrimitiveBuffer.mUAV);

				//if(ggpusenevalid)
				if (numLightmapDataUploads > 0)
				{
					/*ScatterUploadBuilder lightmapDataUploadBuilder(numLightmapDataUploads, LightmapSceneShaderData)*/
				}

				scene.mGPUScene.mPrimitivesToUpdate.reset();

				if (scene.mGPUScene.mPrimitivesUploadDataBuffer.mNumBytes > (uint32)GGPUSceneMaxPooledUploadBufferSize || scene.mGPUScene.mPrimitivesUploadScatterBuffer.mNumBytes > (uint32)GGPUSceneMaxPooledUploadBufferSize)
				{
					scene.mGPUScene.mPrimitivesUploadDataBuffer.release();
					scene.mGPUScene.mPrimitivesUploadScatterBuffer.release();
				}
			}
		}
		BOOST_ASSERT(scene.mGPUScene.mPrimitivesToUpdate.size() == 0);
	}

	int32 GrowOnlySpanAllocator::allocate(int32 num)
	{
		const int32 foundIndex = searchFreeList(num);
		if (foundIndex != INDEX_NONE)
		{
			LinearAllocation freeSpan = mFreeSpans[foundIndex];
			if (freeSpan.mNum > num)
			{
				mFreeSpans[foundIndex] = LinearAllocation(freeSpan.mStartOffset + num, freeSpan.mNum - num);
			}
			else
			{
				mFreeSpans.removeAtSwap(foundIndex);
			}
			return freeSpan.mStartOffset;
		}

		int32 startOffset = mMaxSize;
		mMaxSize = mMaxSize + num;

		return startOffset;
	}

	int32 GrowOnlySpanAllocator::searchFreeList(int32 num)
	{
		for (int32 i = 0; i < mFreeSpans.size(); i++)
		{
			LinearAllocation currentSpan = mFreeSpans[i];
			if (currentSpan.mNum >= num)
			{
				return i;
			}
		}
		return INDEX_NONE;
	}

	void GrowOnlySpanAllocator::free(int32 baseOffset, int32 num)
	{
		BOOST_ASSERT(baseOffset + num <= mMaxSize);

		LinearAllocation newFreeSpan(baseOffset, num);

		bool bMergedIntoExisting = false;

		int32 spanBeforeIndex = INDEX_NONE;
		int32 spanAfterIndex = INDEX_NONE;

		for (int32 i = 0; i < mFreeSpans.size(); i++)
		{
			LinearAllocation currentSpan = mFreeSpans[i];
			if (currentSpan.mStartOffset == newFreeSpan.mStartOffset + newFreeSpan.mNum)
			{
				spanAfterIndex = i;
			}
			if (currentSpan.mStartOffset + currentSpan.mNum == newFreeSpan.mStartOffset)
			{
				spanBeforeIndex = i;
			}
		}

		if (spanBeforeIndex != INDEX_NONE)
		{
			LinearAllocation& spanBefore = mFreeSpans[spanBeforeIndex];
			spanBefore.mNum += newFreeSpan.mNum;

			if (spanAfterIndex != INDEX_NONE)
			{
				LinearAllocation spanAfter = mFreeSpans[spanAfterIndex];
				spanBefore.mNum += spanAfter.mNum;
				mFreeSpans.removeAtSwap(spanAfterIndex);
			}
		}
		else if (spanAfterIndex != INDEX_NONE)
		{
			LinearAllocation& spanAfter = mFreeSpans[spanAfterIndex];
			spanAfter.mStartOffset = newFreeSpan.mStartOffset;
			spanAfter.mNum += newFreeSpan.mNum;
		}
		else
		{
			mFreeSpans.add(newFreeSpan);
		}
	}

	void addPrimitiveToUpdateGPU(Scene& scene, int32 primitiveId)
	{
		if (useGPUScene(GMaxRHIShaderPlatform, scene.getFeatureLevel()))
		{
			if (primitiveId + 1 > scene.mGPUScene.mPrimitivesMarkedToUpdate.size())
			{
				const int32 newSize = align(primitiveId + 1, 64);
				scene.mGPUScene.mPrimitivesMarkedToUpdate.add(0, newSize - scene.mGPUScene.mPrimitivesMarkedToUpdate.size());
			}

			if (!scene.mGPUScene.mPrimitivesMarkedToUpdate[primitiveId])
			{
				scene.mGPUScene.mPrimitivesToUpdate.add(primitiveId);
				scene.mGPUScene.mPrimitivesMarkedToUpdate[primitiveId] = true;
			}
		}
	}

	void uploadDynamicPrimitiveShaderDataForView(RHICommandList& RHICmdList, Scene& scene, ViewInfo& view)
	{
		if (useGPUScene(GMaxRHIShaderPlatform, scene.getFeatureLevel()))
		{
			const int32 numPrimitiveDataUploads = view.mDynamicPrimitiveShaderData.size();
			if (numPrimitiveDataUploads > 0)
			{
				RWBufferStructured& viewPrimitiveShaderDataBuffer = view.mViewState ? view.mViewState->mPrimitiveShaderDataBuffer : view.mOneFramePrimitiveShaderDataBuffer;
				const int32 numPrimitiveEntries = scene.mPrimitives.size() + view.mDynamicPrimitiveShaderData.size();
				const uint32 primitiveSceneNumFloat4s = numPrimitiveEntries * PrimitiveSceneShaderData::PrimitiveDataStrideInFloat4s;

				uint32 viewPrimitiveSceneNumFloat4s = Math::roundUpToPowerOfTwo(primitiveSceneNumFloat4s);
				uint32 bytesPerElement = GPixelFormats[PF_A32B32G32R32F].BlockBytes;

				if (viewPrimitiveSceneNumFloat4s * bytesPerElement != viewPrimitiveShaderDataBuffer.mNumBytes)
				{
					viewPrimitiveShaderDataBuffer.release();
					viewPrimitiveShaderDataBuffer.initialize(bytesPerElement, viewPrimitiveSceneNumFloat4s, 0, TEXT("ViewPrimitiveShaderDataBuffer"));
				}

				memcpyBuffer(RHICmdList, scene.mGPUScene.mPrimitiveBuffer, viewPrimitiveShaderDataBuffer, scene.mPrimitives.size() * PrimitiveSceneShaderData::PrimitiveDataStrideInFloat4s);

				if (numPrimitiveDataUploads > 0)
				{
					ScatterUploadBuilder primitivesUploadBuilder(numPrimitiveDataUploads, PrimitiveSceneShaderData::PrimitiveDataStrideInFloat4s, scene.mGPUScene.mPrimitivesUploadScatterBuffer, scene.mGPUScene.mPrimitivesUploadDataBuffer);

					for (int32 dynamicUploadIndex = 0; dynamicUploadIndex < view.mDynamicPrimitiveShaderData.size(); dynamicUploadIndex++)
					{
						PrimitiveSceneShaderData primitiveSceneData(view.mDynamicPrimitiveShaderData[dynamicUploadIndex]);
						primitivesUploadBuilder.add(scene.mPrimitives.size() + dynamicUploadIndex, &primitiveSceneData.mData[0]);
					}
					RHICmdList.transitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EComputeToCompute, viewPrimitiveShaderDataBuffer.mUAV);
					primitivesUploadBuilder.uploadTo(RHICmdList, viewPrimitiveShaderDataBuffer);
				}
				RHICmdList.transitionResource(EResourceTransitionAccess::EReadable, EResourceTransitionPipeline::EComputeToGfx, viewPrimitiveShaderDataBuffer.mUAV);
				view.mCachedViewConstantShaderParameters->PrimitiveSceneData = viewPrimitiveShaderDataBuffer.mSRV;
			}
			else
			{
				view.mCachedViewConstantShaderParameters->PrimitiveSceneData = scene.mGPUScene.mPrimitiveBuffer.mSRV;
			}
			view.mViewConstantBuffer.updateConstantBufferImmediate(*view.mCachedViewConstantShaderParameters);
		}
	}
}