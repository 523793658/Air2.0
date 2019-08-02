#pragma once
#include "CoreMinimal.h"
#include "RHIConfig.h"
#include "RHIDefinitions.h"
#include "RHIResource.h"
#include "Modules/ModuleInterface.h"
namespace Air
{
	class IRHIComputeContext
	{
	public:
		virtual void RHISetShaderTexture(ComputeShaderRHIParamRef computeShader, uint32 textureIndex, TextureRHIParamRef newTexture) = 0;

		virtual void RHISetShaderSampler(ComputeShaderRHIParamRef computeShader, uint32 samplerIndex, SamplerStateRHIParamRef newState) = 0;

		virtual void RHISetShaderParameter(ComputeShaderRHIParamRef computeShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newData) = 0;

		virtual void RHISetScissorRect(bool bEnable, uint32 minX, uint32 minY, uint32 maxX, uint32 maxY) = 0;
	};

	class IRHICommandContext : public IRHIComputeContext
	{
	public:
		virtual void RHIBeginDrawingViewport(ViewportRHIParamRef viewport, TextureRHIParamRef renderTargetRHI) = 0;
		virtual void RHIEndDrawingViewport(ViewportRHIParamRef viewport, bool bPresent, bool bLockToVsync) = 0;

		virtual void RHIBeginFrame() = 0;

		virtual void RHIEndFrame() = 0;

		virtual void RHIBeginScene() = 0;

		virtual void RHIEndScene() = 0;

		virtual void RHICopyToResolveTarget(TextureRHIParamRef sourceTexture, TextureRHIParamRef destTexture, bool keepOriginalSurface, const ResolveParams& resolveParams) = 0;

		virtual void RHISetRenderTargets(uint32 NumSimultaneousRenderTargets, const RHIRenderTargetView* newRenderTargets, const RHIDepthRenderTargetView* newDepthStencilTarget, uint32 numUAVs, const UnorderedAccessViewRHIParamRef* UAVs) = 0;

		virtual void RHISetBlendState(BlendStateRHIParamRef newState, const LinearColor& blendFactory) = 0;

		virtual void RHISetDepthStencilState(DepthStencilStateRHIParamRef newState, uint32 inStencilRef) = 0;

		virtual void RHISetRasterizerState(RasterizerStateRHIParamRef newState) = 0;

		virtual void RHISetStencilRef(uint32 stencilRef) {}

		virtual void RHISetViewport(uint32 x, uint32 y, float z, uint32 width, uint32 height, float depth) = 0;

		virtual void RHISetMultipleViewports(uint32 count, const ViewportBounds* data) = 0;

		virtual void RHISetRenderTargetsAndClear(const RHISetRenderTargetsInfo& renderTargetsInfo) = 0;

		virtual void RHIClearColorTextures(int32 numTextures, TextureRHIParamRef* textures, const LinearColor* colorArray, IntRect excludeRect) = 0;

		virtual void RHIClearDepthStencilTexture(TextureRHIParamRef texture, EClearDepthStencil clearDepthStencil, float depth, uint32 stencil, IntRect& excludeRect) = 0;

		virtual void RHIBindClearMRTValues(bool bClearColor, bool bClearDepth, bool bClearStencil) = 0;

		virtual void RHISetBoundShaderState(BoundShaderStateRHIParamRef boundShaderState) = 0;

		virtual void RHIBeginDrawIndexedPrimitiveUP(uint32 primitiveType, uint32 numPrimitives, uint32 numVertices, uint32 vertexDataStride, void*& outVertexData, uint32 minVertexIndex, uint32 numIndices, uint32 indexDataStride, void*& outIndexData) = 0;

		virtual void RHIEndDrawIndexedPrimitiveUP() = 0;

		virtual void RHIBeginDrawPrimitiveUP(uint32 primitiveType, uint32 numPrimitives, uint32 numVertices, uint32 vertexDataStride, void*& outVertexData) = 0;

		virtual void RHIEndDrawPrimitiveUP() = 0;

		virtual void RHIDrawIndexedPrimitive(IndexBufferRHIParamRef indexBuffer, uint32 primitiveType, int32 baseVertexIndex, uint32 firstInstance, uint32 numVertex, uint32 startIndex, uint32 numPrimitives, uint32 numInstances) = 0;

