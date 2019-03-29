#pragma once
#include "RenderResource.h"
#define MAX_GLOBAL_UNIFORM_BUFFER_SIZE	4096
namespace Air
{
	class D3D11DynamicRHI;
	enum ED3D11ShaderOffsetBuffer
	{
		GLOBAL_UNIFORM_BUFFER_INDEX = 0,
		MAX_UNIFORM_BUFFER_SLOTS
	};

	extern const uint32 GUniformBufferSizes[MAX_UNIFORM_BUFFER_SLOTS];

	class D3D11UniformBuffer : public RenderResource, public RefCountedObject
	{
	public:
		D3D11UniformBuffer(D3D11DynamicRHI* inD3DRHI, uint32 inSize = 0, uint32 subBuffers = 1);

		virtual ~D3D11UniformBuffer();

		virtual void initDynamicRHI() override;
		virtual void releaseDynamicRHI() override;

		void updateUniform(const uint8* data, uint16 offset, int16 inSize)
		{
			BOOST_ASSERT((uint32)offset + (uint32)inSize <= mMaxSize);
			Memory::memcpy(mShadowData + offset, data, inSize);
			mCurrentUpdateSize = Math::max((uint32)(offset + inSize), mCurrentUpdateSize);
		}

	protected:
		D3D11DynamicRHI* mD3DRHI;
		uint32 mMaxSize;
		uint8* mShadowData;
		uint32 mCurrentUpdateSize;
		uint32 mTotalUpdateSize;
	};
}