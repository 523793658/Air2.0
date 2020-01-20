#pragma once
#include "RHI.h"

#include "D3D11RHI.h"
#include "D3D11Typedefs.h"
#include "D3D11StateCache.h"
#include "AMD/AMD_AGS/inc/amd_ags.h"
#include "D3D11ShaderResource.h"
#include "D3D11Viewport.h"
#include "D3D11ConstantBuffer.h"
#include "D3D11Util.h"
#include "D3D11Buffer.h"
#include "RHIContext.h"
#include "RHICommandList.h"
namespace Air
{

#define DX_MAX_MSAA_COUNT	8

	class D3D11ComputeShader;

	class D3D11RHI_API D3D11DynamicRHI : public DynamicRHI, public IRHICommandContextPSOFallback
	{
	public:
		friend class D3D11Viewport;

		template <typename TRHIType>
		static FORCEINLINE typename TD3D11ResourceTraits<TRHIType>::TConcreteType* ResourceCast(TRHIType* resource)
		{
			return static_cast<typename TD3D11ResourceTraits<TRHIType>::TConcreteType*>(resource);
		}

		D3D11DynamicRHI(IDXGIFactory1* inDXGIFactory1, D3D_FEATURE_LEVEL inFeatureLevel, int32 inChosenAdapter, const DXGI_ADAPTER_DESC& chosenDescription);

		void initUniformBuffers();

		virtual void init() override;

		virtual void shutdown() override;

		virtual IRHICommandContext* getDefualtContext() override;

		virtual void RHIAcquireThreadOwnership() override;

		virtual void RHIReleaseThreadOwnership() override;

		virtual TCHAR* getName() override;

		virtual void RHISetScissorRect(bool bEnable, uint32 minx, uint32 miny, uint32 maxx, uint32 maxy) override;

		virtual void updateMSAASettings() override;

		virtual Texture2DRHIRef RHIGetViewportBackBuffer(RHIViewport* viewport)override;

		virtual void RHIBeginDrawingViewport(RHIViewport* viewport, RHITexture* renderTargetRHI) override;

		virtual void RHIEndDrawingViewport(RHIViewport* viewport, bool bPresent, bool bLockToVsync) override;

		virtual void RHIBeginScene() override;

		virtual void RHIEndScene() override;

		virtual void RHIBeginFrame() override;

		virtual void RHIEndFrame() override;

		virtual void RHISetRenderTargets(uint32 NumSimultaneousRenderTargets, const RHIRenderTargetView* newRenderTargets, const RHIDepthRenderTargetView* newDepthStencilTarget, uint32 numUAVs, RHIUnorderedAccessView* const* UAVs) final override;

		virtual void RHISetBlendState(RHIBlendState* newState, const LinearColor& blendFactory) final override;

		virtual void RHISetDepthStencilState(RHIDepthStencilState* newState, uint32 inStencilRef) final override;

		virtual void RHISetRasterizerState(RHIRasterizerState* newState) final override;

		virtual void RHIResizeViewport(RHIViewport* viewport, uint32 sizeX, uint32 sizeY, bool isFullscreen, EPixelFormat preferredPixelFormat) override;

		virtual void RHIResizeViewport(RHIViewport* viewport, uint32 sizeX, uint32 sizeY, bool isFullscreen) override;



		virtual ViewportRHIRef RHICreateViewport(void* windowHandle, uint32 sizeX, uint32 sizeY, bool isFullscree, EPixelFormat preferredPixelFormat) final override;

		SamplerStateRHIRef RHICreateSamplerState(const SamplerStateInitializerRHI &inInitializerRHI) final override;

		RasterizerStateRHIRef RHICreateRasterizerState(const RasterizerStateInitializerRHI& inInitializerRHI) final override;

		DepthStencilStateRHIRef RHICreateDepthStencilState(const DepthStencilStateInitializerRHI& inInitializerRHI) final override;

		BlendStateRHIRef RHICreateBlendState(const BlendStateInitializerRHI& inInitializerRHI) final override;

		virtual void RHISetRenderTargetsAndClear(const RHISetRenderTargetsInfo& renderTargetsInfo) final override;