		virtual void RHIDrawPrimitive(uint32 primitiveType, int32 baseVertexIndex, uint32 numPrimitives, uint32 numInstances) = 0;

		virtual void RHISetShaderTexture(VertexShaderRHIParamRef vertexShader, uint32 textureIndex, TextureRHIParamRef newTexture) = 0;

		virtual void RHISetShaderTexture(HullShaderRHIParamRef hullShader, uint32 textureIndex, TextureRHIParamRef newTexture) = 0;

		virtual void RHISetShaderTexture(DomainShaderRHIParamRef domainShader, uint32 textureIndex, TextureRHIParamRef newTexture) = 0;

		virtual void RHISetShaderTexture(GeometryShaderRHIParamRef geometryShader, uint32 textureIndex, TextureRHIParamRef newTexture) = 0;

		virtual void RHISetShaderTexture(PixelShaderRHIParamRef pixelShader, uint32 textureIndex, TextureRHIParamRef newTexture) = 0;

		virtual void RHISetShaderTexture(ComputeShaderRHIParamRef computeShader, uint32 textureIndex, TextureRHIParamRef newTexture) = 0;



		virtual void RHISetShaderSampler(VertexShaderRHIParamRef vertexShader, uint32 samplerIndex, SamplerStateRHIParamRef newSampler) = 0;

		virtual void RHISetShaderSampler(HullShaderRHIParamRef hullShader, uint32 textureIndex, SamplerStateRHIParamRef newSampler) = 0;

		virtual void RHISetShaderSampler(DomainShaderRHIParamRef domainShader, uint32 samplerIndex, SamplerStateRHIParamRef newSampler) = 0;

		virtual void RHISetShaderSampler(GeometryShaderRHIParamRef geometryShader, uint32 samplerIndex, SamplerStateRHIParamRef newSampler) = 0;

		virtual void RHISetShaderSampler(PixelShaderRHIParamRef pixelShader, uint32 samplerIndex, SamplerStateRHIParamRef newSampler) = 0;

		virtual void RHISetShaderSampler(ComputeShaderRHIParamRef computeShader, uint32 samplerIndex, SamplerStateRHIParamRef newSampler) = 0;

		virtual void RHISetStreamSource(uint32 streamIndex, VertexBufferRHIParamRef vertexBuffer, uint32 stride, uint32 offset) = 0;

		virtual void RHITransitionResources(EResourceTransitionAccess transitionType, EResourceTransitionPipeline transitionPipeline, UnorderedAccessViewRHIParamRef* InUAVs, int32 numUAVs, ComputeFenceRHIParamRef writeComputeFence)
		{
			if (writeComputeFence)
			{
				writeComputeFence->writeFence();
			}
		}

		void RHITransitionResources(EResourceTransitionAccess transitionAccess, EResourceTransitionPipeline transitionPipeline, UnorderedAccessViewRHIParamRef* inUAVs, int32 numUAVs)
		{
			RHITransitionResources(transitionAccess, transitionPipeline, inUAVs, numUAVs, nullptr);
		}


		virtual void RHITransitionResources(EResourceTransitionAccess transitionType, TextureRHIParamRef* inTexture, int32 numTextures)
		{
			if (transitionType == EResourceTransitionAccess::EReadable)
			{
				const ResolveParams resolveParams;
				for (int32 i = 0; i < numTextures; ++i)
				{
					RHICopyToResolveTarget(inTexture[i], inTexture[i], true, resolveParams);
				}
			}
		}

		virtual void RHISetShaderConstantBuffer(VertexShaderRHIParamRef vertexShader, uint32 bufferIndex, ConstantBufferRHIParamRef buffer) = 0;

		virtual void RHISetShaderConstantBuffer(HullShaderRHIParamRef hullShader, uint32 bufferIndex, ConstantBufferRHIParamRef buffer) = 0;

		virtual void RHISetShaderConstantBuffer(DomainShaderRHIParamRef domainShader, uint32 bufferIndex, ConstantBufferRHIParamRef buffer) = 0;

		virtual void RHISetShaderConstantBuffer(GeometryShaderRHIParamRef geometryShader, uint32 bufferIndex, ConstantBufferRHIParamRef buffer) = 0;

		virtual void RHISetShaderConstantBuffer(PixelShaderRHIParamRef pixelShader, uint32 bufferIndex, ConstantBufferRHIParamRef buffer) = 0;

