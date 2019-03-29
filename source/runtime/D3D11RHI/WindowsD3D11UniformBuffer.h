#pragma once
#include "D3D11UniformBuffer.h"
#include "D3D11Typedefs.h"
struct ID3D11Buffer;
namespace Air
{
	class WinD3D11UniformBuffer : public D3D11UniformBuffer
	{
	public:
		WinD3D11UniformBuffer(D3D11DynamicRHI* inD3DRHI, uint32 inSize = 0, uint32 subBuffers = 1)
			:D3D11UniformBuffer(inD3DRHI, inSize, subBuffers),
			mBuffers(nullptr),
			mCurrentSubBuffer(0),
			mNumSubBuffers(subBuffers)
		{}

		virtual void initDynamicRHI() override;
		virtual void releaseDynamicRHI() override;

		ID3D11Buffer* getUniformBuffer() const
		{
			return mBuffers[mCurrentSubBuffer];
		}

		bool commitUniformsToDevice(bool bDiscardSharedConstants);
	private:
		ID3D11BufferPtr* mBuffers;
		uint32 mCurrentSubBuffer;
		uint32 mNumSubBuffers;
	};
}