		virtual void setScissorRectIfRequiredWhenSettingViewport(uint32 minX, uint32 minY, uint32 maxX, uint32 maxY) {}

		virtual void RHISetViewport(uint32 MinX, uint32 MinY, float MinZ, uint32 maxX, uint32 maxY, float maxZ) override ;

		virtual void RHISetMultipleViewports(uint32 count, const ViewportBounds* data) override;

		void RHITransitionResources(EResourceTransitionAccess transitionType, RHITexture** inTexture, int32 numTextures) override;

		virtual void RHIAdvanceFrameForGetViewportBackBuffer() override;

		virtual Texture2DRHIRef RHICreateTexture2D(uint32 width, uint32 height, uint8 format, uint32 numMips, uint32 numSamplers, uint32 flags, RHIResourceCreateInfo& createInfo) override;

		virtual TextureCubeRHIRef RHICreateTextureCube(uint32 size, uint8 format, uint32 numMips, uint32 flags, RHIResourceCreateInfo& createInfo) override;

		virtual TextureReferenceRHIRef RHICreateTextureReference(LastRenderTimeContainer* lastRenderTime) override;

		virtual void* RHILockTextureCubeFace(RHITextureCube* texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, EResourceLockMode lockMode, uint32& destStride, bool bLockWithinMiptail) override;

		virtual void RHIUnlockTextureCubeFace(RHITextureCube* texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, bool bLockWithinMiptail) override;

		virtual void RHIUpdateTextureReference(RHITextureReference* textureRHI, RHITexture* newTexture) override;

		virtual uint32 RHIComputeMemorySize(RHITexture* textureRHI) final override;

		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView(RHITexture* textureRHI, const RHITextureSRVCreateInfo& createInfo) final override;

		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView(RHIVertexBuffer* vertexBuffer, uint32 stride, uint8 format) final override;

		virtual VertexDeclarationRHIRef RHICreateVertexDeclaration(const VertexDeclarationElementList& elements) final override;

		virtual ConstantBufferRHIRef RHICreateConstantBuffer(const void* contents, const RHIConstantBufferLayout& layout, EConstantBufferUsage usage, EConstantBufferValidation validation) final override;

		virtual void RHIUpdateConstantBuffer(RHIConstantBuffer* constantBufferRHI, const void* contents) final override;

		virtual VertexShaderRHIRef RHICreateVertexShader(const TArray<uint8>& code) final override;

		virtual HullShaderRHIRef RHICreateHullShader(const TArray<uint8>& code) final override;

		virtual DomainShaderRHIRef RHICreateDomainShader(const TArray<uint8>& code) final override;

		virtual GeometryShaderRHIRef RHICreateGeometryShader(const TArray<uint8>& code) final override;

		virtual PixelShaderRHIRef RHICreatePixelShader(const TArray<uint8>& code) final override;

		virtual ComputeShaderRHIRef RHICreateComputeShader(const TArray<uint8>& code) final override;

		virtual void RHIBindClearMRTValues(bool bClearColor, bool bClearDepth, bool bClearStencil) final override;

		virtual GeometryShaderRHIRef RHICreateGeometryShaderWithStreamOutput(const TArray<uint8>& code, const StreamOutElementList& elementList, uint32 numStrides, const uint32* strides, int32 rasterizedStream) final override;

		virtual void* RHILockTexture2D(RHITexture2D* texture, uint32 mipIndex, EResourceLockMode lockMode, uint32& destStride, bool bLockWithinMiptail) final override;

		virtual void RHIUnlockTexture2D(RHITexture2D* texture, uint32 mipIndex, bool bLockWithinMiptail) final override;

		virtual BoundShaderStateRHIRef RHICreateBoundShaderState(RHIVertexDeclaration* vertexDeclaration, RHIVertexShader* vertexShaderRHI, RHIHullShader* hullShaderRHI, RHIDomainShader* domainShaderRHI, RHIGeometryShader* geometryShaderRHI, RHIPixelShader* pixelShaderRHI) final override;

