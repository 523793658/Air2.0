#pragma once
#include "CoreMinimal.h"
#include "ShaderParameters.h"
namespace Air
{
	template<typename TRHIShader, typename TRHICmdList>
	inline void setConstantBufferParameter(
		TRHICmdList& cmdList,
		TRHIShader* shader,
		const ShaderConstantBufferParameter& parameter,
		RHIConstantBuffer* constantBufferRHI
	)
	{
		if (parameter.isBound())
		{
			cmdList.setShaderConstantBuffer(shader, parameter.getBaseIndex(), constantBufferRHI);
		}
	}

	template<typename TRHIShader, typename TBufferStruct, typename TRHICmdList>
	inline void setConstantBufferParameter(
		TRHICmdList& RHICmdList,
		TRHIShader* shader,
		const TShaderConstantBufferParameter<TBufferStruct>& parameter,
		const TConstantBufferRef<TBufferStruct>& constantBufferRef
	)
	{
		BOOST_ASSERT(parameter.isInitialized());
		if (parameter.isBound())
		{
			RHICmdList.setShaderConstantBuffer(shader, parameter.getBaseIndex(), constantBufferRef);
		}
	}

	template<typename TShaderRHIRef, typename TBufferStruct, typename TRHICmdList>
	inline void setConstantBufferParameter(
		TRHICmdList& RHICmdList,
		TShaderRHIRef shader,
		const TShaderConstantBufferParameter<TBufferStruct>& parameter,
		const TConstantBuffer<TBufferStruct>& constantBuffer)
	{
		BOOST_ASSERT(parameter.isInitialized());
		if (parameter.isBound())
		{
			RHICmdList.setShaderConstantBuffer(shader, parameter.getBaseIndex(), constantBuffer.getConstantBufferRHI());
		}
	}

	template<typename TRHIShader>
	inline void setLocalConstantBufferParameter(
		RHICommandList& RhiCmdList,
		TRHIShader* shader,
		const ShaderConstantBufferParameter& parameter,
		const LocalConstantBuffer& localConstantBuffer
	)
	{
		BOOST_ASSERT(parameter.isInitialized());
		if (parameter.isBound())
		{
			RhiCmdList.setLocalShaderConstantBuffer(shader, parameter.getBaseIndex(), localConstantBuffer);
		}
	}

	template<typename ShaderRHIParamRef, class ParameterType, typename TRHICmdList>
	void setShaderValue(TRHICmdList& RHICmdList,
		ShaderRHIParamRef shader,
		const ShaderParameter& parameter,
		const ParameterType& value,
		uint32 elementIndex = 0)
	{
		static_assert(!std::is_pointer<ParameterType>::value, "Passing by value is not valid.");
		const uint32 alignedTypeSize = align(sizeof(ParameterType), ShaderArrayElementAlignBytes);
		const int32 numBytesToSet = Math::min<int32>(sizeof(ParameterType), parameter.getNumBytes() - elementIndex * alignedTypeSize);
		BOOST_ASSERT(parameter.isInitialized());
		if (numBytesToSet > 0)
		{
			RHICmdList.setShaderParameter(
				shader,
				parameter.getBufferIndex(),
				parameter.getBaseIndex() + elementIndex * alignedTypeSize,
				(uint32)numBytesToSet,
				&value);
		}
	}

	template<typename ShaderRHIParamRef>
	void setShaderValue(RHICommandList& RHICmdList, ShaderRHIParamRef shader, const ShaderParameter& parameter, const bool& value, uint32 elementIndex = 0)
	{
		const uint32 boolValue = value;
		setShaderValue(RHICmdList, shader, parameter, boolValue, elementIndex);
	}

	template<typename ShaderRHIParamRef>
	void setShaderValue(RHIAsyncComputeCommandList& RHICmdList,
		ShaderRHIParamRef shader, const ShaderParameter& parameter,
		const bool& value, uint32 elementIndex = 0)
	{
		const uint32 boolValue = value;
		setShaderValue(RHICmdList, shader, parameter, boolValue, elementIndex);
	}

