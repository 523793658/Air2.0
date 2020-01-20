#include "D3D11DynamicRHI.h"
#include "d3d11shader.h"
#include "D3D11Resource.h"
#include "D3D11State.h"
#include "D3D11UnorderedAccessView.h"
#include "NVIDIA/nvapi/nvapi.h"
namespace Air
{

	static int32 GUnbindResourcesBetweenDrawsInDX11 = false;

#define DECLARE_ISBOUNDSHADER(ShaderType) inline void validateBoundShader(D3D11StateCache& inStateCache, RHI##ShaderType* ShaderType##RHI)	  \
{	\
	ID3D11##ShaderType*	cachedShader;\
	inStateCache.get##ShaderType(&cachedShader);	\
	D3D11##ShaderType* ShaderType = D3D11DynamicRHI::ResourceCast(ShaderType##RHI);\
	BOOST_ASSERT(cachedShader == ShaderType->mResource);	\
	if (cachedShader)	\
	{	\
		cachedShader->Release();	\
	}\
}

	DECLARE_ISBOUNDSHADER(VertexShader);
	DECLARE_ISBOUNDSHADER(PixelShader);
	DECLARE_ISBOUNDSHADER(GeometryShader);
	DECLARE_ISBOUNDSHADER(HullShader);
	DECLARE_ISBOUNDSHADER(DomainShader);
	DECLARE_ISBOUNDSHADER(ComputeShader);

#if DO_CHECK
#define VALIDATE_BOUND_SHADER(s) validateBoundShader(mStateCache, s)
#else 
#define VALIDATE_BOUND_SHADER(s)
#endif

	template<EShaderFrequency ShaderFrequency>
	void D3D11DynamicRHI::RHISetShaderResourceViewParameter_Internal(RHIShaderResourceView* srvRHI, uint32 textureIndex, wstring name)
	{
		D3D11ShaderResourceView* srv = ResourceCast(srvRHI);
		D3D11BaseShaderResource* resource = nullptr;
		ID3D11ShaderResourceView* d3d11Srv = nullptr;

		if (srv)
		{
			d3d11Srv = srv->mView;
			resource = srv->mResource;
		}
		setShaderResourceView<ShaderFrequency>(resource, d3d11Srv, textureIndex, name);
	}

	void D3D11DynamicRHI::RHISetShaderResourceViewParameter(RHIVertexShader* vertexShaderRHI, uint32 samplerIndex, RHIShaderResourceView* srvRHI)
	{
		VALIDATE_BOUND_SHADER(vertexShaderRHI);
		
		RHISetShaderResourceViewParameter_Internal<SF_Vertex>(srvRHI, samplerIndex, Name_None);
	}

	void D3D11DynamicRHI::RHISetShaderResourceViewParameter(RHIDomainShader* domainShader, uint32 samplerIndex, RHIShaderResourceView* srvRHI)
	{
		VALIDATE_BOUND_SHADER(domainShader);

		RHISetShaderResourceViewParameter_Internal<SF_Domain>(srvRHI, samplerIndex, Name_None);

	}


	void D3D11DynamicRHI::RHISetShaderResourceViewParameter(RHIHullShader* hullShader, uint32 samplerIndex, RHIShaderResourceView* srv)
	{
		VALIDATE_BOUND_SHADER(hullShader);

		RHISetShaderResourceViewParameter_Internal<SF_Hull>(srv, samplerIndex, Name_None);
	}

	void D3D11DynamicRHI::RHISetShaderResourceViewParameter(RHIGeometryShader* geometryShader, uint32 samplerIndex, RHIShaderResourceView* srv)
	{
		VALIDATE_BOUND_SHADER(geometryShader);

		RHISetShaderResourceViewParameter_Internal<SF_Geometry>(srv, samplerIndex, Name_None);
	}

	void D3D11DynamicRHI::RHISetShaderResourceViewParameter(RHIPixelShader* pixelShader, uint32 samplerIndex, RHIShaderResourceView* srv)
	{
		VALIDATE_BOUND_SHADER(pixelShader);

		RHISetShaderResourceViewParameter_Internal<SF_Pixel>(srv, samplerIndex, Name_None);
	}

