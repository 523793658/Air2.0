#include "MeshPassProcessor.h"
#include "MeshDrawShaderBindings.h"
namespace Air
{
	class ReadOnlyMeshDrawSingleShaderBindings : public MeshDrawShaderBindingsLayout
	{
	public:
		ReadOnlyMeshDrawSingleShaderBindings(const MeshDrawShaderBindingsLayout& inLayout, const uint8* inData)
			:MeshDrawShaderBindingsLayout(inLayout)
		{
			mData = inData;
		}

		inline RHIConstantBuffer* const* getConstantBufferStart() const
		{
			return (RHIConstantBuffer**)(mData + getConstantBufferOffset());
		}

		inline RHISamplerState** getSamplerStart() const
		{
			const uint8* samplerDataStart = mData + getSamplerOffset();
			return (RHISamplerState**)samplerDataStart;
		}

		inline RHIResource** getSRVStart() const
		{
			const uint8* srvDataStart = mData + getSRVOffset();
			return (RHIResource**)srvDataStart;
		}

		inline const uint8* getSRVTypeStart() const
		{
			const uint8* srvTypeDataStart = mData + getSRVTypeOffset();
			return srvTypeDataStart;
		}

		inline const uint8* getLooseDataStart() const
		{
			const uint8* looseDataStart = mData + getLooseDataOffset();
			return looseDataStart;
		}


	private:
		const uint8* mData;
	};

	void MeshDrawShaderBindings::copyFrom(const MeshDrawShaderBindings& other)
	{
		release();
		mShaderLayouts = other.mShaderLayouts;
		allocate(other.mSize);
		PlatformMemory::Memcpy(getData(), other.getData(), mSize);
	}

	void MeshDrawShaderBindings::release()
	{
		if (mSize > ARRAY_COUNT(mInlineStorage))
		{
			delete[] mHeapData;
		}
		mSize = 0;
		mHeapData = nullptr;
	}

	template<class RHIShaderType>
	void MeshDrawShaderBindings::setShaderBindings(RHICommandList& RHICmdList, RHIShaderType shader, const class ReadOnlyMeshDrawSingleShaderBindings& RESTRICT singleShaderBings)
	{
		RHIConstantBuffer* const* RESTRICT constantBufferBindings = singleShaderBings.getConstantBufferStart();
		const ShaderParameterInfo* RESTRICT constantBufferParameters = singleShaderBings.mParameterMapInfo.mConstantBuffers.getData();
		const int32 numConstantBuffers = singleShaderBings.mParameterMapInfo.mConstantBuffers.size();

		for (int32 constantBufferIndex = 0; constantBufferIndex < numConstantBuffers; constantBufferIndex++)
		{
			ShaderParameterInfo parameter = constantBufferParameters[constantBufferIndex];
			RHIConstantBuffer* constantBuffer = constantBufferBindings[constantBufferIndex];
			RHICmdList.setShaderConstantBuffer(shader, parameter.mBaseIndex, constantBuffer);
		}

		RHISamplerState* const* RESTRICT samplerBindings = singleShaderBings.getSamplerStart();
		const ShaderParameterInfo* RESTRICT textureSamplerParameters = singleShaderBings.mParameterMapInfo.mTextureSamplers.getData();
		const int32 numTextureSamplers = singleShaderBings.mParameterMapInfo.mTextureSamplers.size();
		for (int32 samplerIndex = 0; samplerIndex < numTextureSamplers; samplerIndex++)
		{
			ShaderParameterInfo parameter = textureSamplerParameters[samplerIndex];
			RHISamplerState* sampler = (RHISamplerState*)samplerBindings[samplerIndex];
			RHICmdList.setShaderSampler(shader, parameter.mBaseIndex, sampler);
		}

		const uint8* RESTRICT srvType = singleShaderBings.getSRVTypeStart();
		RHIResource* const* RESTRICT srvBindings = singleShaderBings.getSRVStart();
		const ShaderParameterInfo* RESTRICT srvParameters = singleShaderBings.mParameterMapInfo.mSRVs.getData();
		const uint32 numSRVs = singleShaderBings.mParameterMapInfo.mSRVs.size();

		for (uint32 srvIndex = 0; srvIndex < numSRVs; srvIndex++)
		{
			ShaderParameterInfo parameter = srvParameters[srvIndex];
			uint32 typeByteIndex = srvIndex / 8;
			uint32 typeBitIndex = srvIndex - typeByteIndex;
			if (srvType[typeByteIndex] & (1 << typeBitIndex))
			{
				RHIShaderResourceView* srv = (RHIShaderResourceView*)srvBindings[srvIndex];
				RHICmdList.setShaderResourceViewParameter(shader, parameter.mBaseIndex, srv);
			}
			else
			{
				RHITexture* texture = (RHITexture*)srvBindings[srvIndex];
				RHICmdList.setShaderTexture(shader, parameter.mBaseIndex, texture);
			}
		}
		const uint8* looseDataStart = singleShaderBings.getLooseDataStart();

		for (const ShaderLooseParameterBufferInfo& looseParameterBuffer : singleShaderBings.mParameterMapInfo.mLooseParameterBuffers)
		{
			for (ShaderParameterInfo parameter : looseParameterBuffer.mParameters)
			{
				RHICmdList.setShaderParameter(shader,
					looseParameterBuffer.mBufferIndex,
					parameter.mBaseIndex,
					parameter.mSize,
					looseDataStart);
				looseDataStart += parameter.mSize;
			}

		}
	}
}