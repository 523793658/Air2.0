#pragma once
#include "CoreMinimal.h"
#include "RHIConfig.h"
#include "RHIDefinitions.h"
#include "RHIResource.h"
#include "RHIContext.h"
#include "Modules/ModuleInterface.h"
namespace Air
{


	


	class RHI_API DynamicRHI
	{
	public:
		virtual ~DynamicRHI() {};

		virtual void init() = 0;
	
		virtual void postInit() {};

		virtual void shutdown() = 0;

		virtual void RHIAcquireThreadOwnership() = 0;

		virtual void RHIReleaseThreadOwnership() = 0;



		virtual void RHIResizeViewport(RHIViewport* viewport, uint32 sizeX, uint32 sizeY, bool isFullscreen, EPixelFormat preferredPixelFormat) = 0;

		virtual void RHIResizeViewport(RHIViewport* viewport, uint32 sizeX, uint32 sizeY, bool isFullscreen) = 0;


		virtual ViewportRHIRef RHICreateViewport(void* windowHandle, uint32 sizeX, uint32 sizeY, bool isFullscree, EPixelFormat preferredPixelFormat) = 0;

		virtual Texture2DRHIRef RHICreateTexture2D(uint32 width, uint32 height, uint8 format, uint32 numMips, uint32 numSamplers, uint32 flags, RHIResourceCreateInfo& createInfo) = 0;

		virtual TextureCubeRHIRef RHICreateTextureCube(uint32 size, uint8 format, uint32 numMips, uint32 flags, RHIResourceCreateInfo& createInfo) = 0;



		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView(RHITexture* textureRHI, const RHITextureSRVCreateInfo& createInfo) = 0;


		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView(RHIVertexBuffer* vertexBuffer, uint32 stride, uint8 format) = 0;

		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView(RHIStructuredBuffer* structuredBuffer) = 0;



		virtual UnorderedAccessViewRHIRef RHICreateUnorderedAccessView(RHIStructuredBuffer* structuredBuffer, bool bUseUAVCounter, bool bAppendBuffer) = 0;

		virtual UnorderedAccessViewRHIRef RHICreateUnorderedAccessView(RHITexture* texture, uint32 mipLevel) = 0;

		virtual UnorderedAccessViewRHIRef RHICreateUnorderedAccessView(RHIVertexBuffer* vertexBuffer, uint8 format) = 0;

		virtual UnorderedAccessViewRHIRef RHICreateUnorderedAccessView(RHIIndexBuffer* indexBuffer, uint8 format) = 0;

		virtual void RHIDiscardTransientResource_RenderThread(RHIStructuredBuffer* buffer) {}

		virtual void RHIDiscardTransientResource_RenderThread(RHITexture* texture) {}

		virtual void RHIDiscardTransientResource_RenderThread(RHIVertexBuffer* buffer) {}

		virtual void RHIAcquireTransientResource_RenderThread(RHIStructuredBuffer* buffer) {}

		virtual void RHIAcquireTransientResource_RenderThread(RHIVertexBuffer* buffer) {}

		virtual void RHIAcquireTransientResource_RenderThread(RHITexture* texture){}

		virtual VertexShaderRHIRef RHICreateVertexShader(const TArray<uint8>& code) = 0;

		virtual HullShaderRHIRef RHICreateHullShader(const TArray<uint8>& code) = 0;

		virtual DomainShaderRHIRef RHICreateDomainShader(const TArray<uint8>& code) = 0;

		virtual GeometryShaderRHIRef RHICreateGeometryShader(const TArray<uint8>& code) = 0;

		virtual PixelShaderRHIRef RHICreatePixelShader(const TArray<uint8>& code) = 0;

		virtual ComputeShaderRHIRef RHICreateComputeShader(const TArray<uint8>& code) = 0;

		virtual ComputeFenceRHIRef RHICreateComputeFence(const wstring& name)
		{
			return new RHIComputeFence(name);
		}

		

		virtual GeometryShaderRHIRef RHICreateGeometryShaderWithStreamOutput(const TArray<uint8>& code, const StreamOutElementList& elementList, uint32 numStrides, const uint32* strides, int32 rasterizedStream) = 0;

