#include "WindowsD3D11UniformBuffer.h"
#include "D3D11DynamicRHI.h"
namespace Air
{
	bool WinD3D11UniformBuffer::commitUniformsToDevice(bool bDiscardSharedConstants)
	{
		if (mCurrentUpdateSize == 0)
		{
			return false;
		}
		if (bDiscardSharedConstants)
		{
			mTotalUpdateSize = mCurrentUpdateSize;
		}
		else
		{
			mTotalUpdateSize = Math::max(mCurrentUpdateSize, mTotalUpdateSize);
		}
		mCurrentSubBuffer = 1;
		uint32 bufferSize = mMaxSize / 2;
		while (bufferSize >= mTotalUpdateSize && mCurrentSubBuffer < mNumSubBuffers)
		{
			mCurrentSubBuffer++;
			bufferSize /= 2;
		}
		mCurrentSubBuffer--;
		bufferSize *= 2;
		ID3D11Buffer* buffer = mBuffers[mCurrentSubBuffer];
		BOOST_ASSERT(((uint64)(mShadowData) & 0xF) == 0);
		mD3DRHI->getDeviceContext()->UpdateSubresource(buffer, 0, NULL, (void*)mShadowData, bufferSize, bufferSize);
		mCurrentUpdateSize = 0;
		return true;
	}

	void D3D11DynamicRHI::initUniformBuffers()
	{
		mVSUnifomBuffers.reserve(MAX_UNIFORM_BUFFER_SLOTS);
		mHSUnifomBuffers.reserve(MAX_UNIFORM_BUFFER_SLOTS);
		mDSUnifomBuffers.reserve(MAX_UNIFORM_BUFFER_SLOTS);
		mPSUnifomBuffers.reserve(MAX_UNIFORM_BUFFER_SLOTS);
		mGSUnifomBuffers.reserve(MAX_UNIFORM_BUFFER_SLOTS);
		mCSUnifomBuffers.reserve(MAX_UNIFORM_BUFFER_SLOTS);
		for (int32 bufferIndex = 0; bufferIndex < MAX_UNIFORM_BUFFER_SLOTS; bufferIndex++)
		{
			uint32 size = GUniformBufferSizes[bufferIndex];
			uint32 subBuffers = 1;
			if (bufferIndex == GLOBAL_UNIFORM_BUFFER_INDEX)
			{
				subBuffers = 5;
			}

			mVSUnifomBuffers.add(new WinD3D11UniformBuffer(this, size, subBuffers));
			mPSUnifomBuffers.add(new WinD3D11UniformBuffer(this, size, subBuffers));
			mHSUnifomBuffers.add(new WinD3D11UniformBuffer(this, size));
			mDSUnifomBuffers.add(new WinD3D11UniformBuffer(this, size));
			mCSUnifomBuffers.add(new WinD3D11UniformBuffer(this, size));
			mGSUnifomBuffers.add(new WinD3D11UniformBuffer(this, size));

		}
	}

	void WinD3D11UniformBuffer::initDynamicRHI()
	{
		TRefCountPtr<ID3D11Buffer> CBuffer = nullptr;
		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		BOOST_ASSERT(mMaxSize <= D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT && (mMaxSize % 16) == 0);
		bufferDesc.ByteWidth = mMaxSize;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		mBuffers = new TRefCountPtr<ID3D11Buffer>[mNumSubBuffers];
		mCurrentSubBuffer = 0;
		for (uint32 s = 0; s < mNumSubBuffers; s++)
		{
			VERIFYD3D11RESULT_EX(mD3DRHI->getDevice()->CreateBuffer(&bufferDesc, NULL, mBuffers[s].getInitReference()), mD3DRHI->getDevice());
			updateBufferStats(mBuffers[s], true);
			bufferDesc.ByteWidth = align(bufferDesc.ByteWidth / 2, 16);
		}

		D3D11UniformBuffer::initDynamicRHI();
	}

	void WinD3D11UniformBuffer::releaseDynamicRHI()
	{
		D3D11UniformBuffer::releaseDynamicRHI();

		if (mBuffers)
		{
			for (uint32 s = 0; s < mNumSubBuffers; s++)
			{
				updateBufferStats(mBuffers[s], false);
			}
			delete[] mBuffers;
			mBuffers = nullptr;
		}
	}
}