	void D3D11DynamicRHI::RHISetShaderResourceViewParameter(RHIComputeShader* computeShader, uint32 samplerIndex, RHIShaderResourceView* srv)
	{
		VALIDATE_BOUND_SHADER(computeShader);

		RHISetShaderResourceViewParameter_Internal<SF_Compute>(srv, samplerIndex, Name_None);
	}

	void D3D11DynamicRHI::RHISetShaderSampler(RHIVertexShader* vertexShader, uint32 samplerIndex, RHISamplerState* newSampler)
	{
		VALIDATE_BOUND_SHADER(vertexShader);
		D3D11VertexShader* shader = ResourceCast(vertexShader);
		D3D11SamplerState* sampler = ResourceCast(newSampler);
		ID3D11SamplerState* stateResource = sampler->mResource;
		mStateCache.setSamplerState<SF_Vertex>(stateResource, samplerIndex);
	}

	void D3D11DynamicRHI::RHISetShaderSampler(RHIHullShader* hullShader, uint32 samplerIndex, RHISamplerState* newSampler)
	{
		VALIDATE_BOUND_SHADER(hullShader);
		D3D11HullShader* shader = ResourceCast(hullShader);
		D3D11SamplerState* sampler = ResourceCast(newSampler);
		ID3D11SamplerState* stateResource = sampler->mResource;
		mStateCache.setSamplerState<SF_Hull>(stateResource, samplerIndex);
	}

	void D3D11DynamicRHI::RHISetShaderSampler(RHIDomainShader* domainShader, uint32 samplerIndex, RHISamplerState* newSampler)
	{
		VALIDATE_BOUND_SHADER(domainShader);
		D3D11DomainShader* shader = ResourceCast(domainShader);
		D3D11SamplerState* sampler = ResourceCast(newSampler);
		ID3D11SamplerState* stateResource = sampler->mResource;
		mStateCache.setSamplerState<SF_Domain>(stateResource, samplerIndex);
	}

	void D3D11DynamicRHI::RHISetShaderSampler(RHIGeometryShader* geometryShader, uint32 samplerIndex, RHISamplerState* newSampler)
	{
		VALIDATE_BOUND_SHADER(geometryShader);
		D3D11GeometryShader* shader = ResourceCast(geometryShader);
		D3D11SamplerState* sampler = ResourceCast(newSampler);
		ID3D11SamplerState* stateResource = sampler->mResource;
		mStateCache.setSamplerState<SF_Geometry>(stateResource, samplerIndex);
	}

	void D3D11DynamicRHI::RHISetShaderSampler(RHIPixelShader* pixelShader, uint32 samplerIndex, RHISamplerState* newSampler)
	{
		VALIDATE_BOUND_SHADER(pixelShader);
		D3D11PixelShader* shader = ResourceCast(pixelShader);
		D3D11SamplerState* sampler = ResourceCast(newSampler);
		ID3D11SamplerState* stateResource = sampler->mResource;
		mStateCache.setSamplerState<SF_Pixel>(stateResource, samplerIndex);
	}

	/*template<typename ShaderType, EShaderFrequency Frequency>
	void D3D11DynamicRHI::_RHISetShaderTexture(ShaderType* ShaderRHI, uint32 textureIndex, RHITexture* newTextureRHI)
	{
		VALIDATE_BOUND_SHADER(ShaderRHI);
		D3D11TextureBase* newTexture = getD3D11TextureFromRHITexture(newTextureRHI);
		ID3D11ShaderResourceView* shaderResourceView = newTexture ? newTexture->getShaderResourceView() : NULL;
		if ((newTexture == NULL) || (newTexture->getRenderTargetView(0, 0) != NULL) || (newTexture->hasDepthStencilView()))
		{
			setShaderResourceView<Frequency>(newTexture, shaderResourceView, textureIndex, newTextureRHI ? newTextureRHI->getName() : TEXT(""), D3D11StateCache::SRV_Dynamic);
		}
		else
		{
			setShaderResourceView<Frequency>(newTexture, shaderResourceView, textureIndex, newTextureRHI->getName(), D3D11StateCache::SRV_Static);
		}
	}*/

