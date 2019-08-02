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
#include "RHICommandList.h"
namespace Air
{

#define DX_MAX_MSAA_COUNT	8

	class D3D11ComputeShader;

	class D3D11RHI_API D3D11DynamicRHI : public DynamicRHI, public IRHICommandContext
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

		virtual Texture2DRHIRef RHIGetViewportBackBuffer(ViewportRHIParamRef viewport)override;

		virtual void RHIBeginDrawingViewport(ViewportRHIParamRef viewport, TextureRHIParamRef renderTargetRHI) override;

		virtual void RHIEndDrawingViewport(ViewportRHIParamRef viewport, bool bPresent, bool bLockToVsync) override;

		virtual void RHIBeginScene() override;

		virtual void RHIEndScene() override;

		virtual void RHIBeginFrame() override;

		virtual void RHIEndFrame() override;

		virtual void RHISetRenderTargets(uint32 NumSimultaneousRenderTargets, const RHIRenderTargetView* newRenderTargets, const RHIDepthRenderTargetView* newDepthStencilTarget, uint32 numUAVs, const UnorderedAccessViewRHIParamRef* UAVs) final override;

		virtual void RHISetBlendState(BlendStateRHIParamRef newState, const LinearColor& blendFactory) final override;

		virtual void RHISetDepthStencilState(DepthStencilStateRHIParamRef newState, uint32 inStencilRef) final override;

		virtual void RHISetRasterizerState(RasterizerStateRHIParamRef newState) final override;

		virtual void RHICopyToResolveTarget(TextureRHIParamRef sourceTexture, TextureRHIParamRef destTexture, bool keepOriginalSurface, const ResolveParams& resolveParams) final override;

		virtual void RHIResizeViewport(ViewportRHIParamRef viewport, uint32 sizeX, uint32 sizeY, bool isFullscreen, EPixelFormat preferredPixelFormat) override;

		virtual ViewportRHIParamRef RHICreateViewport(void* windowHandle, uint32 sizeX, uint32 sizeY, bool isFullscree, EPixelFormat preferredPixelFormat) final override;

		SamplerStateRHIRef RHICreateSamplerState(const SamplerStateInitializerRHI &inInitializerRHI) final override;

		RasterizerStateRHIRef RHICreateRasterizerState(const RasterizerStateInitializerRHI& inInitializerRHI) final override;

		DepthStencilStateRHIRef RHICreateDepthStencilState(const DepthStencilStateInitializerRHI& inInitializerRHI) final override;

		BlendStateRHIRef RHICreateBlendState(const BlendStateInitializerRHI& inInitializerRHI) final override;

		virtual void RHISetRenderTargetsAndClear(const RHISetRenderTargetsInfo& renderTargetsInfo) final override;

		virtual void setScissorRectIfRequiredWhenSettingViewport(uint32 minX, uint32 minY, uint32 maxX, uint32 maxY) {}

		virtual void RHISetViewport(uint32 MinX, uint32 MinY, float MinZ, uint32 maxX, uint32 maxY, float maxZ) override ;

		virtual void RHISetMultipleViewports(uint32 count, const ViewportBounds* data) override;

		void RHITransitionResources(EResourceTransitionAccess transitionType, TextureRHIParamRef* inTexture, int32 numTextures) override;

		virtual void RHIAdvanceFrameForGetViewportBackBuffer() override;

		virtual Texture2DRHIRef RHICreateTexture2D(uint32 width, uint32 height, uint8 format, uint32 numMips, uint32 numSamplers, uint32 flags, RHIResourceCreateInfo& createInfo) override;

		virtual TextureCubeRHIRef RHICreateTextureCube(uint32 size, uint8 format, uint32 numMips, uint32 flags, RHIResourceCreateInfo& createInfo) override;

		virtual TextureReferenceRHIRef RHICreateTextureReference(LastRenderTimeContainer* lastRenderTime) override;

		virtual void* RHILockTextureCubeFace(TextureCubeRHIParamRef texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, EResourceLockMode lockMode, uint32& destStride, bool bLockWithinMiptail) override;

		virtual void RHIUnlockTextureCubeFace(TextureCubeRHIParamRef texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, bool bLockWithinMiptail) override;

		virtual void RHIUpdateTextureReference(TextureReferenceRHIParamRef textureRHI, TextureRHIParamRef newTexture) override;

		virtual uint32 RHIComputeMemorySize(TextureRHIParamRef textureRHI) final override;

		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView(Texture2DRHIParamRef texture2DRHI, uint32 mipLevel) final override;

		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView(Texture2DRHIParamRef texture2DRHI, uint32 mipLevel, uint8 numMipLevel, uint8 format) final override;

		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView(VertexBufferRHIParamRef vertexBuffer, uint32 stride, uint8 format) final override;

		virtual VertexDeclarationRHIRef RHICreateVertexDeclaration(const VertexDeclarationElementList& elements) final override;

		virtual ConstantBufferRHIRef RHICreateConstantBuffer(const void* contents, const RHIConstantBufferLayout& layout, EConstantBufferUsage usage) final override;

