#include "RHI.h"
#include "Misc/App.h"
#include "D3D11DynamicRHI.h"
#include "RenderingThread.h"
#include "HAL/PlatformMisc.h"
#include "Serialization/MemoryReader.h"
#include "OneColorShader.h"
#include "GenericPlatform/genericPlatformDriver.h"
#include "D3D11Util.h"
#include "RenderResource.h"
#include "HardwareInfo.h"
#include "NVIDIA/nvapi/nvapi.h"
#include "HAL/PlatformProperties.h"
#include "D3D11State.h"
#include "AMD/AMD_AGS/inc/amd_ags.h"
#include "IHeadMountedDisplayModule.h"
#include "GenericPlatform/genericPlatformDriver.h"
#include "D3D11Util.h"
#include "RHICommandList.h"
#include "RHIUtilities.h"
#include "D3D11RHI.h"
#include "NVIDIA/nvapi/nvapi.h"
#include "Template/AirTemplate.h"
#include "D3D11UnorderedAccessView.h"
#include "RenderUtils.h"
#include "D3D11Viewport.h"
#include "RHIStaticStates.h"
#include "GlobalShader.h"
#include "D3D11Shader.h"
#include <delayimp.h>
#include "ResolveShader.h"
#include "D3D11UniformBuffer.h"
#include "StaticBoundShaderState.h"
#include "ScreenRendering.h"
#include "PipelineStateCache.h"
namespace Air
{
	int32 GDX11ForcedGPUs = -1;





	bool D3D11RHI_ShouldCreateWithD3DDebug()
	{
#ifdef _DEBUG
		return true;
#else
		return false;
#endif
	}

	struct RTVDesc
	{
		uint32 width;
		uint32 height;
		DXGI_SAMPLE_DESC mSampleDesc;
	};

	RTVDesc getRenderTargetViewDesc(ID3D11RenderTargetView* renderView)
	{
		D3D11_RENDER_TARGET_VIEW_DESC targetDesc;
		renderView->GetDesc(&targetDesc);
		TRefCountPtr<ID3D11Resource> baseResource;
		renderView->GetResource((ID3D11Resource**)baseResource.getInitReference());
		uint32 mipIndex = 0;
		RTVDesc ret;
		memset(&ret, 0, sizeof(ret));
		switch (targetDesc.ViewDimension)
		{
		case D3D11_RTV_DIMENSION_TEXTURE2D:
		case D3D11_RTV_DIMENSION_TEXTURE2DMS:
		case D3D11_RTV_DIMENSION_TEXTURE2DARRAY:
		case D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY:
		{
			D3D11_TEXTURE2D_DESC desc;
			((ID3D11Texture2D*)(baseResource.getReference()))->GetDesc(&desc);
			ret.width = desc.Width;
			ret.height = desc.Height;
			ret.mSampleDesc = desc.SampleDesc;
			if (targetDesc.ViewDimension == D3D11_RTV_DIMENSION_TEXTURE2D || targetDesc.ViewDimension == D3D11_RTV_DIMENSION_TEXTURE2DARRAY)
			{
				mipIndex = targetDesc.Texture2D.MipSlice;
			}
			break;
		}
		case D3D11_RTV_DIMENSION_TEXTURE3D:
		{
			D3D11_TEXTURE3D_DESC desc;
			((ID3D11Texture3D*)(baseResource.getReference()))->GetDesc(&desc);
			ret.width = desc.Width;
			ret.height = desc.Height;
			ret.mSampleDesc.Count = 1;
			ret.mSampleDesc.Quality = 0;
			mipIndex = targetDesc.Texture3D.MipSlice;
			break;
		}
		default:
			break;
		}
		ret.width >>= mipIndex;
		ret.height >>= mipIndex;
		return ret;
	}

	static ResolveRect getDefaultRect(const ResolveRect& rect, uint32 defaultWidth, uint32 defaultHeight)
	{
		if (rect.X1 >= 0 && rect.X2 >= 0 && rect.Y1 >= 0 && rect.Y2 >= 0)
		{
			return rect;
		}
		else
		{
			return ResolveRect(0, 0, defaultWidth, defaultHeight);
		}
	}

	static inline DXGI_FORMAT convertTypelessToUnorm(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			return DXGI_FORMAT_R8G8B8A8_UNORM;

		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			return DXGI_FORMAT_B8G8R8A8_UNORM;
		default:
			return format;
		}
	}
	static D3D11_PRIMITIVE_TOPOLOGY getD3D11PrimitiveType(uint32 primitiveType, bool bUsingTessellation)
	{
		if (bUsingTessellation)
		{
			switch (primitiveType)
			{
			case PT_1_ControlPointPatchList :
				return D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
			case PT_2_ControlPointPatchList:
				return D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
			case PT_TriangleList: 
				return D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;

			case PT_LineList:
			case PT_TriangleStrip:
			case PT_QuadList:
			case PT_PointList:
				BOOST_ASSERT(false);
				break;
			default:
				break;
			}
		}
		switch (primitiveType)
		{
		case PT_TriangleList: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case  PT_TriangleStrip: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		case PT_LineList: return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		case PT_PointList: return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		case PT_3_ControlPointPatchList: 
			return D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
		case PT_4_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
		case PT_5_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST;
		case PT_6_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST;
		case PT_7_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST;
		case PT_8_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST;
		case PT_9_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST;
		case PT_10_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST;
		case PT_11_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST;
		case PT_12_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST;
		case PT_13_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST;
		case PT_14_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST;
		case PT_15_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST;
		case PT_16_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;
		case PT_17_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST;
		case PT_18_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST;
		case PT_19_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST;
		case PT_20_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST;
		case PT_21_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST;
		case PT_22_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST;
		case PT_23_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST;
		case PT_24_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST;
		case PT_25_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
		case PT_26_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST;
		case PT_27_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST;
		case PT_28_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST;
		case PT_29_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST;
		case PT_30_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST;
		case PT_31_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST;
		case PT_32_ControlPointPatchList:
			return D3D11_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST;
		default:
			break;
		}
		return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}

	static uint32 countAdapterOutputs(IDXGIAdapter* adapter)
	{
		uint32 outputCount = 0;
		for (;;)
		{
			IDXGIOutput* output;
			HRESULT hr = adapter->EnumOutputs(outputCount, &output);
			if (FAILED(hr))
			{
				break;
			}
			++outputCount;
		}
		return outputCount;
	}


	static D3D_FEATURE_LEVEL getAllowedD3DFeatureLevel()
	{
		D3D_FEATURE_LEVEL allowedFeatureLevel = D3D_FEATURE_LEVEL_11_0;
		return allowedFeatureLevel;
	}

	static void safeCreateDXGIFactory(IDXGIFactory1**dxgiFactory1)
	{
		::CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)dxgiFactory1);
	}

	static bool isDelayLoadException(PEXCEPTION_POINTERS exceptionPointers)
	{
#if WINVER > 0x502
		switch (exceptionPointers->ExceptionRecord->ExceptionCode)
		{
		case VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND):
		case VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND):
			return EXCEPTION_EXECUTE_HANDLER;
		default:
			return EXCEPTION_CONTINUE_SEARCH;
		}
#else
		return EXCEPTION_EXECUTE_HANDLER;
