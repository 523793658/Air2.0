#pragma once
#include "CoreMinimal.h"
#include "RHIUtilities.h"
#include "RHICommandList.h"
#include "Containers/DynamicRHIResourceArray.h"
#include "RenderCore.h"

namespace Air
{

	template<typename ResourceType>
	extern RENDER_CORE_API void memcpyResource(RHICommandList& RHICmdList, const ResourceType& dstBuffer, const ResourceType& srcBuffer, uint32 numBytes, uint32 dstOffset = 0, uint32 srcOffset = 0);

	template<typename ResourceType>
	extern RENDER_CORE_API bool resizeResourceIfNeeded(RHICommandList& RHICmdList, ResourceType& texture, uint32 numBytes, const TCHAR* debugName);

	class ScatterUploadBuffer
	{
	public:
		enum {PrimitiveDataStrideInFloat4s = 36};
		ByteAddressBuffer mScatterBuffer;
		ByteAddressBuffer mUploadBuffer;

		uint32* mScatterData = nullptr;
		uint8* mUploadData = nullptr;

		uint32 mNumScatters = 0;
		uint32 mMaxScatters = 0;

		uint32 mNumScattersAllocated = 0;

		uint32 mNumBytesPerElement = 0;

		bool bFloat4Buffer = false;

		RENDER_CORE_API void init(uint32 numElements, uint32 inNumBytesElement, bool bInFloat4Buffer, const TCHAR* debugName);

		template<typename ResourceType>
		RENDER_CORE_API void ResourceUploadTo(RHICommandList& RHICmdList, ResourceType& dstBuffer, bool bFlush = false);

		void* add_GetRef(uint32 index, uint32 num = 1)
		{
			BOOST_ASSERT(mNumScatters + num <= mMaxScatters);
			BOOST_ASSERT(mScatterData != nullptr);
			BOOST_ASSERT(mUploadData != nullptr);
			for (uint32 i = 0; i < num; ++i)
			{
				mScatterData[i] = index + i;
			}

			void* result = mUploadData;
			mScatterData += num;
			mUploadData += num * mNumBytesPerElement;
			mNumScatters += num;
			return result;
		}

		void add(uint32 index, const void* data, uint32 num = 1)
		{
			void* dst = add_GetRef(index, num);
			Memory::memcpy(dst, data, num * mNumBytesPerElement);
		}

		void release()
		{
			mScatterBuffer.release();
			mUploadBuffer.release();
		}

		uint32 getNumBytes() const
		{
			return mScatterBuffer.mNumBytes + mUploadBuffer.mNumBytes;
		}
	};
}