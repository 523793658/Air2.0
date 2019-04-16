#include "RawIndexBuffer.h"
#include "RHICommandList.h"
namespace Air
{
	RawStaticIndexBuffer::RawStaticIndexBuffer(bool inNeedsCPUAccess /* = false */)
		:mIndexStorage(inNeedsCPUAccess)
		,b32Bit(false)
	{

	}

	void RawStaticIndexBuffer::setIndices(const TArray<uint32>& inIndices, EIndexBufferStride::Type desiredStride)
	{
		int32 numIndices = inIndices.size();
		bool bShouldUse32Bit = false;
		if(desiredStride == EIndexBufferStride::Force32Bit)
		{
			bShouldUse32Bit = true;
		}
		else if (desiredStride == EIndexBufferStride::AutoDetect)
		{
			int32 i = 0; 
			while (!bShouldUse32Bit && i < numIndices)
			{
				bShouldUse32Bit = inIndices[i] > std::numeric_limits<uint16>::max();
				i++;
			}
		}
		int32 indexStride = bShouldUse32Bit ? sizeof(uint32) : sizeof(uint16);
		mIndexStorage.empty(indexStride * numIndices);
		mIndexStorage.addUninitialized(indexStride * numIndices);
		if (bShouldUse32Bit)
		{
			BOOST_ASSERT(mIndexStorage.size() == inIndices.size() * inIndices.getTypeSize());
			Memory::memcpy(mIndexStorage.getData(), inIndices.getData(), mIndexStorage.size());
			b32Bit = true;
		}
		else
		{
			BOOST_ASSERT(mIndexStorage.size() == inIndices.size() * sizeof(uint16));
			uint16* destIndices16Bit = (uint16*)mIndexStorage.getData();
			for (int32 i = 0; i < numIndices; i++)
			{
				destIndices16Bit[i] = inIndices[i];
			}
			b32Bit = false;
		}
	}

	void RawStaticIndexBuffer::getCopy(TArray<uint32>& outIndices) const
	{
		int32 numIndices = b32Bit ? (mIndexStorage.size() / 4) : (mIndexStorage.size() / 2);
		outIndices.empty(numIndices);
		outIndices.addUninitialized(numIndices);
		if (b32Bit)
		{
			BOOST_ASSERT(mIndexStorage.size() == outIndices.size() * outIndices.getTypeSize());
			Memory::memcpy(outIndices.getData(), mIndexStorage.getData(), mIndexStorage.size());
		}
		else
		{
			BOOST_ASSERT(mIndexStorage.size() == outIndices.size() * sizeof(16));
			const uint16* srcIndices16Bits = (const uint16*)mIndexStorage.getData();
			for (int32 i = 0; i < numIndices; i++)
			{
				outIndices[i] = srcIndices16Bits[i];
			}
		}
	}

	IndexArrayView RawStaticIndexBuffer::getArrayView() const
	{
		int32 numIndices = b32Bit ? (mIndexStorage.size() / 4) : (mIndexStorage.size() / 2);
		return IndexArrayView(mIndexStorage.getData(), numIndices, b32Bit);
	}

	void RawStaticIndexBuffer::initRHI()
	{
		uint16 indexStride = b32Bit ? sizeof(uint32) : sizeof(uint16);
		uint16 sizeInBytes = mIndexStorage.size();
		if (sizeInBytes > 0)
		{
			RHIResourceCreateInfo createInfo(&mIndexStorage);
			mIndexBufferRHI = RHICreateIndexBuffer(indexStride, sizeInBytes, BUF_Static, createInfo);
		}
	}

	void RawStaticIndexBuffer::serialize(Archive& ar, bool bNeedsCPUAccess)
	{
		ar << b32Bit;
		mIndexStorage.bulkSerialize(ar);
	}
}