	template<typename ShaderTypeRHIParamRef, typename TRHICmdList>
	FORCEINLINE void setTextureParameter(
		TRHICmdList& RHICmdList,
		ShaderTypeRHIParamRef shader,
		const ShaderResourceParameter& textureParameter,
		const ShaderResourceParameter& samplerParameter,
		const Texture* texture,
		uint32 elementIndex = 0
	)
	{
		BOOST_ASSERT(textureParameter.isInitialized());
		BOOST_ASSERT(samplerParameter.isInitialized());
		if (textureParameter.isBound())
		{
			texture->mLastRenderTime = App::getCurrentTime();
			if (elementIndex < textureParameter.getNumResources())
			{
				RHICmdList.setShaderTexture(shader, textureParameter.getBaseIndex() + elementIndex, texture->mTextureRHI);
			}
		}

		if (samplerParameter.isBound())
		{
			if (elementIndex < samplerParameter.getNumResources())
			{
				RHICmdList.setShaderSampler(shader, samplerParameter.getBaseIndex() + elementIndex, texture->mSamplerStateRHI);
			}
		}
	}

	template<typename TRHIShader, typename TRHICmdList>
	FORCEINLINE void setTextureParameter(TRHICmdList& RHICmdList, TRHIShader* shader,
		const ShaderResourceParameter& textureParameter,
		const ShaderResourceParameter& samplerParameter,
		RHISamplerState* samplerStateRHI,
		RHITexture* textureRHI,
		uint32 elementIndex = 0)
	{
		BOOST_ASSERT(textureParameter.isInitialized());
		BOOST_ASSERT(samplerParameter.isInitialized());
		if (textureParameter.isBound())
		{
			if (elementIndex < textureParameter.getNumResources())
			{
				RHICmdList.setShaderTexture(shader, textureParameter.getBaseIndex() + elementIndex, textureRHI);
			}
		}
		if (samplerParameter.isBound())
		{
			if (elementIndex < samplerParameter.getNumResources())
			{
				RHICmdList.setShaderSampler(shader, samplerParameter.getBaseIndex() + elementIndex, samplerStateRHI);
			}
		}
	}

	template<typename TRHIShader, typename TRHICmdList>
	FORCEINLINE void setTextureParameter(TRHICmdList& RHICmdList,
		TRHIShader* shader, const ShaderResourceParameter& parameter, RHITexture* newTextureRHI)
	{
		if (parameter.isBound())
		{
			RHICmdList.setShaderTexture(shader, parameter.getBaseIndex(), newTextureRHI);
		}
	}

	template<typename TRHIShader, typename TRHICmdList>
	FORCEINLINE void setSRVParameter(
		TRHICmdList& RHICmdList,
		TRHIShader* shader,
		const ShaderResourceParameter& parameter,
		RHIShaderResourceView* newShaderResourceViewHRI
	)
	{
		if (parameter.isBound())
		{
			RHICmdList.setShaderResourceViewParameter(
				shader,
				parameter.getBaseIndex(),
				newShaderResourceViewHRI
			);
		}
	}


	template<typename TRHIShader, typename TRHICmdList>
	FORCEINLINE void setSRVParameter(
		TRHICmdList& RHICmdList,
		TRefCountPtr<TRHIShader> shader,
		const ShaderResourceParameter& parameter,
		RHIShaderResourceView* newShaderResourceViewHRI
	)
	{
		if (parameter.isBound())
		{
			RHICmdList.setShaderResourceViewParameter(
				shader.getReference(),
				parameter.getBaseIndex(),
				newShaderResourceViewHRI
			);
		}
	}

	template<typename TRHICmdList>
	FORCEINLINE void setUAVParameter(
		TRHICmdList& RHICmdList,
		RHIComputeShader* computeShader,
		const ShaderResourceParameter& parameter,
		RHIUnorderedAccessView* newUnorderedAccessView
	)
	{
		if (parameter.isBound())
		{
			RHICmdList.setUAVParameter(
				computeShader,
				parameter.getBaseIndex(),
				newUnorderedAccessView
			);
		}
	}

	template<typename TRHICmdList>
	inline bool setUAVParameterIfCS(TRHICmdList& RHICmdList, RHIVertexShader* shader, const ShaderResourceParameter& uavParameter, RHIUnorderedAccessView* uav)
	{
		return false;
	}

	template<typename TRHICmdList>
	inline bool setUAVParameterIfCS(TRHICmdList& RHICmdList, RHIPixelShader* shader, const ShaderResourceParameter& uavParameter, RHIUnorderedAccessView* uav)
	{
		return false;
	}

	template<typename TRHICmdList>
	inline bool setUAVParameterIfCS(TRHICmdList& RHICmdList, RHIDomainShader* shader, const ShaderResourceParameter& uavParameter, RHIUnorderedAccessView* uav)
	{
		return false;
	}