	void D3D11DynamicRHI::RHISetShaderTexture(RHIVertexShader* vertexShader, uint32 textureIndex, RHITexture* newTextureRHI)
	{
		VALIDATE_BOUND_SHADER(vertexShader);
		D3D11TextureBase* newTexture = getD3D11TextureFromRHITexture(newTextureRHI);
		ID3D11ShaderResourceView* shaderResourceView = newTexture ? newTexture->getShaderResourceView() : NULL;
		if ((newTexture == NULL) || (newTexture->getRenderTargetView(0, 0) != NULL) || (newTexture->hasDepthStencilView()))
		{
			setShaderResourceView<SF_Vertex>(newTexture, shaderResourceView, textureIndex, newTextureRHI ? newTextureRHI->getName() : TEXT(""), D3D11StateCache::SRV_Dynamic);
		}
		else
		{
			setShaderResourceView<SF_Vertex>(newTexture, shaderResourceView, textureIndex, newTextureRHI->getName(), D3D11StateCache::SRV_Static);
		}
	}

	void D3D11DynamicRHI::RHISetShaderTexture(RHIHullShader* hullShader, uint32 textureIndex, RHITexture* newTextureRHI)
	{
		VALIDATE_BOUND_SHADER(hullShader);
		D3D11TextureBase* newTexture = getD3D11TextureFromRHITexture(newTextureRHI);
		ID3D11ShaderResourceView* shaderResourceView = newTexture ? newTexture->getShaderResourceView() : NULL;
		if ((newTexture == NULL) || (newTexture->getRenderTargetView(0, 0) != NULL) || (newTexture->hasDepthStencilView()))
		{
			setShaderResourceView<SF_Hull>(newTexture, shaderResourceView, textureIndex, newTextureRHI ? newTextureRHI->getName() : TEXT(""), D3D11StateCache::SRV_Dynamic);
		}
		else
		{
			setShaderResourceView<SF_Hull>(newTexture, shaderResourceView, textureIndex, newTextureRHI->getName(), D3D11StateCache::SRV_Static);
		}
	}


	void D3D11DynamicRHI::RHISetShaderTexture(RHIDomainShader* domainShader, uint32 textureIndex, RHITexture* newTextureRHI)
	{
		VALIDATE_BOUND_SHADER(domainShader);
		D3D11TextureBase* newTexture = getD3D11TextureFromRHITexture(newTextureRHI);
		ID3D11ShaderResourceView* shaderResourceView = newTexture ? newTexture->getShaderResourceView() : NULL;
		if ((newTexture == NULL) || (newTexture->getRenderTargetView(0, 0) != NULL) || (newTexture->hasDepthStencilView()))
		{
			setShaderResourceView<SF_Domain>(newTexture, shaderResourceView, textureIndex, newTextureRHI ? newTextureRHI->getName() : TEXT(""), D3D11StateCache::SRV_Dynamic);
		}
		else
		{
			setShaderResourceView<SF_Domain>(newTexture, shaderResourceView, textureIndex, newTextureRHI->getName(), D3D11StateCache::SRV_Static);
		}
	}


	void D3D11DynamicRHI::RHISetShaderTexture(RHIGeometryShader* geometryShader, uint32 textureIndex, RHITexture* newTextureRHI)
	{
		VALIDATE_BOUND_SHADER(geometryShader);
		D3D11TextureBase* newTexture = getD3D11TextureFromRHITexture(newTextureRHI);
		ID3D11ShaderResourceView* shaderResourceView = newTexture ? newTexture->getShaderResourceView() : NULL;
		if ((newTexture == NULL) || (newTexture->getRenderTargetView(0, 0) != NULL) || (newTexture->hasDepthStencilView()))
		{
			setShaderResourceView<SF_Geometry>(newTexture, shaderResourceView, textureIndex, newTextureRHI ? newTextureRHI->getName() : TEXT(""), D3D11StateCache::SRV_Dynamic);
		}
		else
		{
			setShaderResourceView<SF_Geometry>(newTexture, shaderResourceView, textureIndex, newTextureRHI->getName(), D3D11StateCache::SRV_Static);
		}
	}


