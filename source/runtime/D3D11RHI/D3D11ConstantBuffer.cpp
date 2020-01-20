#include "D3D11ConstantBuffer.h"
#include "D3D11DynamicRHI.h"
#include "D3D11Util.h"
namespace Air
{

#if _DEBUG
	constexpr EConstantBufferValidation ConstantBufferValidation = EConstantBufferValidation::ValidateResources;
#else
	constexpr EConstantBufferValidation ConstantBufferValidation = EConstantBufferValidation::None;
#endif

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

	ConstantBufferRHIRef D3D11DynamicRHI::RHICreateConstantBuffer(const void* contents, const RHIConstantBufferLayout& layout, EConstantBufferUsage usage, EConstantBufferValidation validation)
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
		if (layout.mResources.size())
		{
			int32 numResource = layout.mResources.size();
			newConstantBuffer->mResourceTable.empty(numResource);
			newConstantBuffer->mResourceTable.addZeroed(numResource);
			for (int i = 0; i < numResource; ++i)
			{
				RHIResource* resource = *(RHIResource * *)((uint8*)contents + layout.mResources[i].mMemberOffset);
				if (!(GMaxRHIFeatureLevel <= ERHIFeatureLevel::ES3_1 && (layout.mResources[i].mMemberType == CBMT_SRV || layout.mResources[i].mMemberType == CBMT_RDG_TEXTURE_SRV || layout.mResources[i].mMemberType == CBMT_RDG_BUFFER_SRV)) && validation == EConstantBufferValidation::ValidateResources)
				{
					BOOST_ASSERT(resource);
				}
				newConstantBuffer->mResourceTable[i] = resource;
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
	void updateConstantBufferContents(ID3D11Device* device, ID3D11DeviceContext* context, D3D11ConstantBuffer* constantBuffer, const void* contents, uint32 constantBufferSize)
	{
		if (constantBufferSize > 0)
		{
			BOOST_ASSERT(align(contents, 16) == contents);
			D3D11_MAPPED_SUBRESOURCE mappedSubresource;
			VERIFYD3D11RESULT_EX(context->Map(constantBuffer->mResource.getReference(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource), device);
			BOOST_ASSERT(mappedSubresource.RowPitch >= constantBufferSize);
			Memory::memcpy(mappedSubresource.pData, contents, constantBufferSize);
			context->Unmap(constantBuffer->mResource.getReference(), 0);
		}
	}

	void D3D11DynamicRHI::RHIUpdateConstantBuffer(RHIConstantBuffer* constantBufferRHI, const void* contents) 
	{
		BOOST_ASSERT(isInRenderingThread());
		BOOST_ASSERT(constantBufferRHI);

		D3D11ConstantBuffer* constantBuffer = ResourceCast(constantBufferRHI);
		const RHIConstantBufferLayout& layout = constantBufferRHI->getLayout();

		const uint32 constantBufferSize = layout.mConstantBufferSize;
		const int32 numResources = layout.mResources.size();

		BOOST_ASSERT(constantBuffer->mResourceTable.size() == numResources);

		RHICommandListImmediate& RHICmdList = RHICommandListExecutor::getImmediateCommandList();

		if (RHICmdList.bypass())
		{
			updateConstantBufferContents(mD3D11Device, mD3D11Context, constantBuffer, contents, constantBufferSize);

			for (int32 resourceIndex = 0; resourceIndex < numResources; resourceIndex++)
			{
				RHIResource* resource = *(RHIResource * *)((uint8*)contents + layout.mResources[resourceIndex].mMemberOffset);

				if (!(GMaxRHIFeatureLevel <= ERHIFeatureLevel::ES3_1 && (layout.mResources[resourceIndex].mMemberType == CBMT_SRV || layout.mResources[resourceIndex].mMemberType == CBMT_RDG_TEXTURE_SRV || layout.mResources[resourceIndex].mMemberType == CBMT_RDG_BUFFER_SRV)) && ConstantBufferValidation == EConstantBufferValidation::ValidateResources)
				{
					BOOST_ASSERT(resource);
				}
				constantBuffer->mResourceTable[resourceIndex] = resource;
			}
		}
		else
		{
			RHIResource** cmdListResources = nullptr;
			void* cmdListConstantBufferData = nullptr;

			if (numResources > 0)
			{
				cmdListResources = (RHIResource * *)RHICmdList.alloc(sizeof(RHIResource*) * numResources, alignof(RHIResource*));

				for (int32 resourceIndex = 0; resourceIndex < numResources; ++resourceIndex)
				{
					RHIResource* resource = *(RHIResource * *)((uint8*)contents + layout.mResources[resourceIndex].mMemberOffset);
					if (!(GMaxRHIFeatureLevel <= ERHIFeatureLevel::ES3_1 && (layout.mResources[resourceIndex].mMemberType == CBMT_SRV || layout.mResources[resourceIndex].mMemberType == CBMT_RDG_TEXTURE_SRV || layout.mResources[resourceIndex].mMemberType == CBMT_RDG_BUFFER_SRV)) && ConstantBufferValidation == EConstantBufferValidation::ValidateResources)
					{
						BOOST_ASSERT(resource);
					}
					cmdListResources[resourceIndex] = resource;
				}
			}
			if (constantBufferSize > 0)
			{
				cmdListConstantBufferData = (void*)RHICmdList.alloc(constantBufferSize, 16);

				Memory::memcpy(cmdListConstantBufferData, contents, constantBufferSize);
			}

			RHICmdList.enqueueLambda([direct3DDeviceContext = mD3D11Context.getReference(),
				direct3DDevice = mD3D11Device.getReference(),
				constantBuffer, cmdListResources, numResources, cmdListConstantBufferData, constantBufferSize](RHICommandList&){
				updateConstantBufferContents(direct3DDevice, direct3DDeviceContext, constantBuffer, cmdListConstantBufferData, constantBufferSize);
				for (int32 resourceIndex = 0; resourceIndex < numResources; ++resourceIndex)
				{
					constantBuffer->mResourceTable[resourceIndex] = cmdListResources[resourceIndex];
				}
			});
			RHICmdList.RHIThreadFence(true);
		}
	}
}