	template<typename TRHICmdList>
	inline bool setUAVParameterIfCS(TRHICmdList& RHICmdList, RHIHullShader* shader, const ShaderResourceParameter& uavParameter, RHIUnorderedAccessView* uav)
	{
		return false;
	}

	template<typename TRHICmdList>
	inline bool setUAVParameterIfCS(TRHICmdList& RHICmdList, RHIGeometryShader* shader, const ShaderResourceParameter& uavParameter, RHIUnorderedAccessView* uav)
	{
		return false;
	}

	template<typename TRHICmdList>
	inline bool setUAVParameterIfCS(TRHICmdList& RHICmdList, RHIComputeShader* shader, const ShaderResourceParameter& uavParameter, RHIUnorderedAccessView* uav)
	{
		setUAVParameter(RHICmdList, shader, uavParameter, uav);
		return uavParameter.isBound();
	}


	template<typename TRHIShader, typename TRHICmdList>
	FORCEINLINE void setSamplerParameter(
		TRHICmdList& RHICmdList,
		TRHIShader* shader,
		const ShaderResourceParameter& parameter,
		RHISamplerState* samplerStateRHI
	)
	{
		if (parameter.isBound())
		{
			RHICmdList.setShaderSampler(
				shader,
				parameter.getBaseIndex(),
				samplerStateRHI
			);
		}
	}


	template<typename TShaderRHIRef, typename TBufferStruct>
	inline void setConstantBufferParameterImmediate(RHICommandList& RHICmdList, TShaderRHIRef shader, const TShaderConstantBufferParameter<TBufferStruct>& parameter, const TBufferStruct& constantBufferValue)
	{
		BOOST_ASSERT(parameter.isInitialized());
		if (parameter.isBound())
		{
			RHICmdList.setShaderConstantBuffer(shader, parameter.getBaseIndex(), RHICreateConstantBuffer(&constantBufferValue, TBufferStruct::StaticStructMetadata.getLayout(), ConstantBuffer_SingleDraw));
		}
	}

	template<typename TShaderRHIRef, typename TBufferStruct, typename TRHICmdList>
	inline void setConstantBufferParameterImmediate(TRHICmdList& RHICmdList, TShaderRHIRef shader, const TShaderConstantBufferParameter<TBufferStruct>& parameter, const TBufferStruct& constantBufferValue)
	{
		BOOST_ASSERT(parameter.isInitialized());
		if (parameter.isBound())
		{
			RHICmdList.setShaderConstantBuffer(shader, parameter.getBaseIndex(), RHICreateConstantBuffer(&constantBufferValue, TBufferStruct::mStaticStruct.getLayout(), ConstantBuffer_SingleDraw));
		}
	}

	template<typename TShaderRHIRef, typename TRHICmdList>
	inline void RWShaderParameter::setBuffer(TRHICmdList& RHICmdList, TShaderRHIRef shader, const RWBuffer* rwBuffer) const
	{
		if (!setUAVParameterIfCS(RHICmdList, shader, mUAVParameter, RWBuffer.mUAV))
		{
			setSRVParameter(RHICmdList, shader, mSRVParameter, rwBuffer->mSRV);
		}
	}

	template<typename TShaderRHIRef, typename TRHICmdList>
	inline void RWShaderParameter::setBuffer(TRHICmdList& RHICmdList, TShaderRHIRef shader, const RWBufferStructured& rwBuffer) const
	{
		if (!setUAVParameterIfCS(RHICmdList, shader, mUAVParameter, rwBuffer.mUAV))
		{
			setSRVParameter(RHICmdList, shader, mSRVParameter, rwBuffer..mSRV);
		}
	}

	template<typename TShaderRHIRef, typename TRHICmdList>
	inline void RWShaderParameter::setTexture(TRHICmdList& RHICmdList, TShaderRHIRef shader, RHITexture* texture, RHIUnorderedAccessView* uav) const
	{
		if (!setUAVParameterIfCS(RHICmdList, shader, mUAVParameter, uav))
		{
			setSRVParameter(RHICmdList, shader, mSRVParameter, texture);
		}
	}

	template<typename TRHICmdList>
	inline void RWShaderParameter::unsetUAV(TRHICmdList& RHICmdList, RHIComputeShader* computeShader) const
	{
		setUAVParameter(RHICmdList, computeShader, mUAVParameter, UnorderedAccessViewRHIRef());
	}
}