		virtual void RHISetBoundShaderState(RHIBoundShaderState* boundShaderState) final override;

		virtual void RHISetShaderConstantBuffer(RHIVertexShader* vertexShader, uint32 bufferIndex, RHIConstantBuffer* buffer) final override;

		virtual void RHISetShaderConstantBuffer(RHIHullShader* hullShader, uint32 bufferIndex, RHIConstantBuffer* buffer) final override;

		virtual void RHISetShaderConstantBuffer(RHIDomainShader* domainShader, uint32 bufferIndex, RHIConstantBuffer* buffer) final override;

		virtual void RHISetShaderConstantBuffer(RHIGeometryShader* geometryShader, uint32 bufferindex, RHIConstantBuffer* buffer) final override;

		virtual void RHISetShaderConstantBuffer(RHIPixelShader* pixelShader, uint32 bufferIndex, RHIConstantBuffer* buffer) final override;

		virtual void RHISetShaderConstantBuffer(RHIComputeShader* computeShader, uint32 bufferIndex, RHIConstantBuffer* buffer) final override;

		virtual void RHISetShaderParameter(RHIVertexShader* vertexShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) final override;

		virtual void RHISetShaderParameter(RHIHullShader* hullShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) final override;

		virtual void RHISetShaderParameter(RHIDomainShader* hullShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) final override;

		virtual void RHISetShaderParameter(RHIGeometryShader* hullShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) final override;

		virtual void RHISetShaderParameter(RHIPixelShader* hullShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) final override;
		static DXGI_FORMAT getPlatformTextureResourceFormat(DXGI_FORMAT inFormat, uint32 inFlags);

		virtual void RHISetShaderParameter(RHIComputeShader* hullShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) final override;

		virtual void RHISetShaderTexture(RHIVertexShader* vertexShader, uint32 textureIndex, RHITexture* newTexture) final override;

		virtual void RHISetShaderTexture(RHIHullShader* vertexShader, uint32 textureIndex, RHITexture* newTexture) final override;

		virtual void RHISetShaderTexture(RHIDomainShader* vertexShader, uint32 textureIndex, RHITexture* newTexture) final override;

		virtual void RHISetShaderTexture(RHIGeometryShader* vertexShader, uint32 textureIndex, RHITexture* newTexture) final override;

		virtual void RHISetShaderTexture(RHIPixelShader* vertexShader, uint32 textureIndex, RHITexture* newTexture) final override;

		virtual void RHISetShaderTexture(RHIComputeShader* vertexShader, uint32 textureIndex, RHITexture* newTexture) final override;

		virtual void RHISetShaderSampler(RHIVertexShader* vertexShader, uint32 samplerIndex, RHISamplerState* newSampler) final override;

		virtual void RHISetShaderSampler(RHIHullShader* vertexShader, uint32 samplerIndex, RHISamplerState* newSampler) final override;

		virtual void RHISetShaderSampler(RHIDomainShader* vertexShader, uint32 samplerIndex, RHISamplerState* newSampler) final override;

		virtual void RHISetShaderSampler(RHIGeometryShader* vertexShader, uint32 samplerIndex, RHISamplerState* newSampler) final override;

		virtual void RHISetShaderSampler(RHIPixelShader* vertexShader, uint32 samplerIndex, RHISamplerState* newSampler) final override;

		virtual void RHISetShaderSampler(RHIComputeShader* vertexShader, uint32 samplerIndex, RHISamplerState* newSampler) final override;

		virtual void RHISetStreamSource(uint32 streamIndex, RHIVertexBuffer* vertexBuffer, uint32 offset) final override;

		virtual void RHIDrawPrimitive(int32 baseVertexIndex, uint32 numPrimitives, uint32 numInstances) final override;

		virtual void RHIDrawIndexedPrimitive(RHIIndexBuffer* indexBuffer, uint32 primitiveType, int32 baseVertexIndex, uint32 firstInstance, uint32 numVertex, uint32 startIndex, uint32 numPrimitives, uint32 numInstances)final override;

		virtual VertexBufferRHIRef RHICreateVertexBuffer(uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo) final override;

