#pragma once
#include "VulkanGlobals.h"
#include "DynamicRHI.h"
#include "RHIContext.h"
namespace Air
{
	class VulkanDevice;

	class VulkanDynamicRHI : public DynamicRHI
	{
	public:
		virtual void init() final override;

		virtual void shutdown() final override;

		virtual void RHIAcquireThreadOwnership() final override;

		virtual void RHIReleaseThreadOwnership() final override;

		virtual void RHIResizeViewport(RHIViewport* viewport, uint32 sizeX, uint32 sizeY, bool isFullscreen) final override;

		virtual void RHIResizeViewport(RHIViewport* viewport, uint32 sizeX, uint32 sizeY, bool isFullscreen, EPixelFormat preferredPixelFormat)final override;



		virtual ViewportRHIRef RHICreateViewport(void* windowHandle, uint32 sizeX, uint32 sizeY, bool isFullscree, EPixelFormat preferredPixelFormat)final override;

		virtual Texture2DRHIRef RHICreateTexture2D(uint32 width, uint32 height, uint8 format, uint32 numMips, uint32 numSamplers, uint32 flags, RHIResourceCreateInfo& createInfo)final override;

		virtual TextureCubeRHIRef RHICreateTextureCube(uint32 size, uint8 format, uint32 numMips, uint32 flags, RHIResourceCreateInfo& createInfo)final override;



		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView(RHITexture* texture2DRHI, const RHITextureSRVCreateInfo& createInfol)final override;


		virtual ShaderResourceViewRHIRef RHICreateShaderResourceView(RHIVertexBuffer* vertexBuffer, uint32 stride, uint8 format)final override;

		virtual VertexShaderRHIRef RHICreateVertexShader(const TArray<uint8>& code)final override;

		virtual HullShaderRHIRef RHICreateHullShader(const TArray<uint8>& code)final override;

		virtual DomainShaderRHIRef RHICreateDomainShader(const TArray<uint8>& code)final override;

		virtual GeometryShaderRHIRef RHICreateGeometryShader(const TArray<uint8>& code)final override;

		virtual PixelShaderRHIRef RHICreatePixelShader(const TArray<uint8>& code)final override;

		virtual ComputeShaderRHIRef RHICreateComputeShader(const TArray<uint8>& code)final override;

		virtual GeometryShaderRHIRef RHICreateGeometryShaderWithStreamOutput(const TArray<uint8>& code, const StreamOutElementList& elementList, uint32 numStrides, const uint32* strides, int32 rasterizedStream)final override;

		virtual VertexBufferRHIRef RHICreateVertexBuffer(uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo)final override;

		virtual void* RHILockVertexBuffer(RHIVertexBuffer* vertexBuffer, uint32 offset, uint32 size, EResourceLockMode lockMode)final override;

		virtual void RHIUnlockVertexBuffer(RHIVertexBuffer* vertexBuffer)final override;

		virtual IndexBufferRHIRef RHICreateIndexBuffer(uint32 stride, uint32 size, uint32 inUsage, RHIResourceCreateInfo& createInfo)final override;

		virtual void* RHILockIndexBuffer(RHIIndexBuffer* indexBuffer, uint32 Offset, uint32 size, EResourceLockMode lockMode)final override;

		virtual void RHIUnlockIndexBuffer(RHIIndexBuffer* indexBuffer)final override;

		virtual VertexDeclarationRHIRef RHICreateVertexDeclaration(const VertexDeclarationElementList& elements)final override;

		virtual void* RHILockTextureCubeFace(RHITextureCube* texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, EResourceLockMode lockMode, uint32& destStride, bool bLockWithinMiptail)final override;

		virtual void RHIUnlockTextureCubeFace(RHITextureCube* texture, uint32 faceIndex, uint32 arrayIndex, uint32 mipIndex, bool bLockWithinMiptail)final override;

		virtual void updateMSAASettings() final override;

		virtual IRHICommandContext* getDefualtContext()final override;

		virtual TCHAR* getName()final override;

		virtual void RHIAdvanceFrameForGetViewportBackBuffer()final override;

		virtual uint32 RHIComputeMemorySize(RHITexture* textureRHI)final override;

		virtual ConstantBufferRHIRef RHICreateConstantBuffer(const void* contents, const RHIConstantBufferLayout& layout, EConstantBufferUsage usage, EConstantBufferValidation validation = EConstantBufferValidation::ValidateResources)final override;

		virtual SamplerStateRHIRef RHICreateSamplerState(const SamplerStateInitializerRHI& inInitializerRHI)final override;

		virtual RasterizerStateRHIRef RHICreateRasterizerState(const RasterizerStateInitializerRHI& inInitializerRHI)final override;

		virtual DepthStencilStateRHIRef RHICreateDepthStencilState(const DepthStencilStateInitializerRHI& inInitializerRHI)final override;

		virtual BlendStateRHIRef RHICreateBlendState(const BlendStateInitializerRHI& inInitializerRHI)final override;

		virtual BoundShaderStateRHIRef RHICreateBoundShaderState(RHIVertexDeclaration* vertexDeclaration, RHIVertexShader* vertexShaderRHI, RHIHullShader* hullShaderRHI, RHIDomainShader* domainShaderRHI, RHIGeometryShader* geometryShaderRHI, RHIPixelShader* pixelShaderRHI)final override;

		virtual void* RHILockTexture2D(RHITexture2D* texture, uint32 mipIndex, EResourceLockMode lockMode, uint32& destStride, bool bLockWithinMiptail)final override;

		virtual void RHIUnlockTexture2D(RHITexture2D* texture, uint32 mipIndex, bool bLockWithinMiptail)final override;

		virtual TextureReferenceRHIRef RHICreateTextureReference(LastRenderTimeContainer* lastRenderTime)final override;

		virtual void RHIBindDebugLabelName(RHITexture* texture, const TCHAR* name)final override;

		virtual void RHIReadSurfaceFloatData(RHITexture* texture, IntRect rect, TArray<Float16Color>& outData, ECubeFace cubeface, int32 arrayIndex, int32 mipIndex)final override;

		virtual Texture2DRHIRef RHIGetViewportBackBuffer(RHIViewport* viewport)final override;

		inline VulkanDevice* getDevice()
		{
			return mDevice;
		}
	protected:
		void initInstance();

		void createInstance();

		void selectAndInitDevice();

		static void getInstanceLayersAndExtensions(TArray<const ANSICHAR*>& outInstanceExtensions, TArray<const ANSICHAR*>& outInstanceLayers, bool& bOutDebugUtils);
	protected:
		bool bSupportsDebugUtilsExt = false;

		bool bSupportsDebugCallbackExt = false;

		VulkanDevice* mDevice;

		VkInstance mInstance;

		TArray<const ANSICHAR*> mInstanceExtension;
		TArray<const ANSICHAR*> mInstanceLayers;

	};






	class VulkanDynamicRHIModule : public IDynamicRHIModule
	{
	public:
		virtual bool isSupported() override;

		virtual DynamicRHI* createRHI(ERHIFeatureLevel::Type requestedFeatureLevel = ERHIFeatureLevel::Num) override;
	};
}