		virtual void RHISetShaderConstantBuffer(ComputeShaderRHIParamRef computeShader, uint32 bufferIndex, ConstantBufferRHIParamRef buffer) = 0;

		virtual void RHISetShaderParameter(VertexShaderRHIParamRef vertexShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) = 0;

		virtual void RHISetShaderParameter(HullShaderRHIParamRef hullShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) = 0;

		virtual void RHISetShaderParameter(DomainShaderRHIParamRef domainShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) = 0;

		virtual void RHISetShaderParameter(GeometryShaderRHIParamRef geometryShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) = 0;

		virtual void RHISetShaderParameter(PixelShaderRHIParamRef pixelShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) = 0;

		virtual void RHISetShaderParameter(ComputeShaderRHIParamRef computeShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) = 0;

		virtual void RHIUpdateTextureReference(TextureReferenceRHIParamRef textureRHI, TextureRHIParamRef newTexture) = 0;
	};


	class RHI_API DynamicRHI
	{
	public:
		virtual ~DynamicRHI() {};

		virtual void init() = 0;
	
		virtual void postInit() {};

		virtual void shutdown() = 0;

		virtual void RHIAcquireThreadOwnership() = 0;

		virtual void RHIReleaseThreadOwnership() = 0;

		virtual void RHISetViewport(uint32 MinX, uint32 MinY, float MinZ, uint32 maxX, uint32 maxY, float maxZ) = 0;

		virtual void RHIResizeViewport(ViewportRHIParamRef viewport, uint32 sizeX, uint32 sizeY, bool isFullscreen, EPixelFormat preferredPixelFormat) = 0;

	

		virtual ViewportRHIParamRef RHICreateViewport(void* windowHandle, uint32 sizeX, uint32 sizeY, bool isFullscree, EPixelFormat preferredPixelFormat) = 0;

		virtual Texture2DRHIRef RHICreateTexture2D(uint32 width, uint32 height, uint8 format, uint32 numMips, uint32 numSamplers, uint32 flags, RHIResourceCreateInfo& createInfo) = 0;

		virtual TextureCubeRHIRef RHICreateTextureCube(uint32 size, uint8 format, uint32 numMips, uint32 flags, RHIResourceCreateInfo& createInfo) = 0;



		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView(Texture2DRHIParamRef texture2DRHI, uint32 mipLevel) = 0;

		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView(Texture2DRHIParamRef texture2DRHI, uint32 mipLevel, uint8 numMipLevel, uint8 format) = 0;

		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView(VertexBufferRHIParamRef vertexBuffer, uint32 stride, uint8 format) = 0;

		virtual VertexShaderRHIRef RHICreateVertexShader(const TArray<uint8>& code) = 0;

		virtual HullShaderRHIRef RHICreateHullShader(const TArray<uint8>& code) = 0;

		virtual DomainShaderRHIRef RHICreateDomainShader(const TArray<uint8>& code) = 0;

		virtual GeometryShaderRHIRef RHICreateGeometryShader(const TArray<uint8>& code) = 0;

		virtual PixelShaderRHIRef RHICreatePixelShader(const TArray<uint8>& code) = 0;

		virtual ComputeShaderRHIRef RHICreateComputeShader(const TArray<uint8>& code) = 0;

		virtual GeometryShaderRHIRef RHICreateGeometryShaderWithStreamOutput(const TArray<uint8>& code, const StreamOutElementList& elementList, uint32 numStrides, const uint32* strides, int32 rasterizedStream) = 0;

		virtual VertexBufferRHIRef RHICreateVertexBuffer(uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo) = 0;

		virtual void* RHILockVertexBuffer(VertexBufferRHIParamRef vertexBuffer, uint32 offset, uint32 size, EResourceLockMode lockMode) = 0;

		virtual void RHIUnlockVertexBuffer(VertexBufferRHIParamRef vertexBuffer) = 0;

		virtual IndexBufferRHIRef RHICreateIndexBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo) = 0;

		virtual void* RHILockIndexBuffer(IndexBufferRHIParamRef indexBuffer, uint32 Offset, uint32 size, EResourceLockMode lockMode) = 0;

		virtual void RHIUnlockIndexBuffer(IndexBufferRHIParamRef indexBuffer) = 0;

