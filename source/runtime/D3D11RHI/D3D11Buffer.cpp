#include "D3D11Buffer.h"
#include "D3D11DynamicRHI.h"
namespace Air
{
	void* D3D11DynamicBuffer::lock(uint32 size)
	{
		BOOST_ASSERT(mlockedBufferIndex == -1 && mBuffers.size() > 0);
		int32 bufferIndex = 0;
		int32 numBuffers = mBuffers.size();
		while (bufferIndex < numBuffers && mBufferSizes[bufferIndex] < size)
		{
			bufferIndex++;
		}
		if (bufferIndex == numBuffers)
		{
			bufferIndex--;
			TRefCountPtr<ID3D11Buffer> buffer;
			D3D11_BUFFER_DESC desc;
			ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = mBindFlags;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			desc.ByteWidth = size;
			VERIFYD3D11RESULT_EX(mD3DRHI->getDevice()->CreateBuffer(&desc, NULL, buffer.getInitReference()), mD3DRHI->getDevice());
			updateBufferStats(mBuffers[bufferIndex], false);
			updateBufferStats(buffer, true);
			mBuffers[bufferIndex] = buffer;
			mBufferSizes[bufferIndex] = size;
		}
		mlockedBufferIndex = bufferIndex;
		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		VERIFYD3D11RESULT_EX(mD3DRHI->getDeviceContext()->Map(mBuffers[bufferIndex], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource), mD3DRHI->getDevice());
		return mappedSubresource.pData;
	}

	ID3D11Buffer* D3D11DynamicBuffer::unlock()
	{
		BOOST_ASSERT(mlockedBufferIndex != -1);
		ID3D11Buffer* lockedBuffer = mBuffers[mlockedBufferIndex];
		mD3DRHI->getDeviceContext()->Unmap(lockedBuffer, 0);
		mlockedBufferIndex = -1;
		return lockedBuffer;
	}
}