	void D3D11DynamicRHI::RHISetShaderTexture(RHIPixelShader* pixelShader, uint32 textureIndex, RHITexture* newTextureRHI)
	{
		VALIDATE_BOUND_SHADER(pixelShader);
		D3D11TextureBase* newTexture = getD3D11TextureFromRHITexture(newTextureRHI);
		ID3D11ShaderResourceView* shaderResourceView = newTexture ? newTexture->getShaderResourceView() : NULL;
		if ((newTexture == NULL) || (newTexture->getRenderTargetView(0, 0) != NULL) || (newTexture->hasDepthStencilView()))
		{
			setShaderResourceView<SF_Geometry>(newTexture, shaderResourceView, textureIndex, newTextureRHI ? newTextureRHI->getName() : TEXT(""), D3D11StateCache::SRV_Dynamic);
		}
		else
		{
			setShaderResourceView<SF_Geometry>(newTexture, shaderResourceView, textureIndex, newTextureRHI->getName(), D3D11StateCache::SRV_Static);
		}
	}




	void D3D11DynamicRHI::RHISetShaderTexture(RHIComputeShader* computeShader, uint32 textureIndex, RHITexture* newTextureRHI)
	{
		VALIDATE_BOUND_SHADER(computeShader);
		D3D11TextureBase* newTexture = getD3D11TextureFromRHITexture(newTextureRHI);
		ID3D11ShaderResourceView* shaderResourceView = newTexture ? newTexture->getShaderResourceView() : NULL;
		if ((newTexture == NULL) || (newTexture->getRenderTargetView(0, 0) != NULL) || (newTexture->hasDepthStencilView()))
		{
			setShaderResourceView<SF_Compute>(newTexture, shaderResourceView, textureIndex, newTextureRHI ? newTextureRHI->getName() : TEXT(""), D3D11StateCache::SRV_Dynamic);
		}
		else
		{
			setShaderResourceView<SF_Compute>(newTexture, shaderResourceView, textureIndex, newTextureRHI->getName(), D3D11StateCache::SRV_Static);
		}
	}


	



	void D3D11DynamicRHI::RHISetUAVParameter(RHIComputeShader* computeShader, uint32 uavIndex, RHIUnorderedAccessView* uavRHI)
	{
		D3D11UnorderedAccessView* uav = ResourceCast(uavRHI);
		if (uav)
		{
			conditionalClearShaderResource(uav->mResource, true);

			const EResourceTransitionAccess currentUAVAccess = uav->mResource->getCurrentGPUAccess();
			const bool uavDirty = uav->mResource->isDirty();
			//BOOST_ASSERT(genabledx)
			uav->mResource->setDirty(true, mPresentCounter);
		}

		ID3D11UnorderedAccessView* d3d11UAV = uav ? uav->mView : NULL;
		uint32 initialCount = -1;
		mD3D11Context->CSSetUnorderedAccessViews(uavIndex, 1, &d3d11UAV, &initialCount);
	}

	void D3D11DynamicRHI::RHISetUAVParameter(RHIComputeShader* computeShader, uint32 uavIndex, RHIUnorderedAccessView* uavRHI, uint32 initialCount)
	{
		D3D11UnorderedAccessView* uav = ResourceCast(uavRHI);
		if (uav)
		{
			conditionalClearShaderResource(uav->mResource, true);

			const EResourceTransitionAccess currentUAVAccess = uav->mResource->getCurrentGPUAccess();
			const bool uavDirty = uav->mResource->isDirty();
			//BOOST_ASSERT(genabledx)
			uav->mResource->setDirty(true, mPresentCounter);
		}

		ID3D11UnorderedAccessView* d3d11UAV = uav ? uav->mView : NULL;
		mD3D11Context->CSSetUnorderedAccessViews(uavIndex, 1, &d3d11UAV, &initialCount);
	}

	void D3D11DynamicRHI::RHISetComputeShader(RHIComputeShader* computeShaderRHI)
	{
		D3D11ComputeShader* computeShader = ResourceCast(computeShaderRHI);
		setCurrentComputeShader(computeShaderRHI);
	}

