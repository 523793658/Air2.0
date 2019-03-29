#include "D3D11StateCache.h"
#include "RHIDefinitions.h"
#include "HAL/AirMemory.h"
namespace Air
{

	void D3D11StateCacheBase::clearState()
	{
		if (mDirect3DDeviceIMContext)
		{
			mDirect3DDeviceIMContext->ClearState();
		}

#if D3D11_ALLOW_STATE_CACHE
		for (uint32 ShaderFrequency = 0; ShaderFrequency < SF_NumFrequencies; ShaderFrequency++)
		{
			for (uint32 index = 0; index < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; ++index)
			{
				if (mCurrentShaderResourceView[ShaderFrequency][index])
				{
					mCurrentShaderResourceView[ShaderFrequency][index]->Release();
					mCurrentShaderResourceView[ShaderFrequency][index] = NULL;
				}
			}
		}
		mCurrentRasterizerState = nullptr;
		
		mCurrentReferenceStencil = 0;
		mCurrentDepthStencilState = nullptr;

		mCurrentVertexShader = nullptr;
		mCurrentHullShader = nullptr;
		mCurrentDomainShader = nullptr;
		mCurrentGeometryShader = nullptr;
		mCurrentPixelShader = nullptr;
		mCurrentComputeShader = nullptr;

		mCurrentBlendFactor[0] = 1.0f;
		mCurrentBlendFactor[1] = 1.0f;
		mCurrentBlendFactor[2] = 1.0f;
		mCurrentBlendFactor[3] = 1.0f;

		Memory::Memset(&mCurrentViewport[0], 0, sizeof(D3D11_VIEWPORT) * D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
		mCurrentNumberOfViewports = 0;

		mCurrentBlendSampleMask = 0xffffffff;
		mCurrentBlendState = nullptr;

		mCurrentInputLayout = nullptr;

		Memory::memzero(mCurrentVertexBuffers, sizeof(mCurrentVertexBuffers));

		Memory::memzero(mCurrentSamplerStates, sizeof(mCurrentSamplerStates));

		mCurrentIndexBuffer = nullptr;
		mCurrentIndexFormat = DXGI_FORMAT_UNKNOWN;

		mCurrentIndexOffset = 0;
		mCurrentPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

		for (uint32 frequency = 0; frequency < SF_NumFrequencies; frequency++)
		{
			for (uint32 index = 0; index < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT; index++)
			{
				mCurrentConstantBuffers[frequency][index].mBuffer = nullptr;
				mCurrentConstantBuffers[frequency][index].mFirstConstant = 0;
				mCurrentConstantBuffers[frequency][index].mNumConstants = D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT;
			}
		}
#endif
	}

}