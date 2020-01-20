#pragma once

#include "RenderResource.h"
#include "RHIUtilities.h"

namespace Air
{

	struct DynamicReadBufferPool;

	struct DynamicAlloReadBuffer : public DynamicReadBuffer
	{
		int32 mAllocatedByteCount;

		DynamicAlloReadBuffer()
			:DynamicReadBuffer()
			,mAllocatedByteCount(0)
		{

		}

		void unlock()
		{
			DynamicReadBuffer::unlock();
			mAllocatedByteCount = 0;
		}
	};

	class RENDER_CORE_API GlobalDynamicReadBuffer : public RenderResource
	{
	public:
		struct Allocation
		{
			uint8* mBuffer;
			DynamicAlloReadBuffer* mReadBuffer;
			uint32 mFirstIndex;

			Allocation()
				:mBuffer(nullptr)
				,mReadBuffer(nullptr)
				, mFirstIndex(0)
			{}

			FORCEINLINE bool isValid() const
			{
				return mBuffer != nullptr;
			}
		};

		GlobalDynamicReadBuffer();

		~GlobalDynamicReadBuffer();

		Allocation allocateFloat(uint32 num);

		Allocation allocateInt32(uint32 num);

		void commit();

		bool isRenderAlarmLoggingEnabled() const;

	protected:

		virtual void initRHI() override;

		virtual void releaseRHI() override;

		void cleanup();

		DynamicReadBufferPool* mFloatBufferPool;
		DynamicReadBufferPool* mInt32BufferPool;

		size_t mTotalAllocatedSinceLastCommit;
	};
}