		virtual VertexDeclarationRHIRef RHICreateVertexDeclaration(const VertexDeclarationElementList& elements) = 0;

		virtual void* RHILockTextureCubeFace(TextureCubeRHIParamRef texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, EResourceLockMode lockMode, uint32& destStride, bool bLockWithinMiptail) = 0;

		virtual void RHIUnlockTextureCubeFace(TextureCubeRHIParamRef texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, bool bLockWithinMiptail) = 0;
		


		virtual Texture2DRHIRef RHICreateTexture2D_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 width, uint32 height, uint8 format, uint32 numMips, uint32 numSamples, uint32 flags, RHIResourceCreateInfo& createInfo);

		virtual TextureCubeRHIRef RHICreateTextureCube_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 size, uint8 format, uint32 numMips, uint32 flags, RHIResourceCreateInfo& createInfo);

		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView_RenderThread(class RHICommandListImmediate& RHICmdList, Texture2DRHIParamRef Texture2DRHI, uint8 mipLevel);

		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView_RenderThread(class RHICommandListImmediate& RHICmdList, Texture2DRHIParamRef Texture2DRHI, uint8 mipLevel, uint8 numMipLevel, uint8 format);

		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView_RenderThread(class RHICommandListImmediate& RHICmdList, VertexBufferRHIParamRef vertexBuffer, uint32 stride, uint8 format);

		virtual VertexBufferRHIRef createAndLockVertexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo, void*& outPtr);

		

		virtual VertexBufferRHIRef createVertexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo);

		virtual void* lockVertexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, VertexBufferRHIParamRef vertexBuffer, uint32 offset, uint32 sizeRHI, EResourceLockMode lockMode);

		virtual void unlockVertexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, VertexBufferRHIParamRef vertexBuffer);

		virtual IndexBufferRHIRef createAndLockIndexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo, void*& outDataBuffer);

		virtual IndexBufferRHIRef createIndexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo);

		virtual void* lockIndexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, IndexBufferRHIParamRef indexBuffer, uint32 offset, uint32 sizeRHI, EResourceLockMode lockMode);

		virtual void unlockIndexBuffer_RenderThread(class RHICommandListImmediate& RHICmdList, IndexBufferRHIParamRef indexBuffer);

		virtual VertexDeclarationRHIRef createVertexDeclaration_RenderThread(class RHICommandListImmediate& RHICmdList, const VertexDeclarationElementList& elements);


		virtual void updateMSAASettings() = 0;

		virtual void enableHDR() {}

		virtual void shutdownHDR(){}

		virtual Texture2DRHIRef RHIGetViewportBackBuffer(ViewportRHIParamRef viewport) = 0;

		void enableIdealGPUCaptureOptions(bool enable);

		virtual IRHICommandContext* getDefualtContext() = 0;

		virtual IRHIComputeContext* getDefualtAsynicComputeContext()
		{
			IRHIComputeContext* computeContext = getDefualtContext();
			return computeContext;
		}

		virtual TCHAR* getName() = 0;

		virtual void RHIAdvanceFrameForGetViewportBackBuffer() = 0;

		virtual uint32 RHIComputeMemorySize(TextureRHIParamRef textureRHI) = 0;

		virtual ConstantBufferRHIRef RHICreateConstantBuffer(const void* contents, const RHIConstantBufferLayout& layout, EConstantBufferUsage usage) = 0;

		virtual SamplerStateRHIRef RHICreateSamplerState(const SamplerStateInitializerRHI &inInitializerRHI) = 0;

		virtual RasterizerStateRHIRef RHICreateRasterizerState(const RasterizerStateInitializerRHI &inInitializerRHI) = 0;

		virtual DepthStencilStateRHIRef RHICreateDepthStencilState(const DepthStencilStateInitializerRHI & inInitializerRHI) = 0;

		virtual BlendStateRHIRef RHICreateBlendState(const BlendStateInitializerRHI& inInitializerRHI) = 0;

		virtual BoundShaderStateRHIRef RHICreateBoundShaderState(VertexDeclarationRHIParamRef vertexDeclaration, VertexShaderRHIParamRef vertexShaderRHI, HullShaderRHIParamRef hullShaderRHI, DomainShaderRHIParamRef domainShaderRHI, GeometryShaderRHIParamRef geometryShaderRHI, PixelShaderRHIParamRef pixelShaderRHI) = 0;

		virtual void* RHILockTexture2D(Texture2DRHIParamRef texture, uint32 mipIndex, EResourceLockMode lockMode, uint32 & destStride, bool bLockWithinMiptail) = 0;

		virtual void RHIUnlockTexture2D(Texture2DRHIParamRef texture, uint32 mipIndex, bool bLockWithinMiptail) = 0;

		virtual TextureReferenceRHIRef RHICreateTextureReference(LastRenderTimeContainer* lastRenderTime) = 0;

		virtual void* lockTexture2D_RenderThread(class RHICommandListImmediate& rhiCmdList, Texture2DRHIParamRef texture, uint32 mipIndex, EResourceLockMode lockMode, uint32& destStride, bool bLockWithinMiptail, bool bNeedsDefaultRHIFlush);
		
		virtual void unlockTexture2D_RenderThread(class RHICommandListImmediate& rhiCmdList, Texture2DRHIParamRef texture, uint32 mipIndex, bool bLockWithMiptail, bool bFlushRHIThread = true);

		virtual void RHIBindDebugLabelName(TextureRHIParamRef texture, const TCHAR* name) = 0;

		virtual void RHIReadSurfaceFloatData(TextureRHIParamRef texture, IntRect rect, TArray<Float16Color>& outData, ECubeFace cubeface, int32 arrayIndex, int32 mipIndex) = 0;

		virtual void RHIBindDebugLabelName(UnorderedAccessViewRHIParamRef unorderedAccessViewRHI, const TCHAR* name) {}


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

	FORCEINLINE Texture2DRHIRef RHIGetViewportBackBuffer(ViewportRHIParamRef viewport)
	{
		return GDynamicRHI->RHIGetViewportBackBuffer(viewport);
	}

	FORCEINLINE class IRHIComputeContext* RHIGetDefualtAsyncComputeContext()
	{
		return GDynamicRHI->getDefualtAsynicComputeContext();
	}

	FORCEINLINE void RHIResizeViewpor(ViewportRHIParamRef viewport, uint32 sizeX, uint32 sizeY, bool bIsFullscreen, EPixelFormat preferredPixelFormat)
	{
		GDynamicRHI->RHIResizeViewport(viewport, sizeX, sizeY, bIsFullscreen, preferredPixelFormat);
	}

	FORCEINLINE ViewportRHIParamRef RHICreateViewport(void* windowHandle, uint32 sizeX, uint32 sizeY, bool isFullscree, EPixelFormat preferredPixelFormat)
	{
		return GDynamicRHI->RHICreateViewport(windowHandle, sizeX, sizeY, isFullscree, preferredPixelFormat);
	}

	FORCEINLINE void RHIAdvanceFrameForGetViewportBackBuffer()
	{
		return GDynamicRHI->RHIAdvanceFrameForGetViewportBackBuffer();
	}

	FORCEINLINE uint32 RHIComputeMemorySize(TextureRHIParamRef textureRHI)
	{
		return GDynamicRHI->RHIComputeMemorySize(textureRHI);
	}

	FORCEINLINE ConstantBufferRHIRef RHICreateConstantBuffer(const void* contents, const RHIConstantBufferLayout& layout, EConstantBufferUsage usage)
	{
		return GDynamicRHI->RHICreateConstantBuffer(contents, layout, usage);
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

	FORCEINLINE BoundShaderStateRHIRef RHICreateBoundShaderState(VertexDeclarationRHIParamRef vertexDeclaration, VertexShaderRHIParamRef vertexShaderRHI, HullShaderRHIParamRef hullShaderRHI, DomainShaderRHIParamRef domainShaderRHI, GeometryShaderRHIParamRef geometryShaderRHI, PixelShaderRHIParamRef pixelShaderRHI)
	{
		return GDynamicRHI->RHICreateBoundShaderState(vertexDeclaration, vertexShaderRHI, hullShaderRHI, domainShaderRHI, geometryShaderRHI, pixelShaderRHI);
	}

	FORCEINLINE void RHIBindDebugLabelName(TextureRHIParamRef texture, const TCHAR* name)
	{
		GDynamicRHI->RHIBindDebugLabelName(texture, name);
	}
}