		virtual void* RHILockVertexBuffer(RHIVertexBuffer* vertexBuffer, uint32 offset, uint32 size, EResourceLockMode lockMode) final override;

		virtual void RHIUnlockVertexBuffer(RHIVertexBuffer* vertexBuffer) final override;

		virtual IndexBufferRHIRef RHICreateIndexBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo) final override;

		virtual void* RHILockIndexBuffer(RHIIndexBuffer* indexBuffer, uint32 Offset, uint32 size, EResourceLockMode lockMode) final override;

		virtual void RHIUnlockIndexBuffer(RHIIndexBuffer* indexBuffer) final override;

		virtual void RHIReadSurfaceFloatData(RHITexture* texture, IntRect rect, TArray<Float16Color>& outData, ECubeFace cubeface, int32 arrayIndex, int32 mipIndex) final override;

		uint32 getMaxMSAAQuality(uint32 sampleCount);
		IDXGIFactory1* getFactory() const
		{
			return mDXGIFactory1;
		}

		template<typename BaseResourceType>
		TD3D11Texture2D<BaseResourceType>* createD3D11Texture2D(uint32 width, uint32 height, uint32 size, bool bTextureArray, bool CubeTexture, uint8 format, uint32 numMips, uint32 numSamplers, uint32 flags, RHIResourceCreateInfo& createInfo);

		template<EShaderFrequency ShaderFrequency>
		void setShaderResourceView(D3D11BaseShaderResource* resource, ID3D11ShaderResourceView* srv, int32 resourceIndex, wstring name, D3D11StateCache::ESRV_Type srvType = D3D11StateCache::SRV_Unknown)
		{
			internalSetShaderResourceView<ShaderFrequency>(resource, srv, resourceIndex, name, srvType);
		}

		void RHIBindDebugLabelName(RHITexture* texture, const TCHAR* name) override final;

		virtual void RHISetGraphicsPipelineState(RHIGraphicsPipelineState* graphicsState) final override
		{
			RHIGraphicsPipelineStateFallBack* fallbackGraphicsState = static_cast<RHIGraphicsPipelineStateFallBack*>(graphicsState);
			IRHICommandContextPSOFallback::RHISetGraphicsPipelineState(graphicsState);
			mPrimitiveType = fallbackGraphicsState->mInitializer.mPrimitiveType;
		}

		virtual UnorderedAccessViewRHIRef RHICreateUnorderedAccessView(RHIIndexBuffer* indexBuffer, uint8 format) final override;

		virtual UnorderedAccessViewRHIRef RHICreateUnorderedAccessView(RHIStructuredBuffer* structuredBuffer, bool bUseUAVCounter, bool bAppendBuffer) final override;

		virtual UnorderedAccessViewRHIRef RHICreateUnorderedAccessView(RHITexture* texture, uint32 mipLevel) final override;

		virtual UnorderedAccessViewRHIRef RHICreateUnorderedAccessView(RHIVertexBuffer* vertexBuffer, uint8 format) final override;

		virtual void RHISetShaderResourceViewParameter(RHIVertexShader* vertexShader, uint32 samplerIndex, RHIShaderResourceView* srv) final override;

		virtual void RHISetShaderResourceViewParameter(RHIDomainShader* domainShader, uint32 samplerIndex, RHIShaderResourceView* srv) final override;


		virtual void RHISetShaderResourceViewParameter(RHIHullShader* hullShader, uint32 samplerIndex, RHIShaderResourceView* srv) final override;

		virtual void RHISetShaderResourceViewParameter(RHIGeometryShader* geometryShader, uint32 samplerIndex, RHIShaderResourceView* srv) final override;

		virtual void RHISetShaderResourceViewParameter(RHIPixelShader* pixelShader, uint32 samplerIndex, RHIShaderResourceView* srv) final override;

		virtual void RHISetShaderResourceViewParameter(RHIComputeShader* computeShader, uint32 samplerIndex, RHIShaderResourceView* srv) final override;

