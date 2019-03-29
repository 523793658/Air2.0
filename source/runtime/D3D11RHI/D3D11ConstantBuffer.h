#pragma once
#include "D3D11Typedefs.h"
#include "D3D11Resource.h"
namespace Air
{



	class D3D11ConstantBuffer : public RHIConstantBuffer
	{
	public:
		TRefCountPtr<ID3D11Buffer> mResource;
		RingAllocation mRingAllocation;
		TArray<TRefCountPtr<RHIResource>> mResourceTable;
		D3D11ConstantBuffer(class D3D11DynamicRHI* inD3DRHI, const RHIConstantBufferLayout& inLayout, ID3D11Buffer* inResource, const RingAllocation& inRingAllocation)
			:RHIConstantBuffer(inLayout)
			, mResource(inResource)
			, mRingAllocation(inRingAllocation)
			, mD3D11RHI(inD3DRHI)
		{}

		void updateUniform(const uint8* data, uint16 offset, uint16 inSize)
		{
			BOOST_ASSERT((uint32)offset + (uint32)inSize <= mMaxSize);
			Memory::memcpy(mShadowData + offset, data, inSize);
			mCurrentUpdateSize = Math::max((uint32)(offset + inSize), mCurrentUpdateSize);
		}

	private:
		class D3D11DynamicRHI* mD3D11RHI;
		uint32 mMaxSize;
		uint8* mShadowData;
		uint32 mCurrentUpdateSize;
		uint32 mTotalUpdateSize;
	};


	DECL_D3D11_RESOURCE_TRAITS(D3D11ConstantBuffer, RHIConstantBuffer)

	extern const int32 NumPoolBuckets;
}