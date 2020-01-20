#pragma once
#include "CoreMinimal.h"
#include "D3D11Typedefs.h"
#include "RHIDefinitions.h"
#include "HAL/AirMemory.h"
#define D3D11_ALLOW_STATE_CACHE 1

#define D3D11_STATE_CACHE_INLINE	FORCEINLINE

#ifndef D3D11_STATE_CACHE_DEBUG
#define D3D11_STATE_CACHE_DEBUG 0
#endif

#if D3D11_ALLOW_STATE_CACHE && D3D11_STATE_CACHE_DEBUG
#define D3D11_STATE_CACHE_VERIFY(...) VerifyCacheState()
#define D3D11_STATE_CACHE_VERIFY_PRE(...) VerifyCacheStatePre()
#define D3D11_STATE_CACHE_VERIFY_POST(...) VerifyCacheStatePost()
#else
#define D3D11_STATE_CACHE_VERIFY(...)
#define D3D11_STATE_CACHE_VERIFY_PRE(...)
#define D3D11_STATE_CACHE_VERIFY_POST(...)
#endif

#define D3D11_STATE_CACHE_RUNTIME_TOGGLE 0

#if D3D11_ALLOW_STATE_CACHE && D3D11_STATE_CACHE_RUNTIME_TOGGLE
extern bool GD3D11SkipStateCaching;
#else
static const bool GD3D11SkipStateCaching = false;
#endif


namespace Air
{
	class D3D11StateCacheBase
	{
	public:
		enum ESRV_Type
		{
			SRV_Unknown,
			SRV_Dynamic,
			SRV_Static
		};

		bool bDepthBoundsEnabled{ false };

		float mDepthBoundsMin{ 0.0f };

		float mDepthBoundsMax{ 1.0f };
	public:
		void init(ID3D11DeviceContext* inContext, bool bInAlwaysSetIndexBuffers = false)
		{
			setContext(inContext);
#if D3D11_ALLOW_STATE_CACHE
			mAlwaysSetIndexBuffers = bInAlwaysSetIndexBuffers;
#endif
		}

		virtual D3D11_STATE_CACHE_INLINE void setContext(ID3D11DeviceContext* inContext)
		{
			mDirect3DDeviceIMContext = inContext;
			clearState();
			D3D11_STATE_CACHE_VERIFY();
		}

		D3D11_STATE_CACHE_INLINE void setViewport(D3D11_VIEWPORT viewport)
		{
#if D3D11_ALLOW_STATE_CACHE
			D3D11_STATE_CACHE_VERIFY_PRE();
			if ((mCurrentNumberOfViewports != 1 || Memory::memcmp(&mCurrentViewport[0], &viewport, sizeof(D3D11_VIEWPORT))))
			{
				Memory::memcpy(&mCurrentViewport[0], &viewport, sizeof(D3D11_VIEWPORT));
				mCurrentNumberOfViewports = 1;
				mDirect3DDeviceIMContext->RSSetViewports(1, &viewport);
			}
			D3D11_STATE_CACHE_VERIFY_POST();
#else
			mDirect3DDeviceIMContext->RSSetViewports(1, &viewport);
#endif
		}



		D3D11_STATE_CACHE_INLINE void setViewports(uint32 count, D3D11_VIEWPORT* viewports)
		{
#if D3D11_ALLOW_STATE_CACHE
			D3D11_STATE_CACHE_VERIFY_PRE();
			if (mCurrentNumberOfViewports != count || Memory::memcmp(&mCurrentViewport[0], viewports, sizeof(D3D11_VIEWPORT) * count))
			{
				Memory::memcpy(&mCurrentViewport[0], viewports, sizeof(D3D11_VIEWPORT) * count);
				mCurrentNumberOfViewports = count;
				mDirect3DDeviceIMContext->RSSetViewports(count, viewports);
			}
			D3D11_STATE_CACHE_VERIFY_POST();
#else
			mDirect3DDeviceIMContext->RSSetViewports(count, viewports);
#endif
		}