		virtual VertexShaderRHIRef RHICreateVertexShader(const TArray<uint8>& code) final override;

		virtual HullShaderRHIRef RHICreateHullShader(const TArray<uint8>& code) final override;

		virtual DomainShaderRHIRef RHICreateDomainShader(const TArray<uint8>& code) final override;

		virtual GeometryShaderRHIRef RHICreateGeometryShader(const TArray<uint8>& code) final override;

		virtual PixelShaderRHIRef RHICreatePixelShader(const TArray<uint8>& code) final override;

		virtual ComputeShaderRHIRef RHICreateComputeShader(const TArray<uint8>& code) final override;

		virtual void RHIClearColorTextures(int32 numTextures, TextureRHIParamRef* textures, const LinearColor* colorArray, IntRect excludeRect) final override;

		virtual void RHIClearDepthStencilTexture(TextureRHIParamRef texture, EClearDepthStencil clearDepthStencil, float depth, uint32 stencil, IntRect& excludeRect) final override;

		virtual void RHIBindClearMRTValues(bool bClearColor, bool bClearDepth, bool bClearStencil) final override;

		virtual GeometryShaderRHIRef RHICreateGeometryShaderWithStreamOutput(const TArray<uint8>& code, const StreamOutElementList& elementList, uint32 numStrides, const uint32* strides, int32 rasterizedStream) final override;

		virtual void* RHILockTexture2D(Texture2DRHIParamRef texture, uint32 mipIndex, EResourceLockMode lockMode, uint32& destStride, bool bLockWithinMiptail) final override;

		virtual void RHIUnlockTexture2D(Texture2DRHIParamRef texture, uint32 mipIndex, bool bLockWithinMiptail) final override;

		virtual BoundShaderStateRHIRef RHICreateBoundShaderState(VertexDeclarationRHIParamRef vertexDeclaration, VertexShaderRHIParamRef vertexShaderRHI, HullShaderRHIParamRef hullShaderRHI, DomainShaderRHIParamRef domainShaderRHI, GeometryShaderRHIParamRef geometryShaderRHI, PixelShaderRHIParamRef pixelShaderRHI) final override;

		virtual void RHISetBoundShaderState(BoundShaderStateRHIParamRef boundShaderState) final override;

		virtual void RHISetShaderConstantBuffer(VertexShaderRHIParamRef vertexShader, uint32 bufferIndex, ConstantBufferRHIParamRef buffer) final override;

		virtual void RHISetShaderConstantBuffer(HullShaderRHIParamRef hullShader, uint32 bufferIndex, ConstantBufferRHIParamRef buffer) final override;

		virtual void RHISetShaderConstantBuffer(DomainShaderRHIParamRef domainShader, uint32 bufferIndex, ConstantBufferRHIParamRef buffer) final override;

		virtual void RHISetShaderConstantBuffer(GeometryShaderRHIParamRef geometryShader, uint32 bufferindex, ConstantBufferRHIParamRef buffer) final override;

		virtual void RHISetShaderConstantBuffer(PixelShaderRHIParamRef pixelShader, uint32 bufferIndex, ConstantBufferRHIParamRef buffer) final override;

		virtual void RHISetShaderConstantBuffer(ComputeShaderRHIParamRef computeShader, uint32 bufferIndex, ConstantBufferRHIParamRef buffer) final override;

		virtual void RHISetShaderParameter(VertexShaderRHIParamRef vertexShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) final override;

		virtual void RHISetShaderParameter(HullShaderRHIParamRef hullShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) final override;

		virtual void RHISetShaderParameter(DomainShaderRHIParamRef hullShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) final override;

		virtual void RHISetShaderParameter(GeometryShaderRHIParamRef hullShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) final override;

		virtual void RHISetShaderParameter(PixelShaderRHIParamRef hullShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) final override;
		static DXGI_FORMAT getPlatformTextureResourceFormat(DXGI_FORMAT inFormat, uint32 inFlags);

		virtual void RHISetShaderParameter(ComputeShaderRHIParamRef hullShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) final override;

		virtual void RHISetShaderTexture(VertexShaderRHIParamRef vertexShader, uint32 textureIndex, TextureRHIParamRef newTexture) final override;

		virtual void RHISetShaderTexture(HullShaderRHIParamRef vertexShader, uint32 textureIndex, TextureRHIParamRef newTexture) final override;

		virtual void RHISetShaderTexture(DomainShaderRHIParamRef vertexShader, uint32 textureIndex, TextureRHIParamRef newTexture) final override;

		virtual void RHISetShaderTexture(GeometryShaderRHIParamRef vertexShader, uint32 textureIndex, TextureRHIParamRef newTexture) final override;

		virtual void RHISetShaderTexture(PixelShaderRHIParamRef vertexShader, uint32 textureIndex, TextureRHIParamRef newTexture) final override;

		virtual void RHISetShaderTexture(ComputeShaderRHIParamRef vertexShader, uint32 textureIndex, TextureRHIParamRef newTexture) final override;