		virtual StructuredBufferRHIRef RHICreateStructuredBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo) final override;

		virtual void RHICopyToResolveTarget(RHITexture* sourceTexture, RHITexture* destTexture, const ResolveParams& resolveParams) final override;

		virtual void RHISetUAVParameter(RHIComputeShader* computeShader, uint32 uavIndex, RHIUnorderedAccessView* uav, uint32 initialCount)final override;

		virtual void RHISetUAVParameter(RHIComputeShader* computeShader, uint32 uavIndex, RHIUnorderedAccessView* uav) final override;

		virtual void RHISetComputeShader(RHIComputeShader* computeShader) final override;

		virtual void RHIDispatchComputeShader(uint32 threadGroupCountX, uint32 threadGroupCountY, uint32 threadGroupCountZ) final override;

		virtual void RHIFlushComputeShaderCache() final override;

		virtual void RHIAutomaticCacheFlushAfterComputeShader(bool bEnable) final override;

		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView(RHIStructuredBuffer*) final override;

		virtual void RHISubmitCommandsHint() final override;

		virtual void RHIEnableDepthBoundsTest(bool bEnable) final override
		{
			if (GSupportsDepthBoundsTest && mStateCache.bDepthBoundsEnabled != bEnable)
			{
				enableDepthBoundsTest(bEnable, 0.0f, 1.0f);
			}
		}
		virtual void RHISetDepthBounds(float minDepth, float maxDepth) final override
		{
			if (GSupportsDepthBoundsTest && (mStateCache.mDepthBoundsMin != minDepth || mStateCache.mDepthBoundsMax != maxDepth))
			{
				enableDepthBoundsTest(true, minDepth, maxDepth);
			}
		}

	public:
		void clearState();

		ID3D11Device* getDevice() const
		{
			return mD3D11Device;
		}
		ID3D11DeviceContext* getDeviceContext() const
		{
			return mD3D11Context;
		}

	private:
		void initD3DDevice();

		void setupAfterDeviceCreation();

		void conditionalClearShaderResource(D3D11BaseShaderResource* resource, bool bCheckBoundInputAssember);

		template<EShaderFrequency ShaderFrequency>
		void clearShaderResourceViews(D3D11BaseShaderResource* resource);


		void clearAllShaderResources();


		void commitRenderTargetsAndUAVs();

		template<EShaderFrequency ShaderFrequency>
		void clearAllShaderResourcesForFrequency();


		template <EShaderFrequency ShaderFrequency>
		void internalSetShaderResourceView(D3D11BaseShaderResource* resource, ID3D11ShaderResourceView* srv, int32 resourceIndex, wstring name, D3D11StateCacheBase::ESRV_Type srvType = D3D11StateCacheBase::ESRV_Type::SRV_Unknown);


		template<EShaderFrequency ShaderFrequency>
		void RHISetShaderResourceViewParameter_Internal(RHIShaderResourceView* srvRHI, uint32 textureIndex, wstring name);
	

		virtual void RHIClearMRTImpl(bool bClearColor, int32 numClearColor, const LinearColor* colorArray, bool bClearDepth, float depth, bool bClearStencil, uint32 stencil);

		void validateExclusiveDepthStencilAccess(FExclusiveDepthStencil src) const;

		void trackResourceBoundAsVB(D3D11BaseShaderResource* resource, int32 streamIndex);
		void trackResourceBoundAsIB(D3D11BaseShaderResource* resource);

		void setCurrentComputeShader(RHIComputeShader* computeShader)
		{
			mCurrentComputeShader = computeShader;
		}

		const ComputeShaderRHIRef getCurrentComputeShader() const
		{
			return mCurrentComputeShader;
		}

		void beginUAVOverlap();
		void endUAVOverlap();

		void enableDepthBoundsTest(bool bEnable, float minDepth, float maxDepth);
	protected:
		IDXGIFactory1Ptr mDXGIFactory1;
		D3D_FEATURE_LEVEL	mFeatureLevel;
		
		ID3D11DevicePtr		mD3D11Device;
		ID3D11DeviceContextPtr mD3D11Context;

		DXGI_ADAPTER_DESC	mChosenDescription;
		int32 mChosenAdapter;

		uint32 mPresentCounter;

		AGSContext* mAmdAgsContext;

		D3D11StateCache mStateCache;

		EPrimitiveType mPrimitiveType;

		uint32 mAvailableMSAAQualities[DX_MAX_MSAA_COUNT + 1];

		D3D11BaseShaderResource* mCurrentResourceBoundAsSRVs[SF_NumFrequencies][D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
		D3D11BaseShaderResource* mCurrentResourceBoundAsVBs[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
		D3D11BaseShaderResource* mCurrentResourceBoundAsIB;
		int32 mMaxBoundShaderResourceIndex[SF_NumFrequencies];
		int32 mMaxBoundVertexBufferIndex;
		uint16 mDirtyConstantBuffers[SF_NumFrequencies];

		ConstantBufferRHIRef mBoundConstantBuffers[SF_NumFrequencies][MAX_CONSTANT_BUFFERS_PER_SHADER_STAGE];

		void* mZeroBuffer;

		TRefCountPtr<D3D11Viewport> mDrawingViewport;

		TRefCountPtr<D3D11TextureBase>	mCurrentDepthTexture;

		TRefCountPtr<ID3D11DepthStencilView> mCurrentDepthStencilTarget;

		TRefCountPtr<ID3D11RenderTargetView> mCurrentRenderTargets[MaxSimultaneousRenderTargets];

		TRefCountPtr<ID3D11UnorderedAccessView> mCurrentUAVs[D3D11_PS_CS_UAV_REGISTER_COUNT];

		TArray<TRefCountPtr<D3D11UniformBuffer>> mVSUnifomBuffers;
		TArray<TRefCountPtr<D3D11UniformBuffer>> mHSUnifomBuffers;
		TArray<TRefCountPtr<D3D11UniformBuffer>> mDSUnifomBuffers;
		TArray<TRefCountPtr<D3D11UniformBuffer>> mGSUnifomBuffers;
		TArray<TRefCountPtr<D3D11UniformBuffer>> mPSUnifomBuffers;
		TArray<TRefCountPtr<D3D11UniformBuffer>> mCSUnifomBuffers;

		bool bRenderDoc = false;


		uint32 mNumUAVs{ 0 };

		uint32 mNumSimultaneousRenderTargets{ 0 };

		TArray<D3D11Viewport*> mViewports;

		TGlobalResource<TBoundShaderStateHistory<10000>> mBoundShaderStateHistory;

		bool bUsingTessellation{ false };
		bool bDiscardSharedConstants;


		uint32 mPendingNumVertices;
		uint32 mPendingVertexDataStride;
		uint32 mPendingPrimitiveType;
		uint32 mPendingNumPrimitives;
		uint32 mPendingMinVertexIndex;
		uint32 mPendingNumIndices;
		uint32 mPendingIndexDataStride;

		TRefCountPtr<D3D11DynamicBuffer> mDynamicVB;
		TRefCountPtr<D3D11DynamicBuffer> mDynamicIB;

		ComputeShaderRHIRef mCurrentComputeShader;
	public:
		TMap<D3D11LockedKey, D3D11LockedData> mOutstandingLocks;

	protected:
		FExclusiveDepthStencil mCurrentDSVAccessType;


		void commitGraphicsResourceTables();

		void commitComputeResourceTables(D3D11ComputeShader* computeShader);

		virtual void commitComputeShaderConstants();

		virtual void commitNonComputeShaderConstants();

		template<class ShaderType>
		void setResourcesFromTables(const ShaderType* RESTRICT);

		template<typename TPixelShader>
		void resolveTextureUsingShader(RHICommandList_RecursiveHazardous& RHICmdList, D3D11Texture2D* sourceTexture, D3D11Texture2D* destTexture, ID3D11RenderTargetView* destSurfaceRTV, ID3D11DepthStencilView* DestSurfaceDSV, const D3D11_TEXTURE2D_DESC& resolveTargetDesc, const ResolveRect& sourceRect, ResolveRect& destRect, ID3D11DeviceContext* direct3DDeviceContext, typename TPixelShader::Parameter pixelShaderParameter);
	};

}