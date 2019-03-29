#pragma once
#include "RenderResource.h"

#include "D3D11Typedefs.h"
namespace Air
{
	class D3D11DynamicBuffer : public RenderResource, public RefCountedObject
	{
	public:
		D3D11DynamicBuffer(class D3D11DynamicRHI* inD3DRHI, D3D11_BIND_FLAG inBindFlags, uint32* inBufferSizes);

		~D3D11DynamicBuffer();

		void* lock(uint32 size);
		ID3D11Buffer* unlock();
		virtual void initRHI() override;
		virtual void releaseRHI() override;

	private:
		enum { MAX_BUFFER_SIZES = 4 };
		TArray<uint32, TFixedAllocator<MAX_BUFFER_SIZES>> mBufferSizes;
		TArray<ID3D11BufferPtr, TFixedAllocator<MAX_BUFFER_SIZES>> mBuffers;
		class D3D11DynamicRHI* mD3DRHI;
		D3D11_BIND_FLAG mBindFlags;
		int32 mlockedBufferIndex;
	};
}