		virtual void RHISetShaderSampler(VertexShaderRHIParamRef vertexShader, uint32 samplerIndex, SamplerStateRHIParamRef newSampler) final override;

		virtual void RHISetShaderSampler(HullShaderRHIParamRef vertexShader, uint32 samplerIndex, SamplerStateRHIParamRef newSampler) final override;

		virtual void RHISetShaderSampler(DomainShaderRHIParamRef vertexShader, uint32 samplerIndex, SamplerStateRHIParamRef newSampler) final override;

		virtual void RHISetShaderSampler(GeometryShaderRHIParamRef vertexShader, uint32 samplerIndex, SamplerStateRHIParamRef newSampler) final override;

		virtual void RHISetShaderSampler(PixelShaderRHIParamRef vertexShader, uint32 samplerIndex, SamplerStateRHIParamRef newSampler) final override;

		virtual void RHISetShaderSampler(ComputeShaderRHIParamRef vertexShader, uint32 samplerIndex, SamplerStateRHIParamRef newSampler) final override;

		virtual void RHISetStreamSource(uint32 streamIndex, VertexBufferRHIParamRef vertexBuffer, uint32 stride, uint32 offset) final override;

		virtual void RHIDrawPrimitive(uint32 primitiveType, int32 baseVertexIndex, uint32 numPrimitives, uint32 numInstances) final override;

		virtual void RHIDrawIndexedPrimitive(IndexBufferRHIParamRef indexBuffer, uint32 primitiveType, int32 baseVertexIndex, uint32 firstInstance, uint32 numVertex, uint32 startIndex, uint32 numPrimitives, uint32 numInstances)final override;

		virtual void RHIEndDrawPrimitiveUP() final override;

		virtual void RHIBeginDrawPrimitiveUP(uint32 primitiveType, uint32 numPrimitives, uint32 numVertices, uint32 vertexDataStride, void*& outVertexData) final override;

		virtual void RHIBeginDrawIndexedPrimitiveUP(uint32 primitiveType, uint32 numPrimitives, uint32 numVertices, uint32 vertexDataStride, void*& outVertexData, uint32 minVertexIndex, uint32 numIndices, uint32 indexDataStride, void*& outIndexData) final override;

		virtual void RHIEndDrawIndexedPrimitiveUP() final override;

		virtual VertexBufferRHIRef RHICreateVertexBuffer(uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo) final override;

		virtual void* RHILockVertexBuffer(VertexBufferRHIParamRef vertexBuffer, uint32 offset, uint32 size, EResourceLockMode lockMode) final override;

		virtual void RHIUnlockVertexBuffer(VertexBufferRHIParamRef vertexBuffer) final override;

		virtual IndexBufferRHIRef RHICreateIndexBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo) final override;

		virtual void* RHILockIndexBuffer(IndexBufferRHIParamRef indexBuffer, uint32 Offset, uint32 size, EResourceLockMode lockMode) final override;

		virtual void RHIUnlockIndexBuffer(IndexBufferRHIParamRef indexBuffer) final override;

		virtual void RHIReadSurfaceFloatData(TextureRHIParamRef texture, IntRect rect, TArray<Float16Color>& outData, ECubeFace cubeface, int32 arrayIndex, int32 mipIndex) final override;

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

		void RHIBindDebugLabelName(TextureRHIParamRef texture, const TCHAR* name) override final;
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

		void conditionalClearShaderResource(D3D11BaseShaderResource* resource);

		template<EShaderFrequency ShaderFrequency>
		void clearShaderResourceViews(D3D11BaseShaderResource* resource);


		void clearAllShaderResources();


		void commitRenderTargetsAndUAVs();

		template<EShaderFrequency ShaderFrequency>
		void clearAllShaderResourcesForFrequency();


		template <EShaderFrequency ShaderFrequency>
		void internalSetShaderResourceView(D3D11BaseShaderResource* resource, ID3D11ShaderResourceView* srv, int32 resourceIndex, wstring name, D3D11StateCacheBase::ESRV_Type srvType = D3D11StateCacheBase::ESRV_Type::SRV_Unknown);

		template<typename ShaderType, EShaderFrequency Frequency>
		void _RHISetShaderTexture(ShaderType* ShaderRHI, uint32 textureIndex, TextureRHIParamRef newTextureRHI);


		enum class EForceFullScreenClear
		{
			EDoNotForce,
			EForce
		};

		virtual void RHIClearMRTImpl(bool bClearColor, int32 numClearColor, const LinearColor* colorArray, bool bClearDepth, float depth, bool bClearStencil, uint32 stencil, IntRect excludeRect, bool bForceShaderClear, EForceFullScreenClear bForceFullScreen);

		void validateExclusiveDepthStencilAccess(FExclusiveDepthStencil src) const;
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

		uint32 mAvailableMSAAQualities[DX_MAX_MSAA_COUNT + 1];

		D3D11BaseShaderResource* mCurrentResourceBoundAsSRVs[SF_NumFrequencies][D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];

		int32 mMaxBoundShaderResourceIndex[SF_NumFrequencies];
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