#include "D3D11UniformBuffer.h"
namespace Air
{
	const uint32 GUniformBufferSizes[MAX_UNIFORM_BUFFER_SLOTS] =
	{
		(uint32)align(MAX_GLOBAL_UNIFORM_BUFFER_SIZE, 16),
	};

	D3D11UniformBuffer::D3D11UniformBuffer(D3D11DynamicRHI* inD3DRHI, uint32 inSize /* = 0 */, uint32 subBuffers /* = 1 */)
		:mD3DRHI(inD3DRHI)
		,mMaxSize(inSize)
		,mShadowData(nullptr)
		,mCurrentUpdateSize(0)
		,mTotalUpdateSize(0)
	{
		initResource();
	}

	D3D11UniformBuffer::~D3D11UniformBuffer()
	{
		releaseResource();
	}

	void D3D11UniformBuffer::initDynamicRHI()
	{
		mShadowData = (uint8*)Memory::malloc(mMaxSize, 16);
		Memory::memzero(mShadowData, mMaxSize);
	}

	void D3D11UniformBuffer::releaseDynamicRHI()
	{
		if (mShadowData)
		{
			Memory::free(mShadowData);
		}
	}
}