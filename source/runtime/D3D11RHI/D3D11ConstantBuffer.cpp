#include "D3D11ConstantBuffer.h"
#include "D3D11DynamicRHI.h"
#include "D3D11Util.h"
namespace Air
{
	const int32 NumPoolBuckets = 17;
	const int32 NumSafeFrames = 3;
	struct PooledConstantBuffer
	{
		TRefCountPtr<ID3D11Buffer> mBuffer;
		uint32 mCreatedSize;
		uint32 mFrameFreed;
	};

	TArray<PooledConstantBuffer> mConstantBufferPool[NumPoolBuckets];
	TArray<PooledConstantBuffer> mSafeConstantBufferPools[NumSafeFrames][NumPoolBuckets];

	static bool IsPoolingEnabled()
	{
		/*if(GRHIThread && isInRenderingThread() &&GRHICommandList.isrhith)*/
		return false;
	}

	ConstantBufferRHIRef D3D11DynamicRHI::RHICreateConstantBuffer(const void* contents, const RHIConstantBufferLayout& layout, EConstantBufferUsage usage)
	{
		BOOST_ASSERT(isInRenderingThread());
		D3D11ConstantBuffer* newConstantBuffer = nullptr;
		const uint32 numBytes = layout.mConstantBufferSize;
		if (numBytes > 0)
		{
			BOOST_ASSERT(align(numBytes, 16) == numBytes);
			BOOST_ASSERT(align(contents, 16) == contents);
			BOOST_ASSERT(numBytes <= D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16);
			BOOST_ASSERT(numBytes < (1 << NumPoolBuckets));
			if (IsPoolingEnabled())
			{

			}
			else
			{
				D3D11_BUFFER_DESC desc;
				desc.ByteWidth = numBytes;
				desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				desc.CPUAccessFlags = 0;
				desc.MiscFlags = 0;
				desc.StructureByteStride = 0;
				desc.Usage = D3D11_USAGE_IMMUTABLE;
				D3D11_SUBRESOURCE_DATA immutableData;
				immutableData.pSysMem = contents;
				immutableData.SysMemPitch = immutableData.SysMemSlicePitch = 0;
				TRefCountPtr<ID3D11Buffer> constantBufferResource;
				VERIFYD3D11RESULT_EX(mD3D11Device->CreateBuffer(&desc, &immutableData, constantBufferResource.getInitReference()), mD3D11Device);
				newConstantBuffer = new D3D11ConstantBuffer(this, layout, constantBufferResource, RingAllocation());
			}
		}
		else
		{
			newConstantBuffer = new D3D11ConstantBuffer(this, layout, nullptr, RingAllocation());
		}
		if (layout.mResource.size())
		{
			int32 numResource = layout.mResource.size();
			RHIResource** inResources = (RHIResource**)((uint8*)contents + layout.mResourceOffset);
			newConstantBuffer->mResourceTable.empty(numResource);
			newConstantBuffer->mResourceTable.addZeroed(numResource);
			for (int i = 0; i < numResource; ++i)
			{
				newConstantBuffer->mResourceTable[i] = inResources[i];
			}
		}
		return newConstantBuffer;
	}

	void constantBufferBeginFrame()
	{
		int32 numCleaned = 0;
		for (int32 bucketIndex = 0; bucketIndex < NumPoolBuckets; bucketIndex++)
		{
			for (int32 entryIndex = mConstantBufferPool[bucketIndex].size() - 1; entryIndex >= 0 && numCleaned < 10; entryIndex--)
			{
				PooledConstantBuffer& poolEntry = mConstantBufferPool[bucketIndex][entryIndex];
				BOOST_ASSERT(isValidRef(poolEntry.mBuffer));
				if (GFrameNumberRenderThread - poolEntry.mFrameFreed > 30)
				{
					numCleaned++;
					updateBufferStats(poolEntry.mBuffer, false);
					mConstantBufferPool[bucketIndex].removeAtSwap(entryIndex);
				}
			}
		}
		const int32 safeFrameIndex = GFrameNumberRenderThread % NumSafeFrames;
		for (int32 bucketIndex = 0; bucketIndex < NumPoolBuckets; bucketIndex++)
		{
			int32 lastNum = mConstantBufferPool[bucketIndex].size();
			mConstantBufferPool[bucketIndex].append(mSafeConstantBufferPools[safeFrameIndex][bucketIndex]);
			while (lastNum < mConstantBufferPool[bucketIndex].size())
			{
				BOOST_ASSERT(isValidRef(mConstantBufferPool[bucketIndex][lastNum].mBuffer));
				lastNum++;
			}
			mSafeConstantBufferPools[safeFrameIndex][bucketIndex].reset();
		}
	}

}