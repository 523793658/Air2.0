#include "DynamicBufferAllocator.h"
#include "Containers/IndirectArray.h"
#include <Misc\ScopeLock.h>
namespace Air
{

	int32 GMaxReadBufferRenderingBytesAllocatedPerFrame = 32 * 1024 * 1024;

	int32 GAlignReadBufferRenderingBufferSize = 64 * 1024;

	int32 GMinReadBufferRenderingBufferSize = 256 * 1024;

	struct DynamicReadBufferPool
	{
		TindirectArray<DynamicAlloReadBuffer> mBuffers;
		DynamicAlloReadBuffer* mCurrentBuffer;

		DynamicReadBufferPool()
			:mCurrentBuffer(nullptr)
		{}

		~DynamicReadBufferPool()
		{
			int32 numVertexBuffers = mBuffers.size();
			for (int32 bufferIndex = 0; bufferIndex < numVertexBuffers; ++bufferIndex)
			{
				mBuffers[bufferIndex].release();
			}
		}

		CriticalSection mCriticalSection;
	};

	GlobalDynamicReadBuffer::GlobalDynamicReadBuffer()
		:mTotalAllocatedSinceLastCommit(0)
	{
		mFloatBufferPool = new DynamicReadBufferPool();
		mInt32BufferPool = new DynamicReadBufferPool();
	}

	GlobalDynamicReadBuffer::~GlobalDynamicReadBuffer()
	{
		cleanup();
	}

	void GlobalDynamicReadBuffer::cleanup()
	{
		if (mFloatBufferPool)
		{
			AIR_LOG(LogRendererCore, Log, TEXT("GlobalDynamicReadBuffer::cleanup()"));
			delete mFloatBufferPool;
			mFloatBufferPool = nullptr;
		}

		if (mInt32BufferPool)
		{
			delete mInt32BufferPool;
			mInt32BufferPool = nullptr;
		}
	}


	void GlobalDynamicReadBuffer::initRHI()
	{
		AIR_LOG(LogRendererCore, Verbose, TEXT("GlobalReadBuffer::initRHI"));
	}

	void GlobalDynamicReadBuffer::releaseRHI()
	{
		AIR_LOG(LogRendererCore, Verbose, TEXT("GlobalReadBuffer::releaseRHI"));
		cleanup();
	}

	GlobalDynamicReadBuffer::Allocation GlobalDynamicReadBuffer::allocateFloat(uint32 num)
	{
		ScopeLock scopeLock(&mFloatBufferPool->mCriticalSection);
		Allocation allocation;

		mTotalAllocatedSinceLastCommit += num;
		if (isRenderAlarmLoggingEnabled())
		{
			AIR_LOG(LogRendererCore, Warning, TEXT("GlobalReadBuffer::AllocateFloat(%u), will have allocated %u total this frame"), num, mTotalAllocatedSinceLastCommit);
		}
		uint32 sizeInBytes = sizeof(float) * num;
		DynamicAlloReadBuffer* buffer = mFloatBufferPool->mCurrentBuffer;
		if (buffer == nullptr || buffer->mAllocatedByteCount + sizeInBytes > buffer->mNumBytes)
		{
			buffer = nullptr;
			for (int32 bufferIndex = 0, numBuffers = mFloatBufferPool->mBuffers.size(); bufferIndex < numBuffers; ++bufferIndex)
			{
				DynamicAlloReadBuffer& bufferToCheck = mFloatBufferPool->mBuffers[bufferIndex];
				if (bufferToCheck.mAllocatedByteCount + sizeInBytes <= bufferToCheck.mNumBytes)
				{
					buffer = &bufferToCheck;
					break;
				}
			}
			if (buffer == nullptr)
			{
				const uint32 alignedNum = Math::divideAndRoundUp(num, (uint32)GAlignReadBufferRenderingBufferSize) * GAlignReadBufferRenderingBufferSize;
				const uint64 newBufferSize = Math::max(alignedNum, (uint32)GMinReadBufferRenderingBufferSize);
				buffer = new DynamicAlloReadBuffer();
				mFloatBufferPool->mBuffers.add(buffer);
				buffer->initialize(sizeof(float), newBufferSize, PF_R32_FLOAT, BUF_Dynamic);
			}

			if (buffer->mMappedBuffer == nullptr)
			{
				buffer->lock();
			}

			mFloatBufferPool->mCurrentBuffer = buffer;
		}

		BOOST_ASSERT(buffer != nullptr);
		BOOST_ASSERT(buffer->mAllocatedByteCount + sizeInBytes <= buffer->mNumBytes);
		allocation.mBuffer = buffer->mMappedBuffer + buffer->mAllocatedByteCount;
		allocation.mReadBuffer = buffer;
		allocation.mFirstIndex = buffer->mAllocatedByteCount;
		buffer->mAllocatedByteCount += sizeInBytes;
		return allocation;
	}

