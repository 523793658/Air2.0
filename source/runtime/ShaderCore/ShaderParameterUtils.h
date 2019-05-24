#pragma once
#include "CoreMinimal.h"
#include "ShaderParameters.h"
namespace Air
{
	template<typename TShaderRHIRef, typename TRHICmdList>
	inline void setConstantBufferParameter(
		TRHICmdList& cmdList,
		TShaderRHIRef shader,
		const ShaderConstantBufferParameter& parameter,
		ConstantBufferRHIParamRef constantBufferRHI
	)
	{
		if (parameter.isBound())
		{
			cmdList.setShaderConstantBuffer(shader, parameter.getBaseIndex(), constantBufferRHI);
		}
	}

	template<typename TShaderRHIRef, typename TBufferStruct, typename TRHICmdList>
	inline void setConstantBufferParameter(
		TRHICmdList& RHICmdList,
		TShaderRHIRef shader,
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

	template<typename TShaderRHIRef>
	inline void setLocalConstantBufferParameter(
		RHICommandList& RhiCmdList,
		TShaderRHIRef shader,
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
	void setShaderValue(RHICommandList& RHICmdList, ShaderRHIParamRef shader, const ShaderParameter& parameter, const bool & value, uint32 elementIndex = 0)
	{
		const uint32 boolValue = value;
		setShaderValue(RHICmdList, shader, parameter, boolValue, elementIndex);
	}

	template<typename ShaderRHIParamRef>
	void setShaderValue(RHIAsyncComputeCommandList& RHICmdList,
		ShaderRHIParamRef shader, const ShaderParameter& parameter,
		const bool & value, uint32 elementIndex = 0)
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

	template<typename ShaderTypeRHIParamRef, typename TRHICmdList>
	FORCEINLINE void setTextureParameter(TRHICmdList & RHICmdList, ShaderTypeRHIParamRef shader,
		const ShaderResourceParameter& textureParameter,
		const ShaderResourceParameter& samplerParameter,
		SamplerStateRHIParamRef samplerStateRHI,
		TextureRHIParamRef textureRHI,
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

	template<typename ShaderTypeRHIParamRef, typename TRHICmdList>
	FORCEINLINE void setTextureParameter(TRHICmdList & RHICmdList,
		ShaderTypeRHIParamRef shader, const ShaderResourceParameter& parameter, TextureRHIParamRef newTextureRHI)
	{
		if (parameter.isBound())
		{
			RHICmdList.setShaderTexture(shader, parameter.getBaseIndex(), newTextureRHI);
		}
	}

	template<typename TShaderRHIRef, typename TBufferStruct>
	inline void setConstantBufferParameterImmediate(RHICommandList& RHICmdList, TShaderRHIRef shader, const TShaderConstantBufferParameter<TBufferStruct>& parameter, const TBufferStruct& constantBufferValue)
	{
		BOOST_ASSERT(parameter.isInitialized());
		if (parameter.isBound())
		{
			RHICmdList.setShaderConstantBuffer(shader, parameter.getBaseIndex(), RHICreateConstantBuffer(&constantBufferValue, TBufferStruct::mStaticStruct.getLayout(), ConstantBuffer_SingleDraw));
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
}