	void D3D11DynamicRHI::RHIDispatchComputeShader(uint32 threadGroupCountX, uint32 threadGroupCountY, uint32 threadGroupCountZ)
	{
		RHIComputeShader* computeShaderRHI = getCurrentComputeShader();
		D3D11ComputeShader* computeShader = ResourceCast(computeShaderRHI);

		mStateCache.setComputeShader(computeShader->mResource);

		if (computeShader->bShaderNeedsGlobalConstantBuffer)
		{
			commitComputeShaderConstants();
		}
		commitComputeResourceTables(computeShader);

		mD3D11Context->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
		mStateCache.setComputeShader(nullptr);
	}

	static bool GAllowUAVFlushExt = true;
	static bool GOverlapUAVOBegin = false;

	static bool isUAVOverlapSupported()
	{
		if (!GAllowUAVFlushExt || (!isRHIDeviceNVIDIA() && !isRHIDeviceAMD()))
		{
			return false;
		}
		return true;
	}
#if 0
#else
#define CHECK_AGS(x) {(x);}
#define CHECK_NVAPI(x) {(x);}
#endif



	void D3D11DynamicRHI::endUAVOverlap()
	{
		if (GOverlapUAVOBegin)
		{
			if (isRHIDeviceNVIDIA())
			{
				CHECK_NVAPI(NvAPI_D3D11_EndUAVOverlap(mD3D11Device));
			}
			else if(isRHIDeviceAMD())
			{
				CHECK_AGS(agsDriverExtensionsDX11_EndUAVOverlap(mAmdAgsContext));
			}
			else
			{
				BOOST_ASSERT(false);
			}

			GOverlapUAVOBegin = false;
		}
	}

	void D3D11DynamicRHI::beginUAVOverlap()
	{
		if (GOverlapUAVOBegin)
		{
			if (isRHIDeviceNVIDIA())
			{
				CHECK_NVAPI(NvAPI_D3D11_BeginUAVOverlap(mD3D11Device));
			}
			else if (isRHIDeviceAMD())
			{
				CHECK_AGS(agsDriverExtensionsDX11_BeginUAVOverlap(mAmdAgsContext));
			}
			else
			{
				BOOST_ASSERT(false);
			}

			GOverlapUAVOBegin = true;
		}
	}

	void D3D11DynamicRHI::RHIFlushComputeShaderCache()
	{
		if (!isUAVOverlapSupported())
		{
			return;
		}
		endUAVOverlap();
	}

	void D3D11DynamicRHI::RHIAutomaticCacheFlushAfterComputeShader(bool bEnable)
	{
		const bool bCVarEnabled = true;
		if (GAllowUAVFlushExt != bCVarEnabled)
		{
			endUAVOverlap();
		}

		GAllowUAVFlushExt = bCVarEnabled;

		if (!isUAVOverlapSupported())
		{
			return;
		}

		if (bEnable)
		{
			endUAVOverlap();
		}
		else
		{
			beginUAVOverlap();
		}
	}

	void D3D11DynamicRHI::enableDepthBoundsTest(bool bEnable, float minDepth, float maxDepth)
	{
#if PLATFORM_DESKTOP
		if (minDepth > maxDepth)
		{
			return;
		}

		if (minDepth < 0.f || maxDepth > 1.f)
		{

		}

		minDepth = Math::clamp(minDepth, 0.0f, 1.0f);
		maxDepth = Math::clamp(maxDepth, 0.0f, 1.0f);

		if (isRHIDeviceNVIDIA())
		{
			auto result = NvAPI_D3D11_SetDepthBoundsTest(mD3D11Device, bEnable, minDepth, maxDepth);
			if (result != NVAPI_OK)
			{
				static bool bOnce = false;
				if (!bOnce)
				{
					bOnce = true;
					if (bRenderDoc)
					{

					}
				}
			}
		}
		else if (isRHIDeviceAMD())
		{
			auto result = agsDriverExtensionsDX11_SetDepthBounds(mAmdAgsContext, bEnable, minDepth, maxDepth);
			if (result != AGS_SUCCESS)
			{
				static bool bOnce = false;
				if (!bOnce)
				{
					bOnce = true;
					if (bRenderDoc)
					{

					}
					else
					{

					}
				}
			}
		}
#endif
		mStateCache.bDepthBoundsEnabled = bEnable;
		mStateCache.mDepthBoundsMin = minDepth;
		mStateCache.mDepthBoundsMax = maxDepth;
	}
}