#pragma once
#include "RHIUtilities.h"
#include "Containers/BitArray.h"
namespace Air
{
	class Scene;
	class ViewInfo;

	class LinearAllocation
	{
	public:
		int32 mStartOffset;
		int32 mNum;

		LinearAllocation(int32 inStartOffset, int32 inNum)
			:mStartOffset(inStartOffset)
			,mNum(inNum)
		{

		}

		bool contains(LinearAllocation other)
		{
			return mStartOffset <= other.mStartOffset && (mStartOffset + mNum) >= (other.mStartOffset + other.mNum);
		}
	};

	class GrowOnlySpanAllocator
	{
	public:
		GrowOnlySpanAllocator()
			:mMaxSize(0)
		{}

		int32 allocate(int32 num);

		void free(int32 baseOffset, int32 num);

		int32 getMaxSize() const
		{
			return mMaxSize;
		}


	private:
		int32 mMaxSize;
		TArray<LinearAllocation, TInlineAllocator<10>> mFreeSpans;

		int32 searchFreeList(int32 num);
	};


	class GPUScene
	{
	public:
		ReadBuffer mPrimitivesUploadScatterBuffer;
		ReadBuffer mPrimitivesUploadDataBuffer;

		bool bUpdateAllPrimitives;
		TArray<int32> mPrimitivesToUpdate;
		TBitArray<> mPrimitivesMarkedToUpdate;

		RWBufferStructured mPrimitiveBuffer;

		GrowOnlySpanAllocator mLightmapDataAllocator;

		ReadBuffer mLightmapUploadScatterBuffer;
		ReadBuffer mLightmapUploadDataBuffer;
		RWBufferStructured mLightmapDataBuffer;
	};

	extern void updateGPUScene(RHICommandList& RHICmdList, Scene& scene);
	extern void addPrimitiveToUpdateGPU(Scene& scene, int32 primitiveId);
	extern void uploadDynamicPrimitiveShaderDataForView(RHICommandList& RHICmdList, Scene& scene, ViewInfo& view);
}