#endif
	}
	static bool safeTestD3D11CreateDevice(IDXGIAdapter* adapter, D3D_FEATURE_LEVEL maxFeatureLevel, D3D_FEATURE_LEVEL * outFeatureLevel)
	{
		ID3D11Device* d3ddevice = nullptr;
		ID3D11DeviceContext* D3DDeviceContext = nullptr;
		uint32 deviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
		if (D3D11RHI_ShouldCreateWithD3DDebug())
		{
			deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		}
		D3D_FEATURE_LEVEL requestedFeatureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_0
		};
		int32 firstAllowedFeatureLevel = 0;
		int32 numAllowedFeatureLevels = ARRAY_COUNT(requestedFeatureLevels);
		while (firstAllowedFeatureLevel < numAllowedFeatureLevels)
		{
			if (requestedFeatureLevels[firstAllowedFeatureLevel] == maxFeatureLevel)
			{
				break;
			}
			firstAllowedFeatureLevel++;
		}
		numAllowedFeatureLevels -= firstAllowedFeatureLevel;
		if (numAllowedFeatureLevels == 0)
		{
			return false;
		}
		__try
		{
			if (SUCCEEDED(D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, deviceFlags, &requestedFeatureLevels[firstAllowedFeatureLevel], numAllowedFeatureLevels, D3D11_SDK_VERSION, &d3ddevice, outFeatureLevel, &D3DDeviceContext)));
			{
				d3ddevice->Release();
				D3DDeviceContext->Release();
				return true;
			}
		}
		__except (isDelayLoadException(GetExceptionInformation()))
		{
			CA_SUPPRESS(6322);
		}
		return false;
	}

	bool D3D11RHI_ShouldAllowAsyncResourceCreation()
	{
		static bool bAllowAsyncResourceCreation = true;
		return bAllowAsyncResourceCreation;
	}

	static bool supportsHDROutput(D3D11DynamicRHI* D3D11RHI)
	{
		return false;
	}


	 template<EShaderFrequency Frequency>
	 FORCEINLINE void setResource(D3D11DynamicRHI* RESTRICT d3d11RHI, D3D11StateCache* RESTRICT stateCache, uint32 bindIndex, D3D11BaseShaderResource* RESTRICT shaderResource, ID3D11ShaderResourceView* RESTRICT SRV, wstring resourceName = TEXT(""))
	 {
		 d3d11RHI->setShaderResourceView<Frequency>(shaderResource, SRV, bindIndex, resourceName, D3D11StateCache::SRV_Unknown);
	 }

	 template<EShaderFrequency Frequency>
	 FORCEINLINE void setResource(D3D11DynamicRHI* RESTRICT d3d11RHI, D3D11StateCache* RESTRICT stateCache, uint32 bindIndex, ID3D11SamplerState* RESTRICT samplerState)
	 {
		 stateCache->setSamplerState<Frequency>(samplerState, bindIndex);
	 }


	template<EShaderFrequency ShaderFrequency>
	inline int32 setShaderResourcesFromBuffer_Surface(D3D11DynamicRHI* RESTRICT D3D11RHI, D3D11StateCache* RESTRICT stateCache, D3D11ConstantBuffer* RESTRICT buffer, const uint32* RESTRICT resourceMap, int32 bufferIndex)
	{
		const TRefCountPtr<RHIResource>* RESTRICT resources = buffer->mResourceTable.getData();
		float currentTime = App::getCurrentTime();
		int32 numSetCalls = 0;
		uint32 bufferOffset = resourceMap[bufferIndex];
		if (bufferOffset > 0)
		{
			const uint32 * RESTRICT resourceInfos = &resourceMap[bufferOffset];
			uint32 resourceInfo = *resourceInfos++;
			do
			{
				BOOST_ASSERT(RHIResourceTableEntry::getConstantBufferIndex(resourceInfo) == bufferIndex);
				const uint16 resourceIndex = RHIResourceTableEntry::getResourceIndex(resourceInfo);
				const uint8 bindIndex = RHIResourceTableEntry::getBindIndex(resourceInfo);
				D3D11BaseShaderResource* shaderResource = nullptr;
				ID3D11ShaderResourceView* d3d11Resource = nullptr;
				RHITexture* textureRHI = (RHITexture*)resources[resourceIndex].getReference();
				textureRHI->setLastRenderTime(currentTime);
				D3D11TextureBase* textureD3D11 = getD3D11TextureFromRHITexture(textureRHI);
				shaderResource = textureD3D11->getBaseShaderResource();
				d3d11Resource = textureD3D11->getShaderResourceView();
				setResource<ShaderFrequency>(D3D11RHI, stateCache, bindIndex, shaderResource, d3d11Resource, textureRHI->getName());
				numSetCalls++;
				resourceInfo = *resourceInfos++;
			} while (RHIResourceTableEntry::getConstantBufferIndex(resourceInfo) == bufferIndex);
		}
		return numSetCalls;
	}


	template<EShaderFrequency ShaderFrequency>
	inline int32 setShaderResourcesFromBuffer_SRV(D3D11DynamicRHI* RESTRICT D3D11RHI, D3D11StateCache* RESTRICT stateCache, D3D11ConstantBuffer* RESTRICT buffer, const uint32* RESTRICT resourceMap, int32 bufferIndex)
	{
		const TRefCountPtr<RHIResource>* RESTRICT resources = buffer->mResourceTable.getData();
		float currentTime = App::getCurrentTime();
		int32 numSetCalls = 0;
		uint32 bufferOffset = resourceMap[bufferIndex];
		if (bufferOffset > 0)
		{
			const uint32* RESTRICT resourceInfos = &resourceMap[bufferOffset];
			uint32 resourceInfo = *resourceInfos++;
			do
			{
				BOOST_ASSERT(RHIResourceTableEntry::getConstantBufferIndex(resourceInfo) == bufferIndex);
				const uint16 resourceIndex = RHIResourceTableEntry::getResourceIndex(resourceInfo);
				const uint8 bindIndex = RHIResourceTableEntry::getBindIndex(resourceInfo);
				D3D11BaseShaderResource* shaderResource = nullptr;
				ID3D11ShaderResourceView* d3d11Resource = nullptr;
				D3D11ShaderResourceView* shaderResourceViewRHI = (D3D11ShaderResourceView*)resources[resourceIndex].getReference();
				shaderResource = shaderResourceViewRHI->mResource.getReference();
				d3d11Resource = shaderResourceViewRHI->mView.getReference();
				setResource<ShaderFrequency>(D3D11RHI, stateCache, bindIndex, shaderResource, d3d11Resource);
				numSetCalls++;
				resourceInfo = *resourceInfos++;
			} while (RHIResourceTableEntry::getConstantBufferIndex(resourceInfo) == bufferIndex);
		}
		return numSetCalls;
	}

	template <EShaderFrequency ShaderFrequency>
	inline int32 setShaderResourcesFromBuffer_Sampler(D3D11DynamicRHI* RESTRICT D3D11RHi, D3D11StateCache* RESTRICT stateCache, D3D11ConstantBuffer* RESTRICT buffer, const uint32 * RESTRICT resourceMap, int32 bufferIndex)
	{
		const TRefCountPtr<RHIResource>* RESTRICT resources = buffer->mResourceTable.getData();
		uint32 numSetCalls = 0;
		uint32 bufferOffset = resourceMap[bufferIndex];
		if (bufferOffset > 0)
		{
			const uint32* RESTRICT resourceInfos = &resourceMap[bufferOffset];
			uint32 resourceInfo = *resourceInfos++;
			do
			{
				BOOST_ASSERT(RHIResourceTableEntry::getConstantBufferIndex(resourceInfo) == bufferIndex);
				const uint16 resourceIndex = RHIResourceTableEntry::getResourceIndex(resourceInfo);
				const uint8 bindIndex = RHIResourceTableEntry::getBindIndex(resourceInfo);
				ID3D11SamplerState* d3d11Resource = ((D3D11SamplerState*)resources[resourceIndex].getReference())->mResource.getReference();
				setResource<ShaderFrequency>(D3D11RHi, stateCache, bindIndex, d3d11Resource);
				numSetCalls++;
				resourceInfo = *resourceInfos++;
			} while (RHIResourceTableEntry::getConstantBufferIndex(resourceInfo) == bufferIndex);
		}
		return numSetCalls;
	}

	


	void D3D11DynamicRHI::RHISetMultipleViewports(uint32 count, const ViewportBounds* data)
	{
		BOOST_ASSERT(count > 0);
		BOOST_ASSERT(data);
		D3D11_VIEWPORT* d3dData = (D3D11_VIEWPORT*)data;
		mStateCache.setViewports(count, d3dData);
	}

	void D3D11DynamicRHI::RHITransitionResources(EResourceTransitionAccess transitionType, RHITexture** inTexture, int32 numTextures)
	{
		for (int32 i = 0; i < numTextures; i++)
		{
			RHITexture* renderTarget = inTexture[i];
			if (renderTarget)
			{
				D3D11BaseShaderResource* resource = nullptr;
				D3D11Texture2D* sourceTexture2D = static_cast<D3D11Texture2D*>(renderTarget->getTexture2D());
				if (sourceTexture2D)
				{
					resource = sourceTexture2D;
				}
				D3D11Texture2DArray* sourceTexture2DArray = static_cast<D3D11Texture2DArray*>(renderTarget->getTextureArray());
				if (sourceTexture2DArray)
				{
					resource = sourceTexture2DArray;
				}

				D3D11TextureCube* sourceTextureCube = static_cast<D3D11TextureCube*>(renderTarget->getTextureCube());
				if (sourceTextureCube)
				{
					resource = sourceTextureCube;
				}

				resource->setCurrentGPUAccess(transitionType);
			}
		}
	}

	void D3D11DynamicRHI::shutdown()
	{
		
	}

	IRHICommandContext* D3D11DynamicRHI::getDefualtContext()
	{
		return this;
	}

	TCHAR* D3D11DynamicRHI::getName()
	{
		return TEXT("D3D11");
	}

	D3D11DynamicRHI::D3D11DynamicRHI(IDXGIFactory1* inDXGIFactory1, D3D_FEATURE_LEVEL inFeatureLevel, int32 inChosenAdapter, const DXGI_ADAPTER_DESC& chosenDescription)
		: mFeatureLevel(inFeatureLevel)
		, mChosenAdapter(inChosenAdapter)
		, mChosenDescription(chosenDescription)
	{
		mDXGIFactory1 = inDXGIFactory1;

		BOOST_ASSERT(mFeatureLevel == D3D_FEATURE_LEVEL_11_0 || mFeatureLevel == D3D_FEATURE_LEVEL_10_0);
		if (mFeatureLevel == D3D_FEATURE_LEVEL_10_0)
		{
			GSupportsDepthFetchDuringDepthTest = false;
		}


		GPixelFormats[PF_Unknown].PlatformFormat = DXGI_FORMAT_UNKNOWN;
		GPixelFormats[PF_A32B32G32R32F].PlatformFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		GPixelFormats[PF_B8G8R8A8].PlatformFormat = DXGI_FORMAT_B8G8R8A8_TYPELESS;
		GPixelFormats[PF_G8].PlatformFormat = DXGI_FORMAT_R8_UNORM;
		GPixelFormats[PF_G16].PlatformFormat = DXGI_FORMAT_R16_UNORM;
		GPixelFormats[PF_BC1].PlatformFormat = DXGI_FORMAT_BC1_TYPELESS;
		GPixelFormats[PF_BC2].PlatformFormat = DXGI_FORMAT_BC2_TYPELESS;
		GPixelFormats[PF_BC3].PlatformFormat = DXGI_FORMAT_BC3_TYPELESS;
		GPixelFormats[PF_BC4].PlatformFormat = DXGI_FORMAT_BC4_UNORM;
		GPixelFormats[PF_BC4_SNORM].PlatformFormat = DXGI_FORMAT_BC4_SNORM;
		GPixelFormats[PF_UYVY].PlatformFormat = DXGI_FORMAT_UNKNOWN;		// TODO: Not supported in D3D11
#if DEPTH_32_BIT_CONVERSION
		GPixelFormats[PF_DepthStencil].PlatformFormat = DXGI_FORMAT_R32G8X24_TYPELESS;
		GPixelFormats[PF_DepthStencil].BlockBytes = 5;
		GPixelFormats[PF_X24_G8].PlatformFormat = DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
		GPixelFormats[PF_X24_G8].BlockBytes = 5;
#else
		GPixelFormats[PF_DepthStencil].PlatformFormat = DXGI_FORMAT_R24G8_TYPELESS;
		GPixelFormats[PF_DepthStencil].BlockBytes = 4;
		GPixelFormats[PF_X24_G8].PlatformFormat = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
		GPixelFormats[PF_X24_G8].BlockBytes = 4;
#endif
		GPixelFormats[PF_ShadowDepth].PlatformFormat = DXGI_FORMAT_R16_TYPELESS;
		GPixelFormats[PF_ShadowDepth].BlockBytes = 2;
		GPixelFormats[PF_R32_FLOAT].PlatformFormat = DXGI_FORMAT_R32_FLOAT;
		GPixelFormats[PF_G16R16].PlatformFormat = DXGI_FORMAT_R16G16_UNORM;
		GPixelFormats[PF_G16R16F].PlatformFormat = DXGI_FORMAT_R16G16_FLOAT;
		GPixelFormats[PF_G16R16F_FILTER].PlatformFormat = DXGI_FORMAT_R16G16_FLOAT;
		GPixelFormats[PF_G32R32F].PlatformFormat = DXGI_FORMAT_R32G32_FLOAT;
		GPixelFormats[PF_A2B10G10R10].PlatformFormat = DXGI_FORMAT_R10G10B10A2_UNORM;
		GPixelFormats[PF_A2B10G10R10_UINT].PlatformFormat = DXGI_FORMAT_R10G10B10A2_UINT;
		GPixelFormats[PF_A16B16G16R16].PlatformFormat = DXGI_FORMAT_R16G16B16A16_UNORM;
		GPixelFormats[PF_A16B16G16R16_SINT].PlatformFormat = DXGI_FORMAT_R16G16B16A16_SINT;
		GPixelFormats[PF_A16B16G16R16F].PlatformFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		GPixelFormats[PF_D24].PlatformFormat = DXGI_FORMAT_R24G8_TYPELESS;
		GPixelFormats[PF_R16F].PlatformFormat = DXGI_FORMAT_R16_FLOAT;
		GPixelFormats[PF_R16_UNORM].PlatformFormat = DXGI_FORMAT_R16_UNORM;
		GPixelFormats[PF_R16F_FILTER].PlatformFormat = DXGI_FORMAT_R16_FLOAT;

		GPixelFormats[PF_FloatRGB].PlatformFormat = DXGI_FORMAT_R11G11B10_FLOAT;
		GPixelFormats[PF_FloatRGB].BlockBytes = 4;
		GPixelFormats[PF_FloatRGBA].PlatformFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		GPixelFormats[PF_FloatRGBA].BlockBytes = 8;

		GPixelFormats[PF_FloatR11G11B10].PlatformFormat = DXGI_FORMAT_R11G11B10_FLOAT;
		GPixelFormats[PF_FloatR11G11B10].BlockBytes = 4;
		GPixelFormats[PF_FloatR11G11B10].Supported = true;
		GPixelFormats[PF_R4G4B4A4].PlatformFormat = DXGI_FORMAT_B4G4R4A4_UNORM;

		GPixelFormats[PF_V8U8].PlatformFormat = DXGI_FORMAT_R8G8_SNORM;
		GPixelFormats[PF_BC5].PlatformFormat = DXGI_FORMAT_BC5_UNORM;
		GPixelFormats[PF_BC5_SNORM].PlatformFormat = DXGI_FORMAT_BC5_SNORM;
		GPixelFormats[PF_A1].PlatformFormat = DXGI_FORMAT_R1_UNORM; // Not supported for rendering.
		GPixelFormats[PF_A8].PlatformFormat = DXGI_FORMAT_A8_UNORM;
		GPixelFormats[PF_R8].PlatformFormat = DXGI_FORMAT_R8_UNORM;
		GPixelFormats[PF_R8_SNORM].PlatformFormat = DXGI_FORMAT_R8_SNORM;
		GPixelFormats[PF_R32_UINT].PlatformFormat = DXGI_FORMAT_R32_UINT;
		GPixelFormats[PF_R32_SINT].PlatformFormat = DXGI_FORMAT_R32_SINT;

		GPixelFormats[PF_R16_UINT].PlatformFormat = DXGI_FORMAT_R16_UINT;
		GPixelFormats[PF_R16_SINT].PlatformFormat = DXGI_FORMAT_R16_SINT;
		GPixelFormats[PF_R16_SNORM].PlatformFormat = DXGI_FORMAT_R16_SNORM;
		GPixelFormats[PF_R16G16B16A16_UINT].PlatformFormat = DXGI_FORMAT_R16G16B16A16_UINT;
		GPixelFormats[PF_R16G16B16A16_SINT].PlatformFormat = DXGI_FORMAT_R16G16B16A16_SINT;

		GPixelFormats[PF_R5G6B5_UNORM].PlatformFormat = DXGI_FORMAT_B5G6R5_UNORM;
		GPixelFormats[PF_R5G5B5A1_UNORM].PlatformFormat = DXGI_FORMAT_B5G5R5A1_UNORM;
		GPixelFormats[PF_R8G8B8A8].PlatformFormat = DXGI_FORMAT_B8G8R8A8_TYPELESS;
		GPixelFormats[PF_R8G8B8A8_SNORM].PlatformFormat = DXGI_FORMAT_R8G8B8A8_SNORM;
		GPixelFormats[PF_R8G8B8A8_SINT].PlatformFormat = DXGI_FORMAT_R8G8B8A8_SINT;
		GPixelFormats[PF_R8G8].PlatformFormat = DXGI_FORMAT_R8G8_UNORM;
		GPixelFormats[PF_R8G8_SNORM].PlatformFormat = DXGI_FORMAT_R8G8_SNORM;
		GPixelFormats[PF_R32G32B32A32_UINT].PlatformFormat = DXGI_FORMAT_R32G32B32A32_UINT;
		GPixelFormats[PF_R16G16_UINT].PlatformFormat = DXGI_FORMAT_R16G16_UINT;
		GPixelFormats[PF_R16G16_FLOAT].PlatformFormat = DXGI_FORMAT_R16G16_FLOAT;

		GPixelFormats[PF_BC6H].PlatformFormat = DXGI_FORMAT_BC6H_UF16;
		GPixelFormats[PF_BC7].PlatformFormat = DXGI_FORMAT_BC7_UNORM;
		GPixelFormats[PF_R8_UINT].PlatformFormat = DXGI_FORMAT_R8_UINT;
		GPixelFormats[PF_BC1_SRGB].PlatformFormat = DXGI_FORMAT_BC1_UNORM_SRGB;
		GPixelFormats[PF_BC2_SRGB].PlatformFormat = DXGI_FORMAT_BC2_UNORM_SRGB;
		GPixelFormats[PF_BC3_SRGB].PlatformFormat = DXGI_FORMAT_BC3_UNORM_SRGB;
		GPixelFormats[PF_BC7_SRGB].PlatformFormat = DXGI_FORMAT_BC7_UNORM_SRGB;
		GPixelFormats[PF_B8G8R8A8_SRGB].PlatformFormat = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		GPixelFormats[PF_BC5_SRGB].Supported = false;
		GPixelFormats[PF_BC4_SRGB].Supported = false;
		
	/*	for (int i = 0; i < PF_MAX; i++)
		{
			GPixelFormats[i].Supported = 
		}*/

		if (mFeatureLevel >= D3D_FEATURE_LEVEL_11_0)
		{
			GSupportsSeparateRenderTargetBlendState = true;
			GMaxTextureDemensions = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
			GMaxTextureCubeDemensions = D3D11_REQ_TEXTURECUBE_DIMENSION;
			GMaxTextureDepth = D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
			GMaxTextureArrayLayers = D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
			GRHISupportsMSAADepthSampleAccess = true;
		}
		else if (mFeatureLevel >= D3D_FEATURE_LEVEL_10_0)
		{
			GMaxTextureDemensions = D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION;
			GMaxTextureCubeDemensions = D3D10_REQ_TEXTURECUBE_DIMENSION;
			GMaxTextureDepth = D3D10_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
			GMaxTextureArrayLayers = D3D10_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
		}
		GMaxTextureMipCount = Math::ceilLogTwo(GMaxTextureDemensions) + 1;
		GMaxTextureMipCount = Math::min<int32>(MAX_TEXTURE_MIP_COUNT, GMaxTextureMipCount);
		GMaxShadowDepthBufferSizeX = 4096;
		GMaxShadowDepthBufferSizeY = 4096;

		GSupportsTimestampRenderQueries = true;
		GSupportsResolveCubemapFaces = true;

		initUniformBuffers();
	}

	void D3D11DynamicRHI::RHIAcquireThreadOwnership()
	{

	}

	void D3D11DynamicRHI::RHIReleaseThreadOwnership()
	{
	}

	void D3D11DynamicRHI::init()
	{
		initD3DDevice();
	}

	void D3D11DynamicRHI::RHISetScissorRect(bool bEnable, uint32 minx, uint32 miny, uint32 maxx, uint32 maxy)
	{
		if (bEnable)
		{
			D3D11_RECT scissorRect;
			scissorRect.left = minx;
			scissorRect.right = maxx;
			scissorRect.top = miny;
			scissorRect.bottom = maxy;
			mD3D11Context->RSSetScissorRects(1, &scissorRect);
		}
		else
		{
			D3D11_RECT scissorRect;
			scissorRect.left = 0;
			scissorRect.right = getMax2DTextureDemension();
			scissorRect.top = 0;
			scissorRect.bottom = getMax2DTextureDemension();
			mD3D11Context->RSSetScissorRects(1, &scissorRect);
		}
	}

	void D3D11DynamicRHI::updateMSAASettings()
	{
		mAvailableMSAAQualities[0] = 0xffffffff;
		mAvailableMSAAQualities[1] = 0xffffffff;
		mAvailableMSAAQualities[2] = 0;
		mAvailableMSAAQualities[3] = 0xffffffff;
		mAvailableMSAAQualities[4] = 0;
		mAvailableMSAAQualities[5] = 0xffffffff;
		mAvailableMSAAQualities[6] = 0xffffffff;
		mAvailableMSAAQualities[7] = 0xffffffff;
		mAvailableMSAAQualities[8] = 0;

	}

	void D3D11DynamicRHI::setupAfterDeviceCreation()
	{
		RHISetScissorRect(false, 0, 0, 0, 0);
		updateMSAASettings();

		if (GRHISupportsAsyncTextureCreation)
		{

		}
#if PLATFORM_WINDOWS
		IUnknown* renderDoc;
		IID renderDocID;
		if (SUCCEEDED(IIDFromString(L"{A7AA6116-9C8D-4BBA-9083-B4D816B71B78}", &renderDocID)))
		{
			if (SUCCEEDED(mD3D11Device->QueryInterface(__uuidof(ID3D10Debug), (void**)&renderDoc)))
			{
				GDynamicRHI->enableIdealGPUCaptureOptions(true);
			}
		}
#endif

	}

	void D3D11DynamicRHI::clearState()
	{
		mStateCache.clearState();
		Memory::memzero(mCurrentResourceBoundAsSRVs, sizeof(mCurrentResourceBoundAsSRVs));
		for (int32 frequency = 0; frequency < SF_NumFrequencies; frequency++)
		{
			mMaxBoundShaderResourceIndex[frequency] = INDEX_NONE;
		}
	}

	void D3D11DynamicRHI::RHIBeginDrawingViewport(RHIViewport* viewportRHI, RHITexture* renderTargetRHI)
	{
		D3D11Viewport* viewport = ResourceCast(viewportRHI);
		mDrawingViewport = viewport;

		if (renderTargetRHI == nullptr)
		{
			renderTargetRHI = viewport->getBackBuffer();
			RHITransitionResources(EResourceTransitionAccess::EWritable, &renderTargetRHI, 1);
		}
		RHIRenderTargetView view(renderTargetRHI);
		RHISetRenderTargets(1, &view, nullptr, 0, nullptr);
		RHISetScissorRect(false, 0, 0, 0, 0);
	}

	extern void constantBufferBeginFrame();

	void D3D11DynamicRHI::RHIBeginFrame()
	{
		RHIPrivateBeginFrame();
		constantBufferBeginFrame();
	}

	void D3D11DynamicRHI::RHIEndFrame()
	{
		
	}

	void D3D11DynamicRHI::RHIBeginScene()
	{
	}

	void D3D11DynamicRHI::RHIEndScene()
	{

	}

	void D3D11DynamicRHI::RHIAdvanceFrameForGetViewportBackBuffer()
	{
	}

	void D3D11DynamicRHI::commitRenderTargetsAndUAVs()
	{
		ID3D11RenderTargetView* RTArray[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
		for (uint32 index = 0; index < mNumSimultaneousRenderTargets; ++index)
		{
			RTArray[index] = mCurrentRenderTargets[index];
		}
		ID3D11UnorderedAccessView* UAVArray[D3D11_PS_CS_UAV_REGISTER_COUNT];
		uint32 UAVInitialCountArray[D3D11_PS_CS_UAV_REGISTER_COUNT];
		for (uint32 index = 0; index < mNumUAVs; index++)
		{
			UAVArray[index] = mCurrentUAVs[index];
			UAVInitialCountArray[index] = -1;
		}
		if (mNumUAVs > 0)
		{
			mD3D11Context->OMSetRenderTargetsAndUnorderedAccessViews(mNumSimultaneousRenderTargets, RTArray, mCurrentDepthStencilTarget, mNumSimultaneousRenderTargets, mNumUAVs, UAVArray, UAVInitialCountArray);
		}
		else
		{
			mD3D11Context->OMSetRenderTargets(mNumSimultaneousRenderTargets, RTArray, mCurrentDepthStencilTarget);
		}

	}

	template<EShaderFrequency ShaderFrequency>
	void D3D11DynamicRHI::internalSetShaderResourceView(D3D11BaseShaderResource* resource, ID3D11ShaderResourceView* srv, int32 resourceIndex, wstring name, D3D11StateCacheBase::ESRV_Type srvType)
	{
		if (!((resource && srv) || (!resource && !srv)))
		{
			return;
		}
		if (resource)
		{
			/*const EResourceTransitionAccess currentAccess = resource->getCurrentGPUAccess();
			const bool accessPass = currentAccess == EResourceTransitionAccess::EReadable || (currentAccess == EResourceTransitionAccess::ERWBarrier && !resource->isDirty()) || currentAccess == EResourceTransitionAccess::ERWSubResBarrier;*/

		}
		D3D11BaseShaderResource*& resourceSlot = mCurrentResourceBoundAsSRVs[ShaderFrequency][resourceIndex];
		int32 & maxResourceIndex = mMaxBoundShaderResourceIndex[ShaderFrequency];
		if (resource)
		{
			maxResourceIndex = std::max<int32>(maxResourceIndex, resourceIndex);
			resourceSlot = resource;
		}
		else if (resourceSlot != nullptr)
		{
			resourceSlot = nullptr;
			if (maxResourceIndex == resourceIndex)
			{
				do
				{
					maxResourceIndex--;
				} while (maxResourceIndex >= 0 && mCurrentResourceBoundAsSRVs[ShaderFrequency][maxResourceIndex] == nullptr);
			}
		}
		mStateCache.setShaderResourceView<ShaderFrequency>(srv, resourceIndex, srvType);
	}





	template<EShaderFrequency ShaderFrequency>
	void D3D11DynamicRHI::clearAllShaderResourcesForFrequency()
	{
		int32 maxIndex = mMaxBoundShaderResourceIndex[ShaderFrequency];
		for (int32 index = maxIndex; index >= 0; --index)
		{
			if (mCurrentResourceBoundAsSRVs[ShaderFrequency][index] != nullptr)
			{
				internalSetShaderResourceView<ShaderFrequency>(nullptr, nullptr, index, L"");
			}
		}
	}


	void D3D11DynamicRHI::clearAllShaderResources()
	{
		clearAllShaderResourcesForFrequency<SF_Vertex>();
		clearAllShaderResourcesForFrequency<SF_Hull>();
		clearAllShaderResourcesForFrequency<SF_Domain>();
		clearAllShaderResourcesForFrequency<SF_Geometry>();
		clearAllShaderResourcesForFrequency<SF_Pixel>();
		clearAllShaderResourcesForFrequency<SF_Compute>();

	}

	void D3D11DynamicRHI::RHIEndDrawingViewport(RHIViewport* viewportRHI, bool bPresent, bool bLockToVsync)
	{
		++mPresentCounter;
		D3D11Viewport* viewport = ResourceCast(viewportRHI);
		mDrawingViewport = nullptr;
		mCurrentDepthTexture = nullptr;
		mCurrentDepthStencilTarget = nullptr;
		mCurrentDSVAccessType = FExclusiveDepthStencil::DepthWrite_StencilWrite;
		mCurrentRenderTargets[0] = nullptr;
		for (uint32 index = 1; index < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++index)
		{
			mCurrentRenderTargets[index] = nullptr;
		}

		clearAllShaderResources();
		commitRenderTargetsAndUAVs();
		mStateCache.setVertexShader(nullptr);
		for (uint32 streamIndex = 0; streamIndex < 16; streamIndex++)
		{
			mStateCache.setStreamSource(nullptr, streamIndex, 0, 0);
		}

		mStateCache.setIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
		mStateCache.setPixelShader(nullptr);
		mStateCache.setDomainShader(nullptr);
		mStateCache.setHullShader(nullptr);
		mStateCache.setGeometryShader(nullptr);

		bool nativelyPresented = viewport->present(bLockToVsync);

		if (GNumActiveGPUsForRendering == 1)
		{
		
		}
	}


	void D3D11DynamicRHI::RHISetRenderTargets(uint32 NumSimultaneousRenderTargets, const RHIRenderTargetView* newRenderTargets, const RHIDepthRenderTargetView* depthStencilTarget, uint32 numUAVs, RHIUnorderedAccessView* const* UAVs)
	{
		D3D11TextureBase* newDepthStencilTarget = getD3D11TextureFromRHITexture(depthStencilTarget ? depthStencilTarget->mTexture : nullptr);
		BOOST_ASSERT(NumSimultaneousRenderTargets + numUAVs <= MaxSimultaneousRenderTargets);
		bool targetChanged = false;
		ID3D11DepthStencilView* depthStencilView = nullptr;
		if (newDepthStencilTarget)
		{
			mCurrentDSVAccessType = depthStencilTarget->getDepthStencilAccess();
			depthStencilView = newDepthStencilTarget->getDepthStencilView(mCurrentDSVAccessType);
			conditionalClearShaderResource(newDepthStencilTarget, false);
		}
		if (mCurrentDepthStencilTarget != depthStencilView)
		{
			mCurrentDepthStencilTarget = depthStencilView;
			mCurrentDepthTexture = newDepthStencilTarget;
			targetChanged = true;
		}
		if (newDepthStencilTarget)
		{
			uint32 currentFrame = mPresentCounter;
			const EResourceTransitionAccess currentAccess = newDepthStencilTarget->getCurrentGPUAccess();
			const uint32 lastFrameWritten = newDepthStencilTarget->getLastFrameWritten();
			const bool readable = currentAccess == EResourceTransitionAccess::EReadable;
			const bool depthWrite = depthStencilTarget->getDepthStencilAccess().IsDepthWrite();
			const bool accessValid = !readable || lastFrameWritten != currentFrame || !depthWrite;

			if (!accessValid || (readable && depthWrite))
			{

			}
			if (depthWrite)
			{
				newDepthStencilTarget->setDirty(true, currentFrame);
			}
		}
		ID3D11RenderTargetView* newRenderTargetViews[MaxSimultaneousRenderTargets];
		for (uint32 index = 0; index < MaxSimultaneousRenderTargets; ++index)
		{
			ID3D11RenderTargetView* renderTargetView = nullptr;
			if (index < NumSimultaneousRenderTargets && newRenderTargets[index].mTexture != nullptr)
			{
				int32 RTMipIndex = newRenderTargets[index].mMipIndex;
				int32 RTSliceIndex = newRenderTargets[index].mArraySliceIndex;
				D3D11TextureBase* newRenderTarget = getD3D11TextureFromRHITexture(newRenderTargets[index].mTexture);
				if (newRenderTarget)
				{
					renderTargetView = newRenderTarget->getRenderTargetView(RTMipIndex, RTSliceIndex);
					uint32 currentFrame = mPresentCounter;
					const EResourceTransitionAccess currentAccess = newRenderTarget->getCurrentGPUAccess();
					const uint32 lastFrameWritten = newRenderTarget->getLastFrameWritten();
					const bool readable = currentAccess == EResourceTransitionAccess::EReadable;
					const bool bAccessValid = !readable || lastFrameWritten != currentFrame;
					if (!bAccessValid || readable)
					{

					}
					newRenderTarget->setDirty(true, currentFrame);
				}
				conditionalClearShaderResource(newRenderTarget, false);
			}
			newRenderTargetViews[index] = renderTargetView;
			if (mCurrentRenderTargets[index] != renderTargetView)
			{
				mCurrentRenderTargets[index] = renderTargetView;
				targetChanged = true;
			}
		}
		if (mNumSimultaneousRenderTargets != NumSimultaneousRenderTargets)
		{
			mNumSimultaneousRenderTargets = NumSimultaneousRenderTargets;
			targetChanged = true;
		}
		for (uint32 index = 0; index < MaxSimultaneousUAVs; ++index)
		{
			ID3D11UnorderedAccessView* uav = nullptr;
			if (index < numUAVs && UAVs[index] != nullptr)
			{
				D3D11UnorderedAccessView* RHIUAV = (D3D11UnorderedAccessView*)UAVs[index];
				uav = RHIUAV->mView;
				if (uav)
				{
					const EResourceTransitionAccess currentAccess = RHIUAV->mResource->getCurrentGPUAccess();
					const bool uavDirty = RHIUAV->mResource->isDirty();
					const bool accessPass = (currentAccess == EResourceTransitionAccess::ERWBarrier && !uavDirty) || (currentAccess == EResourceTransitionAccess::ERWNoBarrier);
					RHIUAV->mResource->setDirty(true, mPresentCounter);
				}
				conditionalClearShaderResource(RHIUAV->mResource, true);
			}
			if (mCurrentUAVs[index] != uav)
			{
				mCurrentUAVs[index] = uav;
				targetChanged = true;
			}
		}
		if (mNumUAVs != numUAVs)
		{
			mNumUAVs = numUAVs;
			targetChanged = true;
		}
		if (targetChanged)
		{
			commitRenderTargetsAndUAVs();
		}
		if (newRenderTargetViews[0])
		{
			RTVDesc RTTDesc = getRenderTargetViewDesc(newRenderTargetViews[0]);
			RHISetViewport(0, 0, 0.0f, RTTDesc.width, RTTDesc.height, 1.0f);
		}
		else if(depthStencilView)
		{
			TRefCountPtr<ID3D11Texture2D> depthTargetTexture;
			depthStencilView->GetResource((ID3D11Resource**)depthTargetTexture.getInitReference());
			D3D11_TEXTURE2D_DESC dttDesc;
			depthTargetTexture->GetDesc(&dttDesc);
			RHISetViewport(0, 0, 0.0f, dttDesc.Width, dttDesc.Height, 1.0);
		}
	}

	template<EShaderFrequency ShaderFrequency>
	void D3D11DynamicRHI::clearShaderResourceViews(D3D11BaseShaderResource* resource)
	{
		int32 maxIndex = mMaxBoundShaderResourceIndex[ShaderFrequency];
		for (int32 index = maxIndex; index >= 0; --index)
		{
			if (mCurrentResourceBoundAsSRVs[ShaderFrequency][index] == resource)
			{
				internalSetShaderResourceView<ShaderFrequency>(nullptr, nullptr, index, L"");
			}
		}
	}


	void D3D11DynamicRHI::conditionalClearShaderResource(D3D11BaseShaderResource* resource, bool bCheckBoundInputAssembler)
	{
		clearShaderResourceViews<SF_Vertex>(resource);
		clearShaderResourceViews<SF_Hull>(resource);
		clearShaderResourceViews<SF_Domain>(resource);
		clearShaderResourceViews<SF_Geometry>(resource);
		clearShaderResourceViews<SF_Pixel>(resource);
		clearShaderResourceViews<SF_Compute>(resource);

		if (bCheckBoundInputAssembler)
		{
			for (int32 resourceIndex = mMaxBoundVertexBufferIndex; resourceIndex >= 0; --resourceIndex)
			{
				if (mCurrentResourceBoundAsVBs[resourceIndex] == resource)
				{
					trackResourceBoundAsVB(nullptr, resourceIndex);
					mStateCache.setStreamSource(nullptr, resourceIndex, 0);
				}
			}
			if (resource == mCurrentResourceBoundAsIB)
			{
				trackResourceBoundAsIB(nullptr);
				mStateCache.setIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
			}
		}
	}
	
	void D3D11DynamicRHI::initD3DDevice()
	{
		SCOPED_SUSPEND_RENDERING_THREAD(false);
		if (!mD3D11Device)
		{
			clearState();
			IDXGIAdapter* adapter;
			D3D_DRIVER_TYPE	driverType = D3D_DRIVER_TYPE_UNKNOWN;
			uint32 deviceFlags = D3D11RHI_ShouldAllowAsyncResourceCreation() ? 0 : D3D11_CREATE_DEVICE_SINGLETHREADED;
			const bool bWithD3DDebug = D3D11RHI_ShouldCreateWithD3DDebug();
			if (bWithD3DDebug)
			{
				deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
			}
			GTexturePoolSize = 0;
			IDXGIAdapter *EnumAdapter;

			if (mDXGIFactory1->EnumAdapters(mChosenAdapter, &EnumAdapter) != DXGI_ERROR_NOT_FOUND)
			{
				if (EnumAdapter)
				{
					DXGI_ADAPTER_DESC adapterDesc = mChosenDescription;
					adapter = EnumAdapter;
					GRHIAdapterName = adapterDesc.Description;
					GRHIVendorId = adapterDesc.VendorId;
					GRHIDeviceId = adapterDesc.DeviceId;
					GRHIDeviceRevision = adapterDesc.Revision;

					{
						GPUDriverInfo GPUDriverInfo = PlatformMisc::getGPUDriverInfo(GRHIAdapterName);
					}
					D3D11GlobalStats::GDedicatedVideoMemory = int64(adapterDesc.DedicatedVideoMemory);
					D3D11GlobalStats::GDedicatedSystemMemory = int64(adapterDesc.DedicatedSystemMemory);
					D3D11GlobalStats::GSharedSystemMemory = int64(adapterDesc.SharedSystemMemory);

					int64 totalPhysicalMemory = std::min<int64>(int64(PlatformMemory::getConstants().mTotalPhysicalGB), 8ll) * (1024ll * 1024ll * 1024ll);

					int64 consideredSharedSystemMemory = std::min<uint64>(totalPhysicalMemory / 4ll, D3D11GlobalStats::GSharedSystemMemory / 2ll);

					D3D11GlobalStats::GTotalGraphicsMemory = 0;
					if (isRHIDeviceIntel())
					{
						D3D11GlobalStats::GTotalGraphicsMemory = D3D11GlobalStats::GDedicatedVideoMemory;
						D3D11GlobalStats::GTotalGraphicsMemory += D3D11GlobalStats::GDedicatedSystemMemory;
						D3D11GlobalStats::GTotalGraphicsMemory += consideredSharedSystemMemory;
					}
					else if(D3D11GlobalStats::GDedicatedVideoMemory >= 200 * 1024 * 1024)
					{
						D3D11GlobalStats::GTotalGraphicsMemory = D3D11GlobalStats::GDedicatedVideoMemory;
					}
					else if (D3D11GlobalStats::GDedicatedVideoMemory >= 400 * 1024 * 1024)
					{
						D3D11GlobalStats::GTotalGraphicsMemory = consideredSharedSystemMemory;
					}
					else
					{
						D3D11GlobalStats::GTotalGraphicsMemory = totalPhysicalMemory / 4ll;
					}
					if (sizeof(SIZE_T) < 8)
					{
						D3D11GlobalStats::GTotalGraphicsMemory = std::min<int64>(D3D11GlobalStats::GTotalGraphicsMemory, 1024ll * 1024ll * 1024ll);
					}
					else
					{
						D3D11GlobalStats::GTotalGraphicsMemory = std::min<int64>(D3D11GlobalStats::GTotalGraphicsMemory, 1945 * 1024ll * 1024ll);
					}
					if (GPoolSizeVRAMPercentage > 0)
					{
						float poolSize = float(GPoolSizeVRAMPercentage) * 0.01f * float(D3D11GlobalStats::GTotalGraphicsMemory);
						GTexturePoolSize = int64(std::truncf(poolSize / 1024.0f / 1024.0f) * 1024 * 1024);

					}
					const bool bIsPerfHUD = boost::algorithm::equals(adapterDesc.Description, TEXT("NVIDIA PerfHUD"));
					if (bIsPerfHUD)
					{
						driverType = D3D_DRIVER_TYPE_REFERENCE;
					}
				}
			}
			else
			{

			}
			if (isRHIDeviceAMD())
			{
				AGSGPUInfo amdGpuInfo;
				if (agsInit(&mAmdAgsContext, NULL, &amdGpuInfo) == AGS_SUCCESS)
				{
					bool bFoundMatchingDevice = false;
					for (int32 deviceIndex = 0; deviceIndex < amdGpuInfo.numDevices; deviceIndex++)
					{
						const AGSDeviceInfo& deviceInfo = amdGpuInfo.devices[deviceIndex];
						GRHIDeviceIsAMDPreGCNArchitecture |= (mChosenDescription.VendorId == deviceInfo.vendorId) && (mChosenDescription.DeviceId == deviceInfo.deviceId) && (deviceInfo.architectureVersion == AGSDeviceInfo::ArchitectureVersion_PreGCN);
						bFoundMatchingDevice |= (mChosenDescription.VendorId == deviceInfo.vendorId) && (mChosenDescription.DeviceId == deviceInfo.deviceId);
					}
				}
			}

			D3D_FEATURE_LEVEL actualFeatureLevel = (D3D_FEATURE_LEVEL)0;

			ID3D11Device* device;
			ID3D11DeviceContext* context;
			VERIFYD3d11RESULT(D3D11CreateDevice(adapter, driverType, NULL, deviceFlags, &mFeatureLevel, 1, D3D11_SDK_VERSION, mD3D11Device.getInitReference(), &actualFeatureLevel, mD3D11Context.getInitReference()));
			adapter->Release();
			mStateCache.init(mD3D11Context);

			D3D11_FEATURE_DATA_THREADING ThreadingSupport = { 0 };
			VERIFYD3D11RESULT_EX(mD3D11Device->CheckFeatureSupport(D3D11_FEATURE_THREADING, &ThreadingSupport, sizeof(ThreadingSupport)), mD3D11Device);
			GRHISupportsAsyncTextureCreation = !!ThreadingSupport.DriverConcurrentCreates && (deviceFlags & D3D11_CREATE_DEVICE_SINGLETHREADED) == 0;

			GShaderPlatformForFeatureLevel[ERHIFeatureLevel::ES2] = SP_PCD3D_ES2;
			GShaderPlatformForFeatureLevel[ERHIFeatureLevel::ES3_1] = SP_PCD3D_ES3_1;
			GShaderPlatformForFeatureLevel[ERHIFeatureLevel::SM4] = SP_PCD3D_SM4;
			GShaderPlatformForFeatureLevel[ERHIFeatureLevel::SM5] = SP_PCD3D_SM5;

#if PLATFORM_DESKTOP
			if (isRHIDeviceNVIDIA())
			{
				GSupportsDepthBoundsTest = true;
				NV_GET_CURRENT_SLI_STATE SLICaps;
				Memory::memzero(SLICaps);
				SLICaps.version = NV_GET_CURRENT_SLI_STATE_VER;
				NvAPI_Status SLIStatus = NvAPI_D3D_GetCurrentSLIState(mD3D11Device, &SLICaps);
				if (SLIStatus == NVAPI_OK)
				{
					if (SLICaps.numAFRGroups > 1)
					{
						GNumActiveGPUsForRendering = SLICaps.numAFRGroups;
					}
				}
				else
				{

				}
			}
			else if (isRHIDeviceAMD())
			{
				const uint32 amdShaderExtensionUavSlot = 7;
				uint32 amdSupportedExtensionFlags = 0;
				auto amdAsgResult = agsDriverExtensionsDX11_Init(mAmdAgsContext, mD3D11Device, amdShaderExtensionUavSlot, &amdSupportedExtensionFlags);
				if (amdAsgResult == AGS_SUCCESS && (amdSupportedExtensionFlags & AGS_DX11_EXTENSION_DEPTH_BOUNDS_TEST) != 0)
				{
					GSupportsDepthBoundsTest = true;
				}
			}
			if (GDX11ForcedGPUs > 0)
			{
				GNumActiveGPUsForRendering = GDX11ForcedGPUs;
			}
#endif
			setupAfterDeviceCreation();
			for (TLinkedList<RenderResource*>::TIterator resourceIt(RenderResource::getResourceList()); resourceIt; resourceIt.next())
			{
				resourceIt->initRHI();
			}

			for (TLinkedList<RenderResource*>::TIterator resourceIt(RenderResource::getResourceList()); resourceIt; resourceIt.next())
			{
				resourceIt->initDynamicRHI();
			}
			{
				GRHISupportsHDROutputs = supportsHDROutput(this);
				GRHIHDRDisplayOutputFormat = PF_FloatRGBA;
			}
			HardwareInfo::registerHardwareInfo(NAME_RHI, TEXT("D3D11"));
			
			GRHISupportsTextureStreaming = true;

			GRHISupportsFirstInstance = true;

			GIsRHIInitialized = true;
		}
	}

	void D3D11DynamicRHI::RHISetViewport(uint32 MinX, uint32 MinY, float MinZ, uint32 maxX, uint32 maxY, float maxZ)
	{
		BOOST_ASSERT(MinX <= (uint32)D3D11_VIEWPORT_BOUNDS_MAX);
		BOOST_ASSERT(MinY <= (uint32)D3D11_VIEWPORT_BOUNDS_MAX);
		BOOST_ASSERT(maxX <= (uint32)D3D11_VIEWPORT_BOUNDS_MAX);
		BOOST_ASSERT(maxY <= (uint32)D3D11_VIEWPORT_BOUNDS_MAX);
		D3D11_VIEWPORT viewport = { (float)MinX, (float)MinY, (float)maxX - MinX, (float)maxY - MinY, MinZ, maxZ };
		if (viewport.Width > 0 && viewport.Height > 0)
		{
			mStateCache.setViewport(viewport);
			setScissorRectIfRequiredWhenSettingViewport(MinX, MinY, maxX, maxY);
		}
	}


	
	void D3D11DynamicRHI::RHIResizeViewport(RHIViewport* viewportRHI, uint32 sizeX, uint32 sizeY, bool isFullscreen)
	{
		D3D11Viewport* viewport = ResourceCast(viewportRHI);
		BOOST_ASSERT(isInGameThread());
		viewport->resize(sizeX, sizeX, isFullscreen, PF_Unknown);
	}



	void D3D11DynamicRHI::RHIResizeViewport(RHIViewport* viewportRHI, uint32 sizeX, uint32 sizeY, bool isFullscreen, EPixelFormat preferredPixelFormat)
	{
		D3D11Viewport* viewport = ResourceCast(viewportRHI);
		BOOST_ASSERT(isInGameThread());
		viewport->resize(sizeX, sizeY, isFullscreen, PF_Unknown);
	}

	ViewportRHIRef D3D11DynamicRHI::RHICreateViewport(void* windowHandle, uint32 sizeX, uint32 sizeY, bool isFullscree, EPixelFormat preferredPixelFormat)
	{
		BOOST_ASSERT(isInGameThread());
		if (preferredPixelFormat == EPixelFormat::PF_Unknown)
		{
			preferredPixelFormat = EPixelFormat::PF_A2B10G10R10;
		}
		return new D3D11Viewport(this, (HWND)windowHandle, sizeX, sizeY, isFullscree, preferredPixelFormat);
	}


	void D3D11DynamicRHIModule::findAdapter()
	{
		IDXGIFactory1* dxgiFactory1;
		safeCreateDXGIFactory(&dxgiFactory1);
		if (!dxgiFactory1)
		{
			return;
		}
		bool bAllowPerfHUD = true;
#if 0
		bAllowPerfHUD = false;
#endif
		int32 HmdGraphicsAdapter = IHeadMountedDisplayModule::isAvailable() ? IHeadMountedDisplayModule::get().getGraphicsAdapter() : -1;
		bool bUseHmdGraphicsAdapter = HmdGraphicsAdapter >= 0;
		int32 CVarExplicitAdatperValue = HmdGraphicsAdapter;
		const bool bFavorNonIntegrated = CVarExplicitAdatperValue == -1;
		IDXGIAdapter* tempAdapter;
		D3D_FEATURE_LEVEL maxAllowedFeatureLevel = getAllowedD3DFeatureLevel();

		D3D11Adapter firstWithoutIntegeratedAdapter;
		D3D11Adapter firstAdapter;
		TArray<DXGI_ADAPTER_DESC> adapterDescription;
		bool isAnyAMD = false;
		bool isAnyIntel = false;
		bool isAnyNVIDIA = false;
		for (uint32 adapterIndex = 0; dxgiFactory1->EnumAdapters(adapterIndex, &tempAdapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex)
		{
			DXGI_ADAPTER_DESC& adapterDesc = adapterDescription[adapterDescription.addZeroed()];
			if (tempAdapter)
			{
				D3D_FEATURE_LEVEL actualFeatureLevel = (D3D_FEATURE_LEVEL)0;
				if (safeTestD3D11CreateDevice(tempAdapter, maxAllowedFeatureLevel, &actualFeatureLevel))
				{
					VERIFYD3d11RESULT(tempAdapter->GetDesc(&adapterDesc));
					uint32 outputCount = countAdapterOutputs(tempAdapter);
					bool isAMD = adapterDesc.VendorId == RHIV_AMD;
					bool isIntel = adapterDesc.VendorId == RHIV_Intel;
					bool isNVIDIA = adapterDesc.VendorId == RHIV_NVIDIA;
					bool isMicrosoft = adapterDesc.VendorId == RHIV_Macrosoft;
					if (isAMD) isAnyAMD = true;
					if (isIntel) isAnyIntel = true;
					if (isNVIDIA) isAnyNVIDIA = true;

					const bool isIntegrated = isIntel;
					const bool isPerfHUD = boost::algorithm::equals(adapterDesc.Description, TEXT("NVIDIA PerfHUD"));
					D3D11Adapter currentAdapter(adapterIndex, actualFeatureLevel);
					const bool bSkipHmdGraphicsAdapter = isMicrosoft && CVarExplicitAdatperValue < 0 && bUseHmdGraphicsAdapter;
					const bool bSkipPerfHUDAdapter = isPerfHUD && !bAllowPerfHUD;


					const bool bSkipExplicitAdapter = CVarExplicitAdatperValue >= 0 && adapterIndex != CVarExplicitAdatperValue;

					const bool bSkipAdapter = bSkipHmdGraphicsAdapter || bSkipPerfHUDAdapter || bSkipExplicitAdapter;

					if (!bSkipAdapter)
					{
						if (!isIntegrated && !firstWithoutIntegeratedAdapter.isValid())
						{
							firstWithoutIntegeratedAdapter = currentAdapter;
						}
						if (!firstAdapter.isValid())
						{
							firstAdapter = currentAdapter;
						}
					}
				}
			}
		}
		if (bFavorNonIntegrated && (isAnyAMD || isAnyNVIDIA))
		{
			mChosenAdapter = firstWithoutIntegeratedAdapter;
			if (!mChosenAdapter.isValid())
			{
				mChosenAdapter = firstAdapter;
			}
		}
		else
		{
			mChosenAdapter = firstAdapter;
		}
		if (mChosenAdapter.isValid())
		{
			mChosenDescription = adapterDescription[mChosenAdapter.mAdapterIndex];
		}
		else
		{

		}
		if (mChosenAdapter.isValid() && mChosenAdapter.mMaxSupportedFeatureLevel != D3D_FEATURE_LEVEL_10_0)
		{
			DXGI_ADAPTER_DESC adapterDesc;
			ZeroMemory(&adapterDesc, sizeof(DXGI_ADAPTER_DESC));
			dxgiFactory1->EnumAdapters(mChosenAdapter.mAdapterIndex, &tempAdapter);
			VERIFYD3d11RESULT(tempAdapter->GetDesc(&adapterDesc));
			const bool bIsAMD = adapterDesc.VendorId == RHIV_AMD;
			const bool bIsIntel = adapterDesc.VendorId == RHIV_Intel;
			const bool bIsNVIDIA = adapterDesc.VendorId == RHIV_NVIDIA;
		}
		dxgiFactory1->Release();
	}

	


	void D3D11DynamicRHI::commitNonComputeShaderConstants()
	{
		D3D11BoundShaderState* currentBoundShaderState = (D3D11BoundShaderState*)mBoundShaderStateHistory.getLast();
		BOOST_ASSERT(currentBoundShaderState);
		if (currentBoundShaderState->bShaderNeedsGlobalUniformBuffer[SF_Vertex])
		{
			for (uint32 i = 0; i < MAX_UNIFORM_BUFFER_SLOTS; i++)
			{
				D3D11UniformBuffer* constantBuffer = mVSUnifomBuffers[i];
				D3DRHIUtil::commitUniforms<SF_Vertex>(constantBuffer, mStateCache, i, bDiscardSharedConstants);
			}
		}

		if (bUsingTessellation)
		{
			if (currentBoundShaderState->bShaderNeedsGlobalUniformBuffer[SF_Hull])
			{
				for (uint32 i = 0; i < MAX_UNIFORM_BUFFER_SLOTS; i++)
				{
					D3D11UniformBuffer* uniformBuffer = mHSUnifomBuffers[i];
					D3DRHIUtil::commitUniforms<SF_Hull>(uniformBuffer, mStateCache, i, bDiscardSharedConstants);
				}
			}

			if (currentBoundShaderState->bShaderNeedsGlobalUniformBuffer[SF_Domain])
			{
				for (uint32 i = 0; i < MAX_UNIFORM_BUFFER_SLOTS; i++)
				{
					D3D11UniformBuffer* uniformBuffer = mDSUnifomBuffers[i];
					D3DRHIUtil::commitUniforms<SF_Domain>(uniformBuffer, mStateCache, i, bDiscardSharedConstants);
				}
			}
		}
		if (currentBoundShaderState->bShaderNeedsGlobalUniformBuffer[SF_Geometry])
		{
			for (uint32 i = 0; i < MAX_UNIFORM_BUFFER_SLOTS; i++)
			{
				D3D11UniformBuffer* uniformBuffer = mGSUnifomBuffers[i];
				D3DRHIUtil::commitUniforms<SF_Geometry>(uniformBuffer, mStateCache, i, bDiscardSharedConstants);
			}
		}

		if (currentBoundShaderState->bShaderNeedsGlobalUniformBuffer[SF_Pixel])
		{
			for (int32 i = 0; i < MAX_UNIFORM_BUFFER_SLOTS; i++)
			{
				D3D11UniformBuffer* uniformBuffer = mPSUnifomBuffers[i];
				D3DRHIUtil::commitUniforms<SF_Pixel>(uniformBuffer, mStateCache, i, bDiscardSharedConstants);
			}
		}
		bDiscardSharedConstants = false;
	}

	void D3D11DynamicRHI::commitComputeShaderConstants()
	{
		bool bLocalDiscardSharedConstants = true;
		for (uint32 i = 0; i < MAX_UNIFORM_BUFFER_SLOTS; i++)
		{
			D3D11UniformBuffer * uniformBuffer = mCSUnifomBuffers[i];
			D3DRHIUtil::commitUniforms<SF_Compute>(uniformBuffer, mStateCache, i, bDiscardSharedConstants);
		}
	}

	template<class ShaderType>
	void D3D11DynamicRHI::setResourcesFromTables(const ShaderType* RESTRICT shader)
	{
		BOOST_ASSERT(shader);
		uint32 dirtyBits = shader->mShaderResourceTable.mResourceTableBits & mDirtyConstantBuffers[ShaderType::StaticFrequency];
		while (dirtyBits)
		{
			const uint32 lowestBitMask = (dirtyBits)&(-(int32)dirtyBits);
			const int32 bufferIndex = Math::floorLog2(lowestBitMask);
			dirtyBits ^= lowestBitMask;
			D3D11ConstantBuffer* buffer = (D3D11ConstantBuffer*)mBoundConstantBuffers[ShaderType::StaticFrequency][bufferIndex].getReference();
			BOOST_ASSERT(buffer);
			BOOST_ASSERT(bufferIndex < shader->mShaderResourceTable.mResourceTableLayoutHashes.size());

			setShaderResourcesFromBuffer_Surface<(EShaderFrequency)ShaderType::StaticFrequency>(this, &mStateCache, buffer, shader->mShaderResourceTable.mTextureMap.getData(), bufferIndex);
			setShaderResourcesFromBuffer_SRV<(EShaderFrequency)ShaderType::StaticFrequency>(this, &mStateCache, buffer, shader->mShaderResourceTable.mShaderResourceViewMap.getData(), bufferIndex);
			setShaderResourcesFromBuffer_Sampler<(EShaderFrequency)ShaderType::StaticFrequency>(this, &mStateCache, buffer, shader->mShaderResourceTable.mSamplerMap.getData(), bufferIndex);
		}
		
		mDirtyConstantBuffers[ShaderType::StaticFrequency] = 0;
	}

	void D3D11DynamicRHI::commitGraphicsResourceTables()
	{
		D3D11BoundShaderState* RESTRICT currentBoundShaderState = (D3D11BoundShaderState*)mBoundShaderStateHistory.getLast();
		BOOST_ASSERT(currentBoundShaderState);
		if (auto *shader = currentBoundShaderState->getVertexShader())
		{
			setResourcesFromTables(shader);
		}
		if (auto* shader = currentBoundShaderState->getPixelShader())
		{
			setResourcesFromTables(shader);
		}
		if (auto* shader = currentBoundShaderState->getHullShader())
		{
			setResourcesFromTables(shader);
		}
		if (auto * shader = currentBoundShaderState->getDomainShader())
		{
			setResourcesFromTables(shader);
		}
		if (auto * shader = currentBoundShaderState->getGeometryShader())
		{
			setResourcesFromTables(shader);
		}
	}

	void D3D11DynamicRHI::commitComputeResourceTables(D3D11ComputeShader* inComputeShader)
	{
		D3D11ComputeShader* RESTRICT computeShader = inComputeShader;
		BOOST_ASSERT(computeShader);
		setResourcesFromTables(computeShader);
	}

	void D3D11DynamicRHI::RHISetStreamSource(uint32 streamIndex, RHIVertexBuffer* vertexBuffer, uint32 offset)
	{
		D3D11VertexBuffer* buffer = ResourceCast(vertexBuffer);
		ID3D11Buffer* d3dBuffer = buffer ? buffer->mResource : NULL;
		mStateCache.setStreamSource(d3dBuffer, streamIndex, offset);
	}

	void D3D11DynamicRHI::RHIDrawPrimitive(int32 baseVertexIndex, uint32 numPrimitives, uint32 numInstances)
	{
		commitGraphicsResourceTables();
		commitNonComputeShaderConstants();
		uint32 vertexCount = getVertexCountForPrimitiveCount(numPrimitives, mPrimitiveType);
		mStateCache.setPrimitiveTopology(getD3D11PrimitiveType(mPrimitiveType, bUsingTessellation));
		if (numInstances > 1)
		{
			mD3D11Context->DrawInstanced(vertexCount, numInstances, baseVertexIndex, 0);
		}
		else
		{
			mD3D11Context->Draw(vertexCount, baseVertexIndex);
		}
	}

	void D3D11DynamicRHI::RHIDrawIndexedPrimitive(RHIIndexBuffer* indexBuffer, uint32 primitiveType, int32 baseVertexIndex, uint32 firstInstance, uint32 numVertex, uint32 startIndex, uint32 numPrimitives, uint32 numInstances)
	{
		D3D11IndexBuffer* buffer = ResourceCast(indexBuffer);
		BOOST_ASSERT(numPrimitives > 0);
		commitGraphicsResourceTables();
		commitNonComputeShaderConstants();
		uint32 sizeFormat = sizeof(DXGI_FORMAT);
		const DXGI_FORMAT format = (indexBuffer->getStride() == sizeof(uint16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
		uint32 indexCount = getVertexCountForPrimitiveCount(numPrimitives, primitiveType);
		BOOST_ASSERT((startIndex + indexCount) * indexBuffer->getStride() <= indexBuffer->getSize());
		mStateCache.setIndexBuffer(buffer->mResource, format, 0);
		mStateCache.setPrimitiveTopology(getD3D11PrimitiveType(primitiveType, bUsingTessellation));
		if (numInstances > 1 || firstInstance != 0)
		{
			mD3D11Context->DrawIndexedInstanced(indexCount, numInstances, startIndex, baseVertexIndex, firstInstance);
		}
		else
		{
			mD3D11Context->DrawIndexed(indexCount, startIndex, baseVertexIndex);
		}
	}


	

	

	void D3D11DynamicRHI::RHISetShaderSampler(RHIComputeShader* computeShader, uint32 samplerIndex, RHISamplerState* newSampler)
	{
		//VALIDATE_BOUND_SHADER(computeShader);
		D3D11ComputeShader* shader = ResourceCast(computeShader);
		D3D11SamplerState* sampler = ResourceCast(newSampler);
		ID3D11SamplerState* stateResource = sampler->mResource;
		mStateCache.setSamplerState<SF_Compute>(stateResource, samplerIndex);
	}

	


	DynamicRHI* D3D11DynamicRHIModule::createRHI(ERHIFeatureLevel::Type requestedFeatureLevel /* = ERHIFeatureLevel::Num */)
	{
		IDXGIFactory1* DXGIFactory1;
		safeCreateDXGIFactory(&DXGIFactory1);
		return new D3D11DynamicRHI(DXGIFactory1, mChosenAdapter.mMaxSupportedFeatureLevel, mChosenAdapter.mAdapterIndex, mChosenDescription);
	}
	uint32 D3D11DynamicRHI::getMaxMSAAQuality(uint32 sampleCount)
	{
		if (sampleCount <= DX_MAX_MSAA_COUNT)
		{
			return 0;
		}
		return 0xFFFFFFFF;
	}

	void D3D11DynamicRHI::RHISubmitCommandsHint()
	{

	}

	uint32 D3D11DynamicRHI::RHIComputeMemorySize(RHITexture* textureRHI)
	{
		if (!textureRHI)
		{
			return 0;
		}
		D3D11TextureBase* texture = getD3D11TextureFromRHITexture(textureRHI);
		return texture->getMemorySize();
	}

	
	void D3D11DynamicRHI::RHIReadSurfaceFloatData(RHITexture* textureRHI, IntRect inRect, TArray<Float16Color>& outData, ECubeFace cubeface, int32 arrayIndex, int32 mipIndex)
	{
		D3D11TextureBase* texture = getD3D11TextureFromRHITexture(textureRHI);
		uint32 width = inRect.width();
		uint32 height = inRect.height();

		D3D11_TEXTURE2D_DESC textureDesc;
		((ID3D11Texture2D*)texture->getResource())->GetDesc(&textureDesc);
		BOOST_ASSERT(textureDesc.Format == GPixelFormats[PF_FloatRGBA].PlatformFormat);

		outData.empty(width * height);

		D3D11_BOX rect;
		rect.left = inRect.min.x;
		rect.top = inRect.min.y;
		rect.right = inRect.max.x;
		rect.bottom = inRect.max.y;
		rect.back = 1;
		rect.front = 0;

		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = textureDesc.Format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.MiscFlags = 0;
		TRefCountPtr<ID3D11Texture2D> tempTexture2D;
		VERIFYD3D11RESULT_EX(mD3D11Device->CreateTexture2D(&desc, NULL, tempTexture2D.getInitReference()), mD3D11Device);

		uint32 subresource = 0;
		if (textureDesc.MiscFlags == D3D11_RESOURCE_MISC_TEXTURECUBE)
		{
			uint32 d3dFace = getD3D11CubeFace(cubeface);
			subresource = D3D11CalcSubresource(mipIndex, arrayIndex * 6 + d3dFace, textureDesc.MipLevels);
		}
		mD3D11Context->CopySubresourceRegion(tempTexture2D, 0, 0, 0, 0, texture->getResource(), subresource, &rect);

		D3D11_MAPPED_SUBRESOURCE lockedRect;
		VERIFYD3D11RESULT_EX(mD3D11Context->Map(tempTexture2D, 0, D3D11_MAP_READ, 0, &lockedRect), mD3D11Device);

		int32 totalCount = width * height;

		if (totalCount >= outData.size())
		{
			outData.addZeroed(totalCount);
		}
		for (int32 y = inRect.min.y; y < inRect.max.y; y++)
		{
			Float16Color* srcPtr = (Float16Color*)((uint8*)lockedRect.pData + (y - inRect.min.y) * lockedRect.RowPitch);
			int32 index = (y - inRect.min.y) * width;
			BOOST_ASSERT(index + ((int32)width - 1) < outData.size());
			Float16Color* destColor = &outData[index];
			float16* destPtr = (float16*)(destColor);
			Memory::memcpy(destPtr, srcPtr, width * sizeof(float16) * 4);
		}
		mD3D11Context->Unmap(tempTexture2D, 0);
	}
	


	ShaderResourceViewRHIRef D3D11DynamicRHI::RHICreateShaderResourceView(RHITexture* textureRHI, const RHITextureSRVCreateInfo& createInfo)
	{
		D3D11TextureBase* texture = getD3D11TextureFromRHITexture(textureRHI);
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		DXGI_FORMAT baseTextureFormat = DXGI_FORMAT_UNKNOWN;
		if (textureRHI->getTexture3D() != nullptr)
		{
			
		}
		else if (textureRHI->getTextureCube() != nullptr)
		{
			D3D11TextureCube* textureCube = static_cast<D3D11TextureCube*>(texture);
			D3D11_TEXTURE2D_DESC textureDesc;
			textureCube->getResource()->GetDesc(&textureDesc);
			baseTextureFormat = textureDesc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = createInfo.mNumMipLevels;
		}
		else
		{
			D3D11Texture2D* texture2D = static_cast<D3D11Texture2D*>(texture);
			D3D11_TEXTURE2D_DESC texDesc;
			texture2D->getResource()->GetDesc(&texDesc);
			baseTextureFormat = texDesc.Format;
			if (texDesc.SampleDesc.Count > 1)
			{
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
			}
			else
			{
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = createInfo.mNumMipLevels;
				srvDesc.Texture2D.MostDetailedMip = createInfo.mMipLevel;
			}
		}
		const bool bBaseSRGB = (textureRHI->getFlags() & TexCreate_SRGB) != 0;
		const bool bSRGB = (createInfo.mSRGBOverride == SRGBO_ForceEnable) || (createInfo.mSRGBOverride == SRGBO_Default && bBaseSRGB);
		if (createInfo.mFormat != PF_Unknown)
		{
			baseTextureFormat = (DXGI_FORMAT)GPixelFormats[createInfo.mFormat].PlatformFormat;
		}
		srvDesc.Format = findShaderResourceDXGIFormat(baseTextureFormat, bSRGB);
		TRefCountPtr<ID3D11ShaderResourceView> shaderResourceView;
		VERIFYD3D11RESULT_EX(mD3D11Device->CreateShaderResourceView(texture->getResource(), &srvDesc, shaderResourceView.getInitReference()), mD3D11Device);
		return new D3D11ShaderResourceView(shaderResourceView, texture);
	}



	


	
	
	void D3D11DynamicRHI::validateExclusiveDepthStencilAccess(FExclusiveDepthStencil src) const
	{
		const bool bSrcDepthWrite = src.IsDepthWrite();
		const bool bSrcStencilWrite = src.IsStencilWrite();

		if (bSrcDepthWrite || bSrcStencilWrite)
		{
			BOOST_ASSERT(mCurrentDepthTexture);
			const bool bDstDepthWrite = mCurrentDSVAccessType.IsDepthWrite();
			const bool bDstStencilWrite = mCurrentDSVAccessType.IsStencilWrite();

			BOOST_ASSERT(
				!bSrcDepthWrite || bDstDepthWrite,
				TEXT("Expected: SrcDepthWrite := false or DstDepthWrite := true. Actual: SrcDepthWrite := %s or DstDepthWrite := %s"),
				(bSrcDepthWrite) ? TEXT("true") : TEXT("false"),
				(bDstDepthWrite) ? TEXT("true") : TEXT("false")
			);

			BOOST_ASSERT(
				!bSrcStencilWrite || bDstStencilWrite,
				TEXT("Expected: SrcStencilWrite := false or DstStencilWrite := true. Actual: SrcStencilWrite := %s or DstStencilWrite := %s"),
				(bSrcStencilWrite) ? TEXT("true") : TEXT("false"),
				(bDstStencilWrite) ? TEXT("true") : TEXT("false")
			);
		}
	}

	void D3D11DynamicRHI::RHISetRenderTargetsAndClear(const RHISetRenderTargetsInfo& renderTargetsInfo)
	{
		RHIUnorderedAccessView* uavs[MaxSimultaneousUAVs] = {};
		for (int32 index = 0; index < renderTargetsInfo.mNumUAVs; ++index)
		{
			uavs[index] = renderTargetsInfo.mUnorderedAccessView[index].getReference();
		}
		this->RHISetRenderTargets(renderTargetsInfo.mNumColorRenderTargets, renderTargetsInfo.mColorRenderTarget, &renderTargetsInfo.mDepthStencilRenderTarget, renderTargetsInfo.mNumUAVs, uavs);
		if (renderTargetsInfo.bClearColor || renderTargetsInfo.bClearStencil || renderTargetsInfo.bClearDepth)
		{
			LinearColor clearColors[MaxSimultaneousRenderTargets];
			float depthClear = 0.0f;
			uint32 stencilClear = 0;
			if (renderTargetsInfo.bClearColor)
			{
				for (int32 i = 0; i < renderTargetsInfo.mNumColorRenderTargets; i++)
				{
					if(renderTargetsInfo.mColorRenderTarget[i].mTexture != nullptr)
					{
						const ClearValueBinding& clearValue = renderTargetsInfo.mColorRenderTarget[i].mTexture->getClearBinding();
						clearColors[i] = clearValue.getClearColor();
					}
				}
			}
			if (renderTargetsInfo.bClearDepth || renderTargetsInfo.bClearStencil)
			{
				const ClearValueBinding & clearValue = renderTargetsInfo.mDepthStencilRenderTarget.mTexture->getClearBinding();
				clearValue.getDepthStencil(depthClear, stencilClear);
			}

			this->RHIClearMRTImpl(renderTargetsInfo.bClearColor, renderTargetsInfo.mNumColorRenderTargets, clearColors, renderTargetsInfo.bClearDepth, depthClear, renderTargetsInfo.bClearStencil, stencilClear);
		}
	}


	

	

	void D3D11DynamicRHI::RHIBindClearMRTValues(bool bClearColor, bool bClearDepth, bool bClearStencil)
	{

	}


	void D3D11DynamicRHI::RHIClearMRTImpl(bool bClearColor, int32 numClearColor, const LinearColor* colorArray, bool bClearDepth, float depth, bool bClearStencil, uint32 stencil)
	{
		D3D11BoundRenderTargets boundRenderTargets(mD3D11Context);
		BOOST_ASSERT(!bClearColor || numClearColor >= boundRenderTargets.getNumActiveTargets());
		if (mCurrentDepthTexture)
		{
			FExclusiveDepthStencil requestedAccess;
			requestedAccess.SetDepthStencilWrite(bClearDepth, bClearStencil);
			BOOST_ASSERT(requestedAccess.IsValid(mCurrentDSVAccessType));
		}


		ID3D11DepthStencilView* depthStencilView = boundRenderTargets.getDepthStencilView();
		if (bClearColor && boundRenderTargets.getNumActiveTargets() > 0)
		{
			for (int32 targetIndex = 0; targetIndex < boundRenderTargets.getNumActiveTargets(); targetIndex++)
			{
				ID3D11RenderTargetView* renderTargetView = boundRenderTargets.getRenderTargetView(targetIndex);
				if (renderTargetView != nullptr)
				{
					mD3D11Context->ClearRenderTargetView(renderTargetView, (float*)&colorArray[targetIndex]);
				}
			}
		}

		if ((bClearDepth || bClearStencil) && depthStencilView)
		{
			uint32 clearFlags = 0;
			if (bClearDepth)
			{
				clearFlags |= D3D11_CLEAR_DEPTH;
			}
			if (bClearStencil)
			{
				clearFlags |= D3D11_CLEAR_STENCIL;
			}

			mD3D11Context->ClearDepthStencilView(depthStencilView, clearFlags, depth, stencil);
		}

	}

	int32 GUnbindResourcesBetweenDrawsInDX11 = 0;

	void D3D11DynamicRHI::RHISetBoundShaderState(RHIBoundShaderState* boundShaderState)
	{
		D3D11BoundShaderState * boundState = ResourceCast(boundShaderState);
		mStateCache.setInputLayout(boundState->mInputLayout);
		mStateCache.setVertexShader(boundState->mVertexShader);
		mStateCache.setPixelShader(boundState->mPixelShader);
		mStateCache.setHullShader(boundState->mHullShader);
		mStateCache.setDomainShader(boundState->mDomainShader);
		mStateCache.setGeometryShader(boundState->mGeometryShader);
		if (boundState->mHullShader != nullptr && boundState->mDomainShader != nullptr)
		{
			bUsingTessellation = true;
		}
		else
		{
			bUsingTessellation = false;
		}

		bDiscardSharedConstants = true;
		mBoundShaderStateHistory.add(boundState);
		mDirtyConstantBuffers[SF_Vertex] = 0xffff;
		mDirtyConstantBuffers[SF_Pixel] = 0xffff;
		mDirtyConstantBuffers[SF_Hull] = 0xffff;
		mDirtyConstantBuffers[SF_Domain] = 0xffff;
		mDirtyConstantBuffers[SF_Geometry] = 0xffff;
		for (int32 frequency = 0; frequency < SF_NumFrequencies; frequency++)
		{
			for (int32 bindIndex = 0; bindIndex < MAX_CONSTANT_BUFFERS_PER_SHADER_STAGE; ++bindIndex)
			{
				mBoundConstantBuffers[frequency][bindIndex].safeRelease();
			}
		}
		if (GUnbindResourcesBetweenDrawsInDX11)
		{
			clearAllShaderResources();
		}
	}


	VertexBufferRHIRef D3D11DynamicRHI::RHICreateVertexBuffer(uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo)
	{
		D3D11_BUFFER_DESC desc;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.ByteWidth = size;
		desc.Usage = (inUsage & BUF_AnyDynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = (inUsage & BUF_AnyDynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		if (inUsage & BUF_UnorderedAccess)
		{
			desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
			static bool bRequiresRawView = (GMaxRHIFeatureLevel < ERHIFeatureLevel::SM5);
			if (bRequiresRawView)
			{
				desc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
			}
		}
		if (inUsage & BUF_ByteAddressBuffer)
		{
			desc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		}
		if (inUsage & BUF_StreamOutput)
		{
			desc.BindFlags |= D3D11_BIND_STREAM_OUTPUT;
		}
		if (inUsage & BUF_DrawIndirect)
		{
			desc.MiscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		}
		if (inUsage & BUF_ShaderResource)
		{
			desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		}
		if (PlatformProperties::supportsFastVRAMMemory())
		{
			if (inUsage & BUF_FastVRAM)
			{
				FastVRAMAllocator::getFastVRAMAllocator()->allocUAVBuffer(desc);
			}
		}
		D3D11_SUBRESOURCE_DATA initData;
		D3D11_SUBRESOURCE_DATA* pInitData = nullptr;
		if (createInfo.mResourceArray)
		{
			BOOST_ASSERT(size == createInfo.mResourceArray->getResourceDataSize());
			initData.pSysMem = createInfo.mResourceArray->getResourceData();
			initData.SysMemPitch = size;
			initData.SysMemSlicePitch = 0;
			pInitData = &initData;
		}

		TRefCountPtr<ID3D11Buffer> vertexBufferResource;
		VERIFYD3D11RESULT_EX(mD3D11Device->CreateBuffer(&desc, pInitData, vertexBufferResource.getInitReference()), mD3D11Device);
		if (createInfo.mResourceArray)
		{
			createInfo.mResourceArray->discard();
		}
		return new D3D11VertexBuffer(vertexBufferResource, size, inUsage);
	}


	void* D3D11DynamicRHI::RHILockVertexBuffer(RHIVertexBuffer* inVertexBuffer, uint32 offset, uint32 size, EResourceLockMode lockMode)
	{
		D3D11VertexBuffer* vertexBuffer = ResourceCast(inVertexBuffer);
		conditionalClearShaderResource(vertexBuffer, true);
		D3D11_BUFFER_DESC desc;
		vertexBuffer->mResource->GetDesc(&desc);
		const bool bIsDynamic = (desc.Usage == D3D11_USAGE_DYNAMIC);
		D3D11LockedKey lockedKey(vertexBuffer->mResource);
		D3D11LockedData lockedData;
		if (bIsDynamic)
		{
			BOOST_ASSERT(lockMode == RLM_WriteOnly);
			D3D11_MAPPED_SUBRESOURCE mappedSubresource;
			VERIFYD3D11RESULT_EX(mD3D11Context->Map(vertexBuffer->mResource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource), mD3D11Device);
			lockedData.setData(mappedSubresource.pData);
			lockedData.mPitch = mappedSubresource.RowPitch;
		}
		else
		{
			if (lockMode == RLM_ReadOnly)
			{
				D3D11_BUFFER_DESC stagingBufferDesc;
				ZeroMemory(&stagingBufferDesc, sizeof(D3D11_BUFFER_DESC));
				stagingBufferDesc.ByteWidth = size;
				stagingBufferDesc.Usage = D3D11_USAGE_STAGING;
				stagingBufferDesc.BindFlags = 0;
				stagingBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
				stagingBufferDesc.MiscFlags = 0;
				TRefCountPtr<ID3D11Buffer> stagingVertexBuffer;
				VERIFYD3D11RESULT_EX(mD3D11Device->CreateBuffer(&stagingBufferDesc, nullptr, stagingVertexBuffer.getInitReference()), mD3D11Device);
				lockedData.mStagingResource = stagingVertexBuffer;
				D3D11_BOX sourceBox;
				sourceBox.left = offset;
				sourceBox.right = size;
				sourceBox.top = sourceBox.front = 0;
				sourceBox.bottom = sourceBox.back = 1;
				mD3D11Context->CopySubresourceRegion(stagingVertexBuffer, 0, 0, 0, 0, vertexBuffer->mResource, 0, &sourceBox);
				D3D11_MAPPED_SUBRESOURCE mappedSubresource;
				VERIFYD3D11RESULT_EX(mD3D11Context->Map(stagingVertexBuffer, 0, D3D11_MAP_READ, 0, &mappedSubresource), mD3D11Device);
				lockedData.setData(mappedSubresource.pData);
				lockedData.mPitch = mappedSubresource.RowPitch;
			}
			else
			{
				lockedData.allocData(desc.ByteWidth);
				lockedData.mPitch = desc.ByteWidth;
			}
		}
		mOutstandingLocks.emplace(lockedKey, lockedData);

		return (void*)((uint8*)lockedData.getData() + offset);
	}

	void D3D11DynamicRHI::RHIUnlockVertexBuffer(RHIVertexBuffer* inVertexBuffer)
	{
		D3D11VertexBuffer* vertexBuffer = ResourceCast(inVertexBuffer);

		D3D11_BUFFER_DESC desc;
		vertexBuffer->mResource->GetDesc(&desc);
		const bool bIsDynamic = (desc.Usage == D3D11_USAGE_DYNAMIC);
		D3D11LockedKey lockedKey(vertexBuffer->mResource);
		auto it = mOutstandingLocks.find(lockedKey);
		BOOST_ASSERT(it != mOutstandingLocks.end());
		if (bIsDynamic)
		{
			mD3D11Context->Unmap(vertexBuffer->mResource, 0);
		}
		else
		{
			if (it->second.mStagingResource)
			{
				ID3D11Buffer* stagingBuffer = (ID3D11Buffer*)it->second.mStagingResource.getReference();
				mD3D11Context->Unmap(stagingBuffer, 0);
			}
			else
			{
				mD3D11Context->UpdateSubresource(vertexBuffer->mResource, lockedKey.mSubresource, NULL, it->second.getData(), it->second.mPitch, 0);
				it->second.freeData();
			}
		}
		mOutstandingLocks.erase(it);
	}


	IndexBufferRHIRef D3D11DynamicRHI::RHICreateIndexBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo)
	{
		BOOST_ASSERT(size > 0);
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
		desc.ByteWidth = size;
		desc.Usage = (inUsage & BUF_AnyDynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = (inUsage & BUF_AnyDynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
		desc.MiscFlags = 0;

		if (inUsage & BUF_UnorderedAccess)
		{
			desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		}

		if (inUsage & BUF_DrawIndirect)
		{
			desc.MiscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		}

		if (inUsage & BUF_ShaderResource)
		{
			desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
		}

		D3D11_SUBRESOURCE_DATA initData;
		D3D11_SUBRESOURCE_DATA* pInitData = NULL;
		if (createInfo.mResourceArray)
		{
			BOOST_ASSERT(size == createInfo.mResourceArray->getResourceDataSize());
			initData.pSysMem = createInfo.mResourceArray->getResourceData();
			initData.SysMemPitch = size;
			initData.SysMemSlicePitch = 0;
			pInitData = &initData;
		}
		TRefCountPtr<ID3D11Buffer> indexBufferResource;
		VERIFYD3D11RESULT_EX(mD3D11Device->CreateBuffer(&desc, pInitData, indexBufferResource.getInitReference()), mD3D11Device);
		if (createInfo.mResourceArray)
		{
			createInfo.mResourceArray->discard();
		}
		return new D3D11IndexBuffer(indexBufferResource, stride, size, inUsage);

	}

	void* D3D11DynamicRHI::RHILockIndexBuffer(RHIIndexBuffer* inIndexBuffer, uint32 offset, uint32 size, EResourceLockMode lockMode)
	{
		D3D11IndexBuffer* indexBuffer = ResourceCast(inIndexBuffer);
		conditionalClearShaderResource(indexBuffer, true);
		D3D11_BUFFER_DESC desc;
		indexBuffer->mResource->GetDesc(&desc);
		const bool bIsDynamic = (desc.Usage == D3D11_USAGE_DYNAMIC);
		D3D11LockedKey lockedKey(indexBuffer->mResource);
		D3D11LockedData lockedData;
		if (bIsDynamic)
		{
			BOOST_ASSERT(lockMode == RLM_WriteOnly);
			D3D11_MAPPED_SUBRESOURCE mappedSubresource;
			VERIFYD3D11RESULT_EX(mD3D11Context->Map(indexBuffer->mResource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource), mD3D11Device);
			lockedData.setData(mappedSubresource.pData);
			lockedData.mPitch = mappedSubresource.RowPitch;
		}
		else
		{
			if (lockMode == RLM_ReadOnly)
			{
				D3D11_BUFFER_DESC stagingBufferDesc;
				ZeroMemory(&stagingBufferDesc, sizeof(D3D11_BUFFER_DESC));
				stagingBufferDesc.ByteWidth = size;
				stagingBufferDesc.Usage = D3D11_USAGE_STAGING;
				stagingBufferDesc.BindFlags = 0;
				stagingBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
				stagingBufferDesc.MiscFlags = 0;
				TRefCountPtr<ID3D11Buffer> stagingIndexBuffer;
				VERIFYD3D11RESULT_EX(mD3D11Device->CreateBuffer(&stagingBufferDesc, nullptr, stagingIndexBuffer.getInitReference()), mD3D11Device);
				lockedData.mStagingResource = stagingIndexBuffer;
				mD3D11Context->CopyResource(stagingIndexBuffer, indexBuffer->mResource);
				D3D11_MAPPED_SUBRESOURCE mappedSubresource;
				VERIFYD3D11RESULT_EX(mD3D11Context->Map(stagingIndexBuffer, 0, D3D11_MAP_READ, 0, &mappedSubresource), mD3D11Device);
				lockedData.setData(mappedSubresource.pData);
				lockedData.mPitch = mappedSubresource.RowPitch;
			}
			else
			{
				lockedData.allocData(desc.ByteWidth);
				lockedData.mPitch = desc.ByteWidth;
			}
		}
		mOutstandingLocks.emplace(lockedKey, lockedData);

		return (void*)((uint8*)lockedData.getData() + offset);
	}
	

	void D3D11DynamicRHI::RHIUnlockIndexBuffer(RHIIndexBuffer* inIndexBuffer)
	{
		D3D11IndexBuffer* indexBuffer = ResourceCast(inIndexBuffer);

		D3D11_BUFFER_DESC desc;
		indexBuffer->mResource->GetDesc(&desc);
		const bool bIsDynamic = (desc.Usage == D3D11_USAGE_DYNAMIC);
		D3D11LockedKey lockedKey(indexBuffer->mResource);
		auto it = mOutstandingLocks.find(lockedKey);
		BOOST_ASSERT(it != mOutstandingLocks.end());
		if (bIsDynamic)
		{
			mD3D11Context->Unmap(indexBuffer->mResource, 0);
		}
		else
		{
			if (it->second.mStagingResource)
			{
				ID3D11Buffer* stagingBuffer = (ID3D11Buffer*)it->second.mStagingResource.getReference();
				mD3D11Context->Unmap(stagingBuffer, 0);
			}
			else
			{
				mD3D11Context->UpdateSubresource(indexBuffer->mResource, lockedKey.mSubresource, NULL, it->second.getData(), it->second.mPitch, 0);
				it->second.freeData();
			}
		}
		mOutstandingLocks.erase(it);
	}

#if WITH_D3DX_LIBS
	DXGI_FORMAT D3D11DynamicRHI::getPlatformTextureResourceFormat(DXGI_FORMAT inFormat, uint32 inFlags)
	{
		if (inFlags & TexCreate_Shared)
		{
			return DXGI_FORMAT_B8G8R8A8_UNORM;
		}
		return inFormat;
	}
#endif

	
}