		virtual VertexBufferRHIRef RHICreateVertexBuffer(uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo) = 0;

		virtual void* RHILockVertexBuffer(RHIVertexBuffer* vertexBuffer, uint32 offset, uint32 size, EResourceLockMode lockMode) = 0;

		virtual void RHIUnlockVertexBuffer(RHIVertexBuffer* vertexBuffer) = 0;

		virtual IndexBufferRHIRef RHICreateIndexBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo) = 0;

		virtual void* RHILockIndexBuffer(RHIIndexBuffer* indexBuffer, uint32 Offset, uint32 size, EResourceLockMode lockMode) = 0;

		virtual void RHIUnlockIndexBuffer(RHIIndexBuffer* indexBuffer) = 0;

		virtual VertexDeclarationRHIRef RHICreateVertexDeclaration(const VertexDeclarationElementList& elements) = 0;

		virtual void* RHILockTextureCubeFace(RHITextureCube* texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, EResourceLockMode lockMode, uint32& destStride, bool bLockWithinMiptail) = 0;

		virtual void RHIUnlockTextureCubeFace(RHITextureCube* texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, bool bLockWithinMiptail) = 0;
		
		virtual StructuredBufferRHIRef RHICreateStructuredBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo) = 0;


		virtual Texture2DRHIRef RHICreateTexture2D_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 width, uint32 height, uint8 format, uint32 numMips, uint32 numSamples, uint32 flags, RHIResourceCreateInfo& createInfo);

		virtual TextureCubeRHIRef RHICreateTextureCube_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 size, uint8 format, uint32 numMips, uint32 flags, RHIResourceCreateInfo& createInfo);

		virtual StructuredBufferRHIRef RHICreateStructuredBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo);

		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView_RenderThread(class RHICommandListImmediate& RHICmdList, RHITexture* texture, const RHITextureSRVCreateInfo& createInfo);


		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView_RenderThread(class RHICommandListImmediate& RHICmdList, RHIVertexBuffer* vertexBuffer, uint32 stride, uint8 format);

		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView_RenderThread(class RHICommandListImmediate& RHICmdList, RHIStructuredBuffer* structuredBuffer);

		virtual UnorderedAccessViewRHIRef RHICreateUnorderredAccessView_RenderThread(class RHICommandListImmediate& RHICmdList, RHIStructuredBuffer* structuredBuffer, bool bUseUAVCounter, bool bAppendBuffer);

		virtual UnorderedAccessViewRHIRef RHICreateUnorderredAccessView_RenderThread(class RHICommandListImmediate& RHICmdList, RHITexture* texture, uint32 mipLevel);

		virtual UnorderedAccessViewRHIRef RHICreateUnorderredAccessView_RenderThread(class RHICommandListImmediate& RHICmdList, RHIVertexBuffer* vertexBuffer, uint8 format);

		virtual UnorderedAccessViewRHIRef RHICreateUnorderredAccessView_RenderThread(class RHICommandListImmediate& RHICmdList, RHIIndexBuffer* indexBuffer, uint8 format);

		virtual VertexBufferRHIRef createAndLockVertexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo, void*& outPtr);

		

		virtual VertexBufferRHIRef createVertexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo);

		virtual void* lockVertexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, RHIVertexBuffer* vertexBuffer, uint32 offset, uint32 sizeRHI, EResourceLockMode lockMode);

		virtual void unlockVertexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, RHIVertexBuffer* vertexBuffer);

		virtual IndexBufferRHIRef createAndLockIndexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo, void*& outDataBuffer);

		virtual IndexBufferRHIRef createIndexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo);

		virtual void* lockIndexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, RHIIndexBuffer* indexBuffer, uint32 offset, uint32 sizeRHI, EResourceLockMode lockMode);

		virtual void unlockIndexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, RHIIndexBuffer* indexBuffer);

		virtual VertexDeclarationRHIRef createVertexDeclaration_RenderThread(class RHICommandListImmediate& RHICmdList, const VertexDeclarationElementList& elements);


		virtual void updateMSAASettings() = 0;

		virtual void enableHDR() {}

		virtual void shutdownHDR(){}

		virtual Texture2DRHIRef RHIGetMaskTexture(RHITexture* sourceTextureRHI)
		{
			return nullptr;
		}

		virtual Texture2DRHIRef RHIGetViewportBackBuffer(RHIViewport* viewport) = 0;

		void enableIdealGPUCaptureOptions(bool enable);

		virtual IRHICommandContext* getDefualtContext() = 0;

		virtual IRHIComputeContext* getDefualtAsynicComputeContext()
		{
			IRHIComputeContext* computeContext = getDefualtContext();
			return computeContext;
		}

		virtual TCHAR* getName() = 0;

		virtual void RHIAdvanceFrameForGetViewportBackBuffer() = 0;

		virtual uint32 RHIComputeMemorySize(RHITexture* textureRHI) = 0;

		virtual ConstantBufferRHIRef RHICreateConstantBuffer(const void* contents, const RHIConstantBufferLayout& layout, EConstantBufferUsage usage, EConstantBufferValidation validation) = 0;

		

		virtual void RHIUpdateConstantBuffer(RHIConstantBuffer* constantBufferRHI, const void* contents) = 0;

		virtual SamplerStateRHIRef RHICreateSamplerState(const SamplerStateInitializerRHI &inInitializerRHI) = 0;

		virtual RasterizerStateRHIRef RHICreateRasterizerState(const RasterizerStateInitializerRHI &inInitializerRHI) = 0;

		virtual DepthStencilStateRHIRef RHICreateDepthStencilState(const DepthStencilStateInitializerRHI & inInitializerRHI) = 0;

		virtual BlendStateRHIRef RHICreateBlendState(const BlendStateInitializerRHI& inInitializerRHI) = 0;

		virtual BoundShaderStateRHIRef RHICreateBoundShaderState(RHIVertexDeclaration* vertexDeclaration, RHIVertexShader* vertexShaderRHI, RHIHullShader* hullShaderRHI, RHIDomainShader* domainShaderRHI, RHIGeometryShader* geometryShaderRHI, RHIPixelShader* pixelShaderRHI) = 0;

		virtual void* RHILockTexture2D(RHITexture2D* texture, uint32 mipIndex, EResourceLockMode lockMode, uint32 & destStride, bool bLockWithinMiptail) = 0;

		virtual void RHIUnlockTexture2D(RHITexture2D* texture, uint32 mipIndex, bool bLockWithinMiptail) = 0;

		virtual TextureReferenceRHIRef RHICreateTextureReference(LastRenderTimeContainer* lastRenderTime) = 0;

		virtual void* lockTexture2D_RenderThread(class RHICommandListImmediate& rhiCmdList, RHITexture2D* texture, uint32 mipIndex, EResourceLockMode lockMode, uint32& destStride, bool bLockWithinMiptail, bool bNeedsDefaultRHIFlush);
		
		virtual void unlockTexture2D_RenderThread(class RHICommandListImmediate& rhiCmdList, RHITexture2D* texture, uint32 mipIndex, bool bLockWithMiptail, bool bFlushRHIThread = true);

		virtual void RHIBindDebugLabelName(RHITexture* texture, const TCHAR* name) = 0;

		virtual void RHIReadSurfaceFloatData(RHITexture* texture, IntRect rect, TArray<Float16Color>& outData, ECubeFace cubeface, int32 arrayIndex, int32 mipIndex) = 0;

		virtual void RHIBindDebugLabelName(RHIUnorderedAccessView* unorderedAccessViewRHI, const TCHAR* name) {}


	};


	class IDynamicRHIModule : public IModuleInterface
	{
	public:

		virtual bool isSupported() = 0;

		virtual DynamicRHI* createRHI(ERHIFeatureLevel::Type requestedFeatureLevel = ERHIFeatureLevel::Num) = 0;

		

	};

	DynamicRHI* platformCreateDynamicRHI();


	extern RHI_API DynamicRHI* GDynamicRHI;


	FORCEINLINE class IRHICommandContext* RHIGetDefualtContext()
	{
		return GDynamicRHI->getDefualtContext();
	}

	FORCEINLINE Texture2DRHIRef RHIGetViewportBackBuffer(RHIViewport* viewport)
	{
		return GDynamicRHI->RHIGetViewportBackBuffer(viewport);
	}

	FORCEINLINE class IRHIComputeContext* RHIGetDefualtAsyncComputeContext()
	{
		return GDynamicRHI->getDefualtAsynicComputeContext();
	}

	FORCEINLINE void RHIResizeViewpor(RHIViewport* viewport, uint32 sizeX, uint32 sizeY, bool bIsFullscreen, EPixelFormat preferredPixelFormat)
	{
		GDynamicRHI->RHIResizeViewport(viewport, sizeX, sizeY, bIsFullscreen, preferredPixelFormat);
	}

	FORCEINLINE RHIViewport* RHICreateViewport(void* windowHandle, uint32 sizeX, uint32 sizeY, bool isFullscree, EPixelFormat preferredPixelFormat)
	{
		return GDynamicRHI->RHICreateViewport(windowHandle, sizeX, sizeY, isFullscree, preferredPixelFormat);
	}

	FORCEINLINE void RHIAdvanceFrameForGetViewportBackBuffer()
	{
		return GDynamicRHI->RHIAdvanceFrameForGetViewportBackBuffer();
	}

	FORCEINLINE uint32 RHIComputeMemorySize(RHITexture* textureRHI)
	{
		return GDynamicRHI->RHIComputeMemorySize(textureRHI);
	}

	FORCEINLINE ConstantBufferRHIRef RHICreateConstantBuffer(const void* contents, const RHIConstantBufferLayout& layout, EConstantBufferUsage usage, EConstantBufferValidation validation = EConstantBufferValidation::ValidateResources)
	{
		return GDynamicRHI->RHICreateConstantBuffer(contents, layout, usage, validation);
	}

	FORCEINLINE SamplerStateRHIRef RHICreateSamplerState(const SamplerStateInitializerRHI &inInitializerRHI)
	{
		return GDynamicRHI->RHICreateSamplerState(inInitializerRHI);
	}

	FORCEINLINE RasterizerStateRHIRef RHICreateRasterizerState(const RasterizerStateInitializerRHI& inInitializerRHI)
	{
		return GDynamicRHI->RHICreateRasterizerState(inInitializerRHI);
	}

	FORCEINLINE DepthStencilStateRHIRef RHICreateDepthStencilState(const DepthStencilStateInitializerRHI& inInitializerRHI)
	{
		return GDynamicRHI->RHICreateDepthStencilState(inInitializerRHI);
	}

	FORCEINLINE BlendStateRHIRef RHICreateBlendState(const BlendStateInitializerRHI& inInitializerRHI)
	{
		return GDynamicRHI->RHICreateBlendState(inInitializerRHI);
	}

	FORCEINLINE BoundShaderStateRHIRef RHICreateBoundShaderState(RHIVertexDeclaration* vertexDeclaration, RHIVertexShader* vertexShaderRHI, RHIHullShader* hullShaderRHI, RHIDomainShader* domainShaderRHI, RHIGeometryShader* geometryShaderRHI, RHIPixelShader* pixelShaderRHI)
	{
		return GDynamicRHI->RHICreateBoundShaderState(vertexDeclaration, vertexShaderRHI, hullShaderRHI, domainShaderRHI, geometryShaderRHI, pixelShaderRHI);
	}

	FORCEINLINE void RHIBindDebugLabelName(RHITexture* texture, const TCHAR* name)
	{
		GDynamicRHI->RHIBindDebugLabelName(texture, name);
	}

	FORCEINLINE void RHIUpdateConstantBuffer(RHIConstantBuffer* constantBufferRHI, const void* contents)
	{
		return GDynamicRHI->RHIUpdateConstantBuffer(constantBufferRHI, contents);
	}

	
}