#pragma once
#include "CoreMinimal.h"
#include "RHI.h"
#include "RHIUtilities.h"
namespace Air
{
	void memcpyBuffer(RHICommandList& RHICmdList, const RWBufferStructured& srcBuffer, const RWBufferStructured& dstBuffer, uint32 numFloat4s, uint2 srcOffset = 0, uint2 dstOffset = 0);

	void memsetBuffer(RHICommandList& RHICmdList, const RWBufferStructured& dstBuffer, const float4& value, uint32 numFloat4s, uint32 dstOffsetInFloat4s);

	class RHICommandList;

	bool resizeBufferIfNeeded(RHICommandList& RHICmdList, RWBufferStructured& buffer, uint32 numFloat4s);

	class ScatterUploadBuilder
	{
	public:
		ReadBuffer& mScatterBuffer;
		ReadBuffer& mUploadBuffer;

		uint32* mScatterData;
		float4* mUploadData;

		uint32 mAllocatedNumScatters;
		uint32 mNumScatters;
		uint32 mStrideInFloat4s;

	public:
		ScatterUploadBuilder(uint32 numUploads, uint32 inStrideInFloat4s, ReadBuffer& inScatterBuffer, ReadBuffer& inUploadBuffer);

		void uploadTo(RHICommandList& RHICmdList, RWBufferStructured& dstBuffer);

		void uploadTo_Flush(RHICommandList& RHICmdList, RWBufferStructured& dstBuffer);

		void add(uint32 index, const float4* data)
		{
			BOOST_ASSERT(mNumScatters < mAllocatedNumScatters);
			BOOST_ASSERT(mScatterData != nullptr);
			BOOST_ASSERT(mUploadData != nullptr);

			for (int32 i = 0; i < mStrideInFloat4s; i++)
			{
				mScatterData[i] = index * mStrideInFloat4s + i;
				mUploadData[i] = data[i];
			}

			mScatterData += mStrideInFloat4s;
			mUploadData += mStrideInFloat4s;
			mNumScatters += mStrideInFloat4s;
		}
	};
}