	GlobalDynamicReadBuffer::Allocation GlobalDynamicReadBuffer::allocateInt32(uint32 num)
	{
		ScopeLock scopeLock(&mInt32BufferPool->mCriticalSection);
		Allocation allocation;

		mTotalAllocatedSinceLastCommit += num;
		if (isRenderAlarmLoggingEnabled())
		{
			AIR_LOG(LogRendererCore, Warning, TEXT("GlobalReadBuffer::AllocatInt32(%u), will have allocated %u total this frame"), num, mTotalAllocatedSinceLastCommit);
		}

		uint32 sizeInBytes = sizeof(int32) * num;
		DynamicAlloReadBuffer* buffer = mInt32BufferPool->mCurrentBuffer;
		if (buffer == nullptr || buffer->mAllocatedByteCount + sizeInBytes > buffer->mNumBytes)
		{
			buffer = nullptr;
			for (int32 bufferIndex = 0, numBuffers = mInt32BufferPool->mBuffers.size(); bufferIndex < numBuffers; ++bufferIndex)
			{
				DynamicAlloReadBuffer& bufferToCkeck = mInt32BufferPool->mBuffers[bufferIndex];
				if (bufferToCkeck.mAllocatedByteCount + sizeInBytes <= bufferToCkeck.mNumBytes)
				{
					buffer = &bufferToCkeck;
					break;
				}
			}
			if (buffer == nullptr)
			{
				const uint32 alignedNum = Math::divideAndRoundUp(num, (uint32)GAlignReadBufferRenderingBufferSize) * GAlignReadBufferRenderingBufferSize;
				const uint32 newBufferSize = Math::max(alignedNum, (uint32)GMinReadBufferRenderingBufferSize);
				buffer = new DynamicAlloReadBuffer();
				mInt32BufferPool->mBuffers.add(buffer);
				buffer->initialize(sizeof(int32), newBufferSize, PF_R32_SINT, BUF_Dynamic);
			}
			if (buffer->mMappedBuffer == nullptr)
			{
				buffer->lock();
			}

			mInt32BufferPool->mCurrentBuffer = buffer;
		}

		BOOST_ASSERT(buffer != nullptr);
		BOOST_ASSERT(buffer->mAllocatedByteCount + sizeInBytes <= buffer->mNumBytes);

		allocation.mBuffer = buffer->mMappedBuffer + buffer->mAllocatedByteCount;
		allocation.mReadBuffer = buffer;
		allocation.mFirstIndex = buffer->mAllocatedByteCount;
		buffer->mAllocatedByteCount += sizeInBytes;
		return allocation;
	}


	void GlobalDynamicReadBuffer::commit()
	{
		for (int32 bufferIndex = 0, numBuffers = mFloatBufferPool->mBuffers.size(); bufferIndex < numBuffers; bufferIndex++)
		{
			DynamicAlloReadBuffer& buffer = mFloatBufferPool->mBuffers[bufferIndex];
			if (buffer.mMappedBuffer != nullptr)
			{
				buffer.unlock();
			}
		}
		mFloatBufferPool->mCurrentBuffer = nullptr;
		for (int32 bufferIndex = 0, numBuffers = mInt32BufferPool->mBuffers.size(); bufferIndex < numBuffers; ++bufferIndex)
		{
			DynamicAlloReadBuffer& buffer = mInt32BufferPool->mBuffers[bufferIndex];
			if (buffer.mMappedBuffer != nullptr)
			{
				buffer.unlock();
			}
		}
		mInt32BufferPool->mCurrentBuffer = nullptr;
		mTotalAllocatedSinceLastCommit = 0;
	}

	bool GlobalDynamicReadBuffer::isRenderAlarmLoggingEnabled() const
	{
		return GMaxReadBufferRenderingBytesAllocatedPerFrame > 0 && mTotalAllocatedSinceLastCommit >= (size_t)GMaxReadBufferRenderingBytesAllocatedPerFrame;
	}
}