		D3D11_STATE_CACHE_INLINE void getViewports(uint32* count, D3D11_VIEWPORT *viewports)
		{
#if D3D11_ALLOW_STATE_CACHE
			BOOST_ASSERT(*count);
			if (viewports)
			{
				int32 storageSizeCount = (int32)(*count);
				int32 copyCount = std::min<int32>(std::min<int32>(storageSizeCount, (int32)mCurrentNumberOfViewports), D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
				if (copyCount > 0)
				{
					Memory::memcpy(viewports, &mCurrentViewport[0], sizeof(D3D11_VIEWPORT) * copyCount);
				}
				if (storageSizeCount > copyCount)
				{
					Memory::Memset(&viewports[copyCount], 0, sizeof(D3D11_VIEWPORT)* (storageSizeCount - copyCount));
				}
			}
			*count = mCurrentNumberOfViewports;
#else
			mDirect3DDeviceIMContext->RSGetViewports(count, viewports);
#endif
		}

		D3D11_STATE_CACHE_INLINE void setVertexShader(ID3D11VertexShader* shader)
		{
#if D3D11_ALLOW_STATE_CACHE
			D3D11_STATE_CACHE_VERIFY_PRE();
			if ((mCurrentVertexShader != shader))
			{
				mCurrentVertexShader = shader;
				mDirect3DDeviceIMContext->VSSetShader(shader, nullptr, 0);
			}
			D3D11_STATE_CACHE_VERIFY_POST()
#else
			mDirect3DDeviceIMContext->VSSetShader(shader, nullptr, 0);
#endif
		}

		D3D11_STATE_CACHE_INLINE void getVertexShader(ID3D11VertexShader** vertexShader)
		{
#if D3D11_ALLOW_STATE_CACHE
			*vertexShader = mCurrentVertexShader;
			if (mCurrentVertexShader)
			{
				mCurrentVertexShader->AddRef();
			}
#else
			mDirect3DDeviceIMContext->VSGetShader(vertexShader, nullptr, nullptr);
#endif
		}

		D3D11_STATE_CACHE_INLINE void setPixelShader(ID3D11PixelShader* shader)
		{
#if D3D11_ALLOW_STATE_CACHE
			D3D11_STATE_CACHE_VERIFY_PRE();
			if (mCurrentPixelShader != shader)
			{
				mCurrentPixelShader = shader;
				mDirect3DDeviceIMContext->PSSetShader(shader, nullptr, 0);
			}
			D3D11_STATE_CACHE_VERIFY_POST();
#else
			mDirect3DDeviceIMContext->PSSetShader(shader, nullptr, 0);
#endif
		}


		D3D11_STATE_CACHE_INLINE void getPixelShader(ID3D11PixelShader** pixelShader)
		{
#if D3D11_ALLOW_STATE_CACHE
			*pixelShader = mCurrentPixelShader;
			if (mCurrentPixelShader)
			{
				mCurrentPixelShader->AddRef();
			}
#else
			mDirect3DDeviceIMContext->PSGetShader(pixelShader, nullptr, nullptr);
#endif
		}

		D3D11_STATE_CACHE_INLINE void setHullShader(ID3D11HullShader* shader)
		{
#if D3D11_ALLOW_STATE_CACHE
			D3D11_STATE_CACHE_VERIFY_PRE();
			if (mCurrentHullShader != shader)
			{
				mCurrentHullShader = shader;
				mDirect3DDeviceIMContext->HSSetShader(shader, nullptr, 0);
			}
			D3D11_STATE_CACHE_VERIFY_POST();
#else
			mDirect3DDeviceIMContext->HSSetShader(shader, nullptr, 0);
#endif
		}

		D3D11_STATE_CACHE_INLINE void getHullShader(ID3D11HullShader** hullShader)
		{
#if D3D11_ALLOW_STATE_CACHE
			*hullShader = mCurrentHullShader;
			if (mCurrentHullShader)
			{
				mCurrentHullShader->AddRef();
			}
#else
			mDirect3DDeviceIMContext->HSGetShader(hullShader, nullptr, nullptr);
#endif
		}

		D3D11_STATE_CACHE_INLINE void setDomainShader(ID3D11DomainShader* shader)
		{
#if D3D11_ALLOW_STATE_CACHE
			D3D11_STATE_CACHE_VERIFY_PRE();
			if (mCurrentDomainShader != shader)
			{
				mCurrentDomainShader = shader;
				mDirect3DDeviceIMContext->DSSetShader(shader, nullptr, 0);
			}
			D3D11_STATE_CACHE_VERIFY_POST();
#else
			mDirect3DDeviceIMContext->DSSetShader(shader, nullptr, 0);
#endif
		}

		D3D11_STATE_CACHE_INLINE void getDomainShader(ID3D11DomainShader** domainShader)
		{
#if D3D11_ALLOW_STATE_CACHE
			*domainShader = mCurrentDomainShader;
			if (mCurrentDomainShader)
			{
				mCurrentDomainShader->AddRef();
			}
#else
			mDirect3DDeviceIMContext->DSGetShader(domainShader, nullptr, nullptr);
#endif
		}

		D3D11_STATE_CACHE_INLINE void setComputeShader(ID3D11ComputeShader* shader)
		{
#if D3D11_ALLOW_STATE_CACHE
			D3D11_STATE_CACHE_VERIFY_PRE();
			if (mCurrentComputeShader != shader)
			{
				mCurrentComputeShader = shader;
				mDirect3DDeviceIMContext->CSSetShader(shader, nullptr, 0);
			}
			D3D11_STATE_CACHE_VERIFY_POST();
#else
			mDirect3DDeviceIMContext->CSSetShader(shader, nullptr, 0);
#endif
		}

		D3D11_STATE_CACHE_INLINE void getComputeShader(ID3D11ComputeShader** computeShader)
		{
#if D3D11_ALLOW_STATE_CACHE
			*computeShader = mCurrentComputeShader;
			if (mCurrentComputeShader)
			{
				mCurrentComputeShader->AddRef();
			}
#else
			mDirect3DDeviceIMContext->CSGetShader(computeShader, nullptr, nullptr);
#endif

		}

		D3D11_STATE_CACHE_INLINE void setGeometryShader(ID3D11GeometryShader* shader)
		{
#if D3D11_ALLOW_STATE_CACHE
			D3D11_STATE_CACHE_VERIFY_PRE();
			if (mCurrentGeometryShader != shader)
			{
				mCurrentGeometryShader = shader;
				mDirect3DDeviceIMContext->GSSetShader(shader, nullptr, 0);
			}
			D3D11_STATE_CACHE_VERIFY_POST();
#else
			mDirect3DDeviceIMContext->GSSetShader(shader, nullptr, 0);
#endif
		}

		D3D11_STATE_CACHE_INLINE void getGeometryShader(ID3D11GeometryShader** geometryShader)
		{
#if D3D11_ALLOW_STATE_CACHE
			*geometryShader = mCurrentGeometryShader;
			if (mCurrentGeometryShader)
			{
				mCurrentGeometryShader->AddRef();
			}
#else
			mDirect3DDeviceIMContext->GSGetShader(geometryShader, nullptr, nullptr);
#endif
		}

		D3D11_STATE_CACHE_INLINE void setStreamSource(ID3D11Buffer* vertexBuffer, uint32 streamIndex, uint32 stride, uint32 offset)
		{
			internalSetStreamSource(vertexBuffer, streamIndex, stride, offset, nullptr);
		}

		D3D11_STATE_CACHE_INLINE void setStreamSource(ID3D11Buffer* vertexBuffer, uint32 streamIndex, uint32 offset)
		{
			internalSetStreamSource(vertexBuffer, streamIndex, mStreamStrides[streamIndex], offset, nullptr);
		}

		D3D11_STATE_CACHE_INLINE void setIndexBuffer(ID3D11Buffer* indexBuffer, DXGI_FORMAT format, uint32 offset)
		{
			internalSetIndexBuffer(indexBuffer, format, offset, nullptr);
		}

		template<EShaderFrequency ShaderFrequency>
		D3D11_STATE_CACHE_INLINE void setShaderResourceView(ID3D11ShaderResourceView* srv, uint32 resourceIndex, ESRV_Type srvType = SRV_Unknown)
		{
			internalSetShaderResourceView<ShaderFrequency>(srv, resourceIndex, srvType, nullptr);
		}


		template<EShaderFrequency ShaderFrequency>
		D3D11_STATE_CACHE_INLINE void getShaderResourceViews(uint32 startResourceIndex, uint32 numResources, ID3D11ShaderResourceView** srv)
		{
#if D3D11_ALLOW_STATE_CACHE
			{
				BOOST_ASSERT(startResourceIndex + numResources <= D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
				for (uint32 resourceLoop = 0; resourceLoop < numResources; resourceLoop++)
				{
					srv[resourceLoop] = mCurrentShaderResourceView[ShaderFrequency][resourceLoop + startResourceIndex];
					if (srv[resourceLoop])
					{
						srv[resourceLoop]->AddRef();
					}
				}
			}
#else
			{
				switch (ShaderFrequency)
				{
				case SF_Vertex:
					mDirect3DDeviceIMContext->VSGetShaderResources(startResourceIndex, numResources, srv);
					break;

				case SF_Hull:
					mDirect3DDeviceIMContext->HSGetShaderResources(startResourceIndex, numResources, srv);
					break;
				case SF_Domain:
					mDirect3DDeviceIMContext->DSGetShaderResources(startResourceIndex, numResources, srv);
					break;

				case SF_Geometry:
					mDirect3DDeviceIMContext->GSGetShaderResources(startResourceIndex, numResources, srv);
					break;
				case SF_Pixel:
					mDirect3DDeviceIMContext->PSGetShaderResources(startResourceIndex, numResources, srv);
					break;
				case SF_Compute:
					mDirect3DDeviceIMContext->CSGetShaderResources(startResourceIndex, numResources, srv);
					break;
				default:

					break;
				}
			}
#endif
		}
		template<EShaderFrequency ShaderFrequency>
		D3D11_STATE_CACHE_INLINE void setConstantBuffer(ID3D11Buffer* constantBuffer, uint32 slotIndex)
		{
#if D3D11_ALLOW_STATE_CACHE
			D3D11_STATE_CACHE_VERIFY_PRE();
			D3D11ConstantBufferState& current = mCurrentConstantBuffers[ShaderFrequency][slotIndex];
			if ((current.mBuffer != constantBuffer || current.mFirstConstant != 0 || current.mNumConstants != D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT) || GD3D11SkipStateCaching)
			{
				current.mBuffer = constantBuffer;
				current.mFirstConstant = 0;
				current.mNumConstants = D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT;
				internalSetSetConstantBuffer<ShaderFrequency>(slotIndex, constantBuffer);
			}
			D3D11_STATE_CACHE_VERIFY_POST();
#else
			internalSetSetConstantBuffer<ShaderFrequency>(slotIndex, constantBuffer);
#endif
		}


		
		template<EShaderFrequency ShaderFrequency>
		D3D11_STATE_CACHE_INLINE void getConstantBuffers(uint32 startIndex, uint32 numBuffer, ID3D11Buffer** constantBuffers)
		{
#if D3D11_ALLOW_STATE_CACHE
			BOOST_ASSERT(startIndex + numBuffer <= D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
			for (uint32 constantLoop = 0; constantLoop < numBuffer; constantLoop++)
			{
				D3D11ConstantBufferState& cb = mCurrentConstantBuffers[ShaderFrequency][startIndex + constantLoop];
				constantBuffers[constantLoop] = cb.mBuffer;
				if (constantBuffers[constantLoop])
				{
					constantBuffers[constantLoop]->AddRef();
				}
			}
#else
			switch (ShaderFrequency)
			{
			case SF_Vertex:
				mDirect3DDeviceIMContext->VSGetConstantBuffers(startIndex, numBuffer, constantBuffers);
				break;
			case SF_Hull:
				mDirect3DDeviceIMContext->HSGetConstantBuffers(startIndex, numBuffer, constantBuffers);
				break;
			case SF_Domain:
				mDirect3DDeviceIMContext->DSGetConstantBuffers(startIndex, numBuffer, constantBuffers);
				break;
			case SF_Geometry:
				mDirect3DDeviceIMContext->GSGetConstantBuffers(startIndex, numBuffer, constantBuffers);
				break;
			case SF_Pixel:
				mDirect3DDeviceIMContext->PSGetConstantBuffers(startIndex, numBuffer, constantBuffers);
				break;
			case SF_Compute:
				mDirect3DDeviceIMContext->CSGetConstantBuffers(startIndex, numBuffer, constantBuffers);
				break;
			default:
				break;
			}
#endif
		}

		D3D11_STATE_CACHE_INLINE void setDepthStencilState(ID3D11DepthStencilState* state, uint32 refStencil)
		{
#if D3D11_ALLOW_STATE_CACHE
			D3D11_STATE_CACHE_VERIFY_PRE();
			if ((mCurrentDepthStencilState != state || mCurrentReferenceStencil != refStencil) || GD3D11SkipStateCaching)
			{
				mCurrentDepthStencilState = state;
				mCurrentReferenceStencil = refStencil;
				mDirect3DDeviceIMContext->OMSetDepthStencilState(state, refStencil);
			}
			D3D11_STATE_CACHE_VERIFY_POST()
#else
			mDirect3DDeviceIMContext->OMSetDepthStencilState(state, refStencil);
#endif

		}


		D3D11_STATE_CACHE_INLINE void getDepthStencilState(ID3D11DepthStencilState** depthStencilState, uint32* StencilRef)
		{
#if D3D11_ALLOW_STATE_CACHE
			*depthStencilState = mCurrentDepthStencilState;
			*StencilRef = mCurrentReferenceStencil;
			if (mCurrentDepthStencilState)
			{
				mCurrentDepthStencilState->AddRef();
			}
#else
			mDirect3DDeviceIMContext->OMGetDepthStencilState(depthStencilState, StencilRef);
#endif
		}

		D3D11_STATE_CACHE_INLINE void setBlendState(ID3D11BlendState* blendState, const float* blendFactor, uint32 sampleMask)
		{
#if D3D11_ALLOW_STATE_CACHE
			D3D11_STATE_CACHE_VERIFY_PRE();
			if ((mCurrentBlendState != blendState || Memory::memcmp(blendFactor, mCurrentBlendFactor, sizeof(mCurrentBlendFactor)) != 0 || mCurrentBlendSampleMask != sampleMask) || GD3D11SkipStateCaching)
			{
				mCurrentBlendState = blendState;
				Memory::memcpy(mCurrentBlendFactor, blendFactor, sizeof(mCurrentBlendFactor));
				mCurrentBlendSampleMask = sampleMask;
				mDirect3DDeviceIMContext->OMSetBlendState(blendState, blendFactor, sampleMask);
			}
			D3D11_STATE_CACHE_VERIFY_POST();
#else
			mDirect3DDeviceIMContext->OMSetBlendState(blendState, blendFactor, sampleMask);
#endif
		}


		D3D11_STATE_CACHE_INLINE void getBlendState(ID3D11BlendState** blendState, float* blendFactor, uint32* sampleMask)
		{
#if D3D11_ALLOW_STATE_CACHE
			*blendState = mCurrentBlendState;
			if (mCurrentBlendState)
			{
				mCurrentBlendState->AddRef();
			}
			*sampleMask = mCurrentBlendSampleMask;
			Memory::memcpy(blendFactor, mCurrentBlendFactor, sizeof(mCurrentBlendFactor));
#else
			mDirect3DDeviceIMContext->OMGetBlendState(blendState, blendFactor, sampleMask);
#endif
		}

		D3D11_STATE_CACHE_INLINE void setRasterizerState(ID3D11RasterizerState* rasterizerState)
		{
#if D3D11_ALLOW_STATE_CACHE
			D3D11_STATE_CACHE_VERIFY_PRE();

			if (mCurrentRasterizerState != rasterizerState || GD3D11SkipStateCaching)
			{
				mCurrentRasterizerState = rasterizerState;
				mDirect3DDeviceIMContext->RSSetState(rasterizerState);
			}

			D3D11_STATE_CACHE_VERIFY_POST();

#else
			mDirect3DDeviceIMContext->RSSetState(rasterizerState);
#endif
		}


		D3D11_STATE_CACHE_INLINE void getRasterizerState(ID3D11RasterizerState** rasterizerState)
		{
#if D3D11_ALLOW_STATE_CACHE
			*rasterizerState = mCurrentRasterizerState;
			if (mCurrentRasterizerState)
			{
				mCurrentRasterizerState->AddRef();
			}
#else
			mDirect3DDeviceIMContext->RSGetState(rasterizerState);
#endif
		}

		D3D11_STATE_CACHE_INLINE void setInputLayout(ID3D11InputLayout* inputLayout)
		{
#if D3D11_ALLOW_STATE_CACHE
			D3D11_STATE_CACHE_VERIFY_PRE();
			if (mCurrentInputLayout != inputLayout || GD3D11SkipStateCaching)
			{
				mCurrentInputLayout = inputLayout;
				mDirect3DDeviceIMContext->IASetInputLayout(inputLayout);
			}
			D3D11_STATE_CACHE_VERIFY_POST();
#else
			mDirect3DDeviceIMContext->IASetInputLayout(inputLayout);
#endif
		}

		D3D11_STATE_CACHE_INLINE void getInputLayout(ID3D11InputLayout** inputLayout)
		{
#if D3D11_ALLOW_STATE_CACHE 
			*inputLayout = mCurrentInputLayout;
			if (mCurrentInputLayout)
			{
				mCurrentInputLayout->AddRef();
			}
#else
			mDirect3DDeviceIMContext->IAGetInputLayout(inputLayout);
#endif
		}

		template<EShaderFrequency ShaderFrequency>
		D3D11_STATE_CACHE_INLINE void internalSetSamplerState(uint32 samplerIndex, ID3D11SamplerState*& samplerState)
		{
			CA_SUPPRESS(6326);
			switch (ShaderFrequency)
			{
			case SF_Vertex: mDirect3DDeviceIMContext->VSSetSamplers(samplerIndex, 1, &samplerState);
				break;
			case SF_Hull: mDirect3DDeviceIMContext->HSSetSamplers(samplerIndex, 1, &samplerState);
				break;
			case Air::SF_Domain: mDirect3DDeviceIMContext->DSSetSamplers(samplerIndex, 1, &samplerState);
				break;
			case Air::SF_Pixel: mDirect3DDeviceIMContext->PSSetSamplers(samplerIndex, 1, &samplerState);
				break;
			case Air::SF_Geometry: mDirect3DDeviceIMContext->GSSetSamplers(samplerIndex, 1, &samplerState);
				break;
			case Air::SF_Compute: mDirect3DDeviceIMContext->CSSetSamplers(samplerIndex, 1, &samplerState);
				break;
			}
		}

		typedef void(*TSetSamplerStateAlternate) (D3D11StateCacheBase* stateCache, ID3D11SamplerState* samplerState, uint32 samplerIndex);

		template<EShaderFrequency ShaderFrequency>
		D3D11_STATE_CACHE_INLINE void internalSetSamplerState(ID3D11SamplerState* samplerState, uint32 samplerIndex, TSetSamplerStateAlternate alternatePathFunction)
		{
#if D3D11_ALLOW_STATE_CACHE
			D3D11_STATE_CACHE_VERIFY_PRE();
			BOOST_ASSERT(samplerIndex < ARRAYSIZE(mCurrentSamplerStates[ShaderFrequency]));
			if ((mCurrentSamplerStates[ShaderFrequency][samplerIndex] != samplerState) || GD3D11SkipStateCaching)
			{
				mCurrentSamplerStates[ShaderFrequency][samplerIndex] = samplerState;
				if (alternatePathFunction != nullptr)
				{
					(*alternatePathFunction)(this, samplerState, samplerIndex);
				}
				else
				{
					internalSetSamplerState<ShaderFrequency>(samplerIndex, samplerState);
				}
			}
			D3D11_STATE_CACHE_VERIFY_POST();
#else
			internalSetSamplerState<ShaderFrequency>(samplerIndex, samplerState);
#endif

		}


		template<EShaderFrequency ShaderFrequency>
		D3D11_STATE_CACHE_INLINE void setSamplerState(ID3D11SamplerState* samplerState, uint32 samplerIndex)
		{
			internalSetSamplerState<ShaderFrequency>(samplerState, samplerIndex, nullptr);
		}
		void clearState();

		D3D11_STATE_CACHE_INLINE void setPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY primitiveTopology)
		{
#if D3D11_ALLOW_STATE_CACHE
			D3D11_STATE_CACHE_VERIFY_PRE();
			if ((mCurrentPrimitiveTopology != primitiveTopology) || GD3D11SkipStateCaching)
			{
				mCurrentPrimitiveTopology = primitiveTopology;
				mDirect3DDeviceIMContext->IASetPrimitiveTopology(primitiveTopology);
			}
			D3D11_STATE_CACHE_VERIFY_POST();
#else
			mDirect3DDeviceIMContext->IASetPrimitiveTopology(primitiveTopology);
#endif
		}
	private:
		typedef void(*TSetStreamSourceAlternate)(D3D11StateCacheBase* stateCache, ID3D11Buffer* vertexBuffer, uint32 streamIndex, uint32 stride, uint32 offset);


		D3D11_STATE_CACHE_INLINE void internalSetStreamSource(ID3D11Buffer* vertexBuffer, uint32 streamIndex, uint32 stride, uint32 offset, TSetStreamSourceAlternate alternatePathFunction)
		{
#if D3D11_ALLOW_STATE_CACHE
			D3D11_STATE_CACHE_VERIFY_PRE();
			D3D11VertexBufferState& slot = mCurrentVertexBuffers[streamIndex];
			if ((slot.mVertexBuffer != vertexBuffer || slot.mOffset != offset || slot.mStride != stride))
			{
				slot.mVertexBuffer = vertexBuffer;
				slot.mOffset = offset;
				slot.mStride = stride;
				if (alternatePathFunction != nullptr)
				{
					(*alternatePathFunction)(this, vertexBuffer, streamIndex, stride, offset);
				}
				else
				{
					mDirect3DDeviceIMContext->IASetVertexBuffers(streamIndex, 1, &vertexBuffer, &stride, &offset);
				}
			}
			D3D11_STATE_CACHE_VERIFY_POST();
#else
			mDirect3DDeviceIMContext->IASetVertexBuffers(streamIndex, 1, &vertexBuffer, &stride, &offset);
#endif
		}

		typedef void(*TSetIndexBufferAlternate)(D3D11StateCacheBase* stateCache, ID3D11Buffer* indexBuffer, DXGI_FORMAT format, uint32 offset);

		D3D11_STATE_CACHE_INLINE void internalSetIndexBuffer(ID3D11Buffer* indexBuffer, DXGI_FORMAT format, uint32 offset, TSetIndexBufferAlternate alternatePathFunction)
		{
#if D3D11_ALLOW_STATE_CACHE
			D3D11_STATE_CACHE_VERIFY_PRE();
			if (mAlwaysSetIndexBuffers || (mCurrentIndexBuffer != indexBuffer || mCurrentIndexFormat != format || mCurrentIndexOffset != offset))
			{
				mCurrentIndexBuffer = indexBuffer;
				mCurrentIndexFormat = format;
				mCurrentIndexOffset = offset;
				if (alternatePathFunction != nullptr)
				{
					(*alternatePathFunction)(this, indexBuffer, format, offset);
				}
				else
				{
					mDirect3DDeviceIMContext->IASetIndexBuffer(indexBuffer, format, offset);
				}
			}
			D3D11_STATE_CACHE_VERIFY_POST();
#else
			mDirect3DDeviceIMContext->IASetIndexBuffer(indexBuffer, format, offset);
#endif
		}

		template<EShaderFrequency ShaderFrequency>
		D3D11_STATE_CACHE_INLINE void internalSetShaderResourceView(uint32 resourceIndex, ID3D11ShaderResourceView*& srv)
		{
			CA_SUPPRESS(6326);
			switch (ShaderFrequency)
			{
			case Air::SF_Vertex:
				mDirect3DDeviceIMContext->VSSetShaderResources(resourceIndex, 1, &srv);
				break;
			case Air::SF_Hull:
				mDirect3DDeviceIMContext->HSSetShaderResources(resourceIndex, 1, &srv);
				break;
			case Air::SF_Domain:
				mDirect3DDeviceIMContext->DSSetShaderResources(resourceIndex, 1, &srv);
				break;
			case Air::SF_Pixel:
				mDirect3DDeviceIMContext->PSSetShaderResources(resourceIndex, 1, &srv);
				break;
			case Air::SF_Geometry:
				mDirect3DDeviceIMContext->GSSetShaderResources(resourceIndex, 1, &srv);
				break;
			case Air::SF_Compute:
				mDirect3DDeviceIMContext->CSSetShaderResources(resourceIndex, 1, &srv);
				break;
			}
		}

		typedef void(*TSetSRVAlternate)(D3D11StateCacheBase* stateCache, ID3D11ShaderResourceView* srv, uint32 resourceIndex, ESRV_Type type);
		template<EShaderFrequency ShaderFrequency>
		D3D11_STATE_CACHE_INLINE void internalSetShaderResourceView(ID3D11ShaderResourceView*& srv, uint32 resourceIndex, ESRV_Type srvType, TSetSRVAlternate alternatePathFunction)
		{
#if D3D11_ALLOW_STATE_CACHE
			D3D11_STATE_CACHE_VERIFY_PRE();
			if ((mCurrentShaderResourceView[ShaderFrequency][resourceIndex] != srv))
			{
				if (srv)
				{
					srv->AddRef();
				}
				if (mCurrentShaderResourceView[ShaderFrequency][resourceIndex])
				{
					mCurrentShaderResourceView[ShaderFrequency][resourceIndex]->Release();
				}
				mCurrentShaderResourceView[ShaderFrequency][resourceIndex] = srv;
				if (alternatePathFunction != nullptr)
				{
					(*alternatePathFunction)(this, srv, resourceIndex, srvType);
				}
				else
				{
					internalSetShaderResourceView<ShaderFrequency>(resourceIndex, srv);
				}
			}
			D3D11_STATE_CACHE_VERIFY_POST();
#else
			internalSetShaderResourceView<ShaderFrequency>(resourceIndex, srv);
#endif
		}

		template<EShaderFrequency ShaderFrequency>
		D3D11_STATE_CACHE_INLINE void internalSetSetConstantBuffer(uint32 slotIndex, ID3D11Buffer* & constantBuffer)
		{
			CA_SUPPRESS(6326);
			switch (ShaderFrequency)
			{
			case Air::SF_Vertex:
				mDirect3DDeviceIMContext->VSSetConstantBuffers(slotIndex, 1, &constantBuffer);
				break;
			case Air::SF_Hull:
				mDirect3DDeviceIMContext->HSSetConstantBuffers(slotIndex, 1, &constantBuffer);
				break;
			case Air::SF_Domain:
				mDirect3DDeviceIMContext->DSSetConstantBuffers(slotIndex, 1, &constantBuffer);
				break;
			case Air::SF_Pixel:
				mDirect3DDeviceIMContext->PSSetConstantBuffers(slotIndex, 1, &constantBuffer);
				break;
			case Air::SF_Geometry:
				mDirect3DDeviceIMContext->GSSetConstantBuffers(slotIndex, 1, &constantBuffer);
				break;
			case Air::SF_Compute:
				mDirect3DDeviceIMContext->CSSetConstantBuffers(slotIndex, 1, &constantBuffer);
				break;
			}
		}


	private:
		bool mAlwaysSetIndexBuffers;

	protected:
		ID3D11DeviceContext* mDirect3DDeviceIMContext{ nullptr };
#if D3D11_ALLOW_STATE_CACHE
		ID3D11ShaderResourceView* mCurrentShaderResourceView[SF_NumFrequencies][D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{ nullptr };
		ID3D11RasterizerState* mCurrentRasterizerState{ nullptr };
		uint32 mCurrentReferenceStencil{ 0 };
		ID3D11DepthStencilState* mCurrentDepthStencilState{ nullptr };

		ID3D11VertexShader* mCurrentVertexShader{ nullptr };
		ID3D11HullShader* mCurrentHullShader{ nullptr };
		ID3D11DomainShader* mCurrentDomainShader{ nullptr };
		ID3D11GeometryShader* mCurrentGeometryShader{ nullptr };
		ID3D11PixelShader*	mCurrentPixelShader;
		ID3D11ComputeShader* mCurrentComputeShader;

		float mCurrentBlendFactor[4];
		uint32	mCurrentBlendSampleMask;
		ID3D11BlendState*	mCurrentBlendState;

		uint32 mCurrentNumberOfViewports;
		D3D11_VIEWPORT mCurrentViewport[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		struct D3D11VertexBufferState 
		{
			ID3D11Buffer*	mVertexBuffer;
			uint32			mStride;
			uint32			mOffset;
		}mCurrentVertexBuffers[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];

		ID3D11Buffer* mCurrentIndexBuffer;
		DXGI_FORMAT	mCurrentIndexFormat;
		uint32 mCurrentIndexOffset;
		uint16 mStreamStrides[MaxVertexElementCount];

		D3D11_PRIMITIVE_TOPOLOGY	mCurrentPrimitiveTopology;

		ID3D11InputLayout* mCurrentInputLayout;

		ID3D11SamplerState* mCurrentSamplerStates[SF_NumFrequencies][D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];

		struct D3D11ConstantBufferState
		{
			ID3D11Buffer* mBuffer;
			uint32 mFirstConstant;
			uint32 mNumConstants;
		}mCurrentConstantBuffers[SF_NumFrequencies][D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];




#endif
	};

	typedef D3D11StateCacheBase D3D11StateCache;
}