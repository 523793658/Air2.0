#include "GPUScene.h"
#include "RenderUtils.h"
#include "ScenePrivate.h"
#include "PrimitiveConstantShaderParameters.h"
#include "ByteBuffer.h"
#include "UnifiedBuffer.h"
namespace Air
{
	int32 GGPUSceneUploadEveryFrame = 0;

	int32 GGPUSceneMaxPooledUploadBufferSize = 256000;

	template<typename ResourceType>
	ResourceType* getMirrorGPU(Scene& scene);

	template<>
	RWBufferStructured* getMirrorGPU<RWBufferStructured>(Scene& scene)
	{
		return &scene.mGPUScene.mPrimitiveBuffer;
	}

	template<>
	TextureRWBuffer2D* getMirrorGPU<TextureRWBuffer2D>(Scene& scene)
	{
		return &scene.mGPUScene.mPrimitiveTexture;
	}

	static int32 getMaxPrimitivesUpdate(uint32 numUploads, uint32 inStrideInFloat4s)
	{
		return Math::min((uint32)(GetMaxBufferDimension() / inStrideInFloat4s), numUploads);
	}

	template<typename ResourceType>
	void UpdateGPUSceneInternal(RHICommandListImmediate& RHICmdList, Scene& scene)
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

			ResourceType* mirrorResourceGPU = getMirrorGPU<ResourceType>(scene);
			{
				const uint32 sizeReserve = Math::roundUpToPowerOfTwo(Math::max(scene.mPrimitives.size(), 256));
				bResizedPrimitiveData = resizeResourceIfNeeded(RHICmdList, *mirrorResourceGPU, sizeReserve * sizeof(PrimitiveSceneShaderData::mData), TEXT("PrimitiveData"));
			}
			{
				/*const uint32 SizeReserve = Math::roundUpToPowerOfTwo(Math::max(scene.mGPUScene.mLightmapDataAllocator.getMaxSize(), 256));
				bResizedLightmapData = resizeResourceIfNeeded(RHICmdList, scene.mGPUScene.mLightmapDataBuffer, SizeReserve * sizeof(lightmapsceneshaderda))*/
			}
			const int32 numPrimitiveDataUploads = scene.mGPUScene.mPrimitivesToUpdate.size();
			int32 numLightmapDataUploads = 0;
			if (numPrimitiveDataUploads > 0)
			{
				const int32 maxPrimitivesUploads = getMaxPrimitivesUpdate(numPrimitiveDataUploads, PrimitiveSceneShaderData::PrimitiveDataStrideInFloat4s);

				for (int primitiveOffset = 0; primitiveOffset < numPrimitiveDataUploads; primitiveOffset += maxPrimitivesUploads)
				{
					scene.mGPUScene.mPrimititveUploadbuff
				}
			}
		}
	}




	void updateGPUScene(RHICommandListImmediate& RHICmdList, Scene& scene)
	{
		if (GPUSceneUseTexture2D(scene.getShaderPlatform()))
		{
			UpdateGPUSceneInternal<TextureRWBuffer2D>(RHICmdList, scene);
		}
		else
		{
			UpdateGPUSceneInternal<RWBufferStructured>(RHICmdList, scene);
		}
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