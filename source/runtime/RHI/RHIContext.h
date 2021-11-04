#pragma once
#include "CoreMinimal.h"
#include "Containers/ArrayView.h"
#include "RHIResource.h"
namespace Air
{
	class RHIVertexDeclaration;
	class RHIComputeShader;
	class RHITexture;
	class RHITexture;
	class RHISamplerState;
	class RHIUnorderedAccessView;
	class RHIShaderResourceView;
	class RHIBoundShaderState;
	class RHIVertexShader;
	class RHIHullShader;
	class RHIDomainShader;
	class RHIGeometryShader;
	class RHIPixelShader;
	class RHIComputeFence;
	class RHIViewport;
	struct RHITransition;
	class RHIVertexBuffer;
	class RHIDepthRenderTargetView;
	class RHIRenderTargetView;
	struct ResolveParams;
	typedef TRefCountPtr<RHIBoundShaderState> BoundShaderStateRHIRef;

	BoundShaderStateRHIRef RHICreateBoundShaderState(RHIVertexDeclaration* vertexDeclaration, RHIVertexShader* vertexShaderRHI, RHIHullShader* hullShaderRHI, RHIDomainShader* domainShaderRHI, RHIGeometryShader* geometryShaderRHI, RHIPixelShader* pixelShaderRHI);

	class IRHIComputeContext
	{
	public:
		virtual void RHISetShaderTexture(RHIComputeShader* computeShader, uint32 textureIndex, RHITexture* newTexture) = 0;

		virtual void RHISetShaderSampler(RHIComputeShader* computeShader, uint32 samplerIndex, RHISamplerState* newState) = 0;

		virtual void RHISetShaderParameter(RHIComputeShader* computeShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newData) = 0;

		virtual void RHISetScissorRect(bool bEnable, uint32 minX, uint32 minY, uint32 maxX, uint32 maxY) = 0;

		virtual void RHISetComputeShader(RHIComputeShader* computeShader) = 0;

		virtual void RHISetUAVParameter(RHIComputeShader* computeShader, uint32 uavIndex, RHIUnorderedAccessView* uav) = 0;

		virtual void RHISetUAVParameter(RHIComputeShader* computeShader, uint32 uavIndex, RHIUnorderedAccessView* uav, uint32 initialCount) = 0;

		virtual void RHIDispatchComputeShader(uint32 threadGroupCountX, uint32 threadGroupCountY, uint32 threadGroupCountZ) = 0;

		virtual void RHISetShaderResourceViewParameter(RHIComputeShader* computeShader, uint32 samplerIndex, RHIShaderResourceView* srv) = 0;

		virtual void RHIWaitComputeFence(RHIComputeFence* inFence) = 0;
		
		virtual void RHISubmitCommandsHint() = 0;

		virtual void RHIBeginTransitions(TArrayView<const RHITransition*> transitions) = 0;

		virtual void RHIEndTransitions(TArrayView<const RHITransition*> transitions) = 0;
	};

	class IRHICommandContext : public IRHIComputeContext
	{
	public:
		virtual ~IRHICommandContext()
		{}

		virtual void RHIWaitComputeFence(RHIComputeFence* inFence) override
		{
			if (inFence)
			{
				BOOST_ASSERT(inFence->getWriteEnqueued());
			}
		}

		virtual void RHIBeginDrawingViewport(RHIViewport* viewport, RHITexture* renderTargetRHI) = 0;
		virtual void RHIEndDrawingViewport(RHIViewport* viewport, bool bPresent, bool bLockToVsync) = 0;

		virtual void RHIBeginFrame() = 0;

		virtual void RHIEndFrame() = 0;

		virtual void RHIBeginScene() = 0;

		virtual void RHIEndScene() = 0;

		virtual void RHICopyToResolveTarget(RHITexture* sourceTexture, RHITexture* destTexture, const ResolveParams& resolveParams) = 0;

		virtual void RHISetRenderTargets(uint32 NumSimultaneousRenderTargets, const RHIRenderTargetView* newRenderTargets, const RHIDepthRenderTargetView* newDepthStencilTarget, uint32 numUAVs, RHIUnorderedAccessView* const* UAVs) = 0;

		virtual void RHISetBlendState(RHIBlendState* newState, const LinearColor& blendFactory) = 0;

		virtual void RHISetDepthStencilState(RHIDepthStencilState* newState, uint32 inStencilRef) = 0;

		virtual void RHISetRasterizerState(RHIRasterizerState* newState) = 0;

		virtual void RHISetStencilRef(uint32 stencilRef) {}

		virtual void RHISetViewport(uint32 x, uint32 y, float z, uint32 width, uint32 height, float depth) = 0;

		virtual void RHISetMultipleViewports(uint32 count, const ViewportBounds* data) = 0;

		virtual void RHISetRenderTargetsAndClear(const RHISetRenderTargetsInfo& renderTargetsInfo) = 0;

		virtual void RHIBindClearMRTValues(bool bClearColor, bool bClearDepth, bool bClearStencil) = 0;

		virtual void RHISetBoundShaderState(RHIBoundShaderState* boundShaderState) = 0;

		virtual void RHIDrawIndexedPrimitive(RHIIndexBuffer* indexBuffer, uint32 primitiveType, int32 baseVertexIndex, uint32 firstInstance, uint32 numVertex, uint32 startIndex, uint32 numPrimitives, uint32 numInstances) = 0;

		virtual void RHIDrawPrimitive(int32 baseVertexIndex, uint32 numPrimitives, uint32 numInstances) = 0;

		virtual void RHISetShaderTexture(RHIVertexShader* vertexShader, uint32 textureIndex, RHITexture* newTexture) = 0;

		virtual void RHISetShaderTexture(RHIHullShader* hullShader, uint32 textureIndex, RHITexture* newTexture) = 0;

		virtual void RHISetShaderTexture(RHIDomainShader* domainShader, uint32 textureIndex, RHITexture* newTexture) = 0;

		virtual void RHISetShaderTexture(RHIGeometryShader* geometryShader, uint32 textureIndex, RHITexture* newTexture) = 0;

		virtual void RHISetShaderTexture(RHIPixelShader* pixelShader, uint32 textureIndex, RHITexture* newTexture) = 0;

		virtual void RHISetShaderTexture(RHIComputeShader* computeShader, uint32 textureIndex, RHITexture* newTexture) = 0;

		virtual void RHISetShaderResourceViewParameter(RHIVertexShader* vertexShader, uint32 samplerIndex, RHIShaderResourceView* srv) = 0;

		virtual void RHISetShaderResourceViewParameter(RHIDomainShader* domainShader, uint32 samplerIndex, RHIShaderResourceView* srv) = 0;


		virtual void RHISetShaderResourceViewParameter(RHIHullShader* hullShader, uint32 samplerIndex, RHIShaderResourceView* srv) = 0;

		virtual void RHISetShaderResourceViewParameter(RHIGeometryShader* geometryShader, uint32 samplerIndex, RHIShaderResourceView* srv) = 0;

		virtual void RHISetShaderResourceViewParameter(RHIPixelShader* pixelShader, uint32 samplerIndex, RHIShaderResourceView* srv) = 0;

		virtual void RHISetShaderResourceViewParameter(RHIComputeShader* computeShader, uint32 samplerIndex, RHIShaderResourceView* srv) = 0;

		virtual void RHIAutomaticCacheFlushAfterComputeShader(bool bEnable) = 0;

		virtual void RHIFlushComputeShaderCache() = 0;

		virtual void RHIDispatchComputeShader(uint32 threadGroupCountX, uint32 threadGroupCountY, uint32 threadGroupCountZ) = 0;

		virtual void RHISetStereoViewport(uint32 leftMinX, uint32 rightMinX, uint32 leftMinY, uint32 rightMinY, float z, uint32 leftMaxX, uint32 rightMaxX, uint32 leftMaxY, uint32 rightMaxY, float maxZ)
		{

		}

		virtual void RHISetShaderSampler(RHIVertexShader* vertexShader, uint32 samplerIndex, RHISamplerState* newSampler) = 0;

		virtual void RHISetShaderSampler(RHIHullShader* hullShader, uint32 textureIndex, RHISamplerState* newSampler) = 0;

		virtual void RHISetShaderSampler(RHIDomainShader* domainShader, uint32 samplerIndex, RHISamplerState* newSampler) = 0;

		virtual void RHISetShaderSampler(RHIGeometryShader* geometryShader, uint32 samplerIndex, RHISamplerState* newSampler) = 0;

		virtual void RHISetShaderSampler(RHIPixelShader* pixelShader, uint32 samplerIndex, RHISamplerState* newSampler) = 0;

		virtual void RHISetShaderSampler(RHIComputeShader* computeShader, uint32 samplerIndex, RHISamplerState* newSampler) = 0;

		virtual void RHISetStreamSource(uint32 streamIndex, RHIVertexBuffer* vertexBuffer, uint32 offset) = 0;

		virtual void RHISetShaderConstantBuffer(RHIVertexShader* vertexShader, uint32 bufferIndex, RHIConstantBuffer* buffer) = 0;

		virtual void RHISetShaderConstantBuffer(RHIHullShader* hullShader, uint32 bufferIndex, RHIConstantBuffer* buffer) = 0;

		virtual void RHISetShaderConstantBuffer(RHIDomainShader* domainShader, uint32 bufferIndex, RHIConstantBuffer* buffer) = 0;

		virtual void RHISetShaderConstantBuffer(RHIGeometryShader* geometryShader, uint32 bufferIndex, RHIConstantBuffer* buffer) = 0;

		virtual void RHISetShaderConstantBuffer(RHIPixelShader* pixelShader, uint32 bufferIndex, RHIConstantBuffer* buffer) = 0;

		virtual void RHISetShaderConstantBuffer(RHIComputeShader* computeShader, uint32 bufferIndex, RHIConstantBuffer* buffer) = 0;

		virtual void RHISetShaderParameter(RHIVertexShader* vertexShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) = 0;

		virtual void RHISetShaderParameter(RHIHullShader* hullShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) = 0;

		virtual void RHISetShaderParameter(RHIDomainShader* domainShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) = 0;

		virtual void RHISetShaderParameter(RHIGeometryShader* geometryShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) = 0;

		virtual void RHISetShaderParameter(RHIPixelShader* pixelShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) = 0;

		virtual void RHISetShaderParameter(RHIComputeShader* computeShader, uint32 bufferIndex, uint32 baseIndex, uint32 numBytes, const void* newValue) = 0;

		virtual void RHIUpdateTextureReference(RHITextureReference* textureRHI, RHITexture* newTexture) = 0;

		virtual void RHISetGraphicsPipelineState(RHIGraphicsPipelineState* graphicsState) = 0;

		virtual void RHISetBlendFactor(const LinearColor& blendFactor) {}

		virtual void RHIBeginRenderPass(const RHIRenderPassInfo& inInfo, const TCHAR* inName) = 0;

		virtual void RHIEndRenderPass() = 0;

	protected:
		RHIRenderPassInfo mRenderPassInfo;
	};

	class IRHICommandContextPSOFallback : public IRHICommandContext
	{
	public:

		virtual void RHIEnableDepthBoundsTest(bool bEnable) = 0;

		virtual void RHISetDepthBounds(float minDepth, float maxDepth) = 0;

		virtual void RHISetGraphicsPipelineState(RHIGraphicsPipelineState* graphicsState) override
		{
			RHIGraphicsPipelineStateFallBack* fallbackGraphicsState = static_cast<RHIGraphicsPipelineStateFallBack*>(graphicsState);
			GraphicsPipelineStateInitializer& psoInit = fallbackGraphicsState->mInitializer;
			RHISetBoundShaderState(
				RHICreateBoundShaderState(
					psoInit.mBoundShaderState.mVertexDeclarationRHI,
					psoInit.mBoundShaderState.mVertexShaderRHI,
					psoInit.mBoundShaderState.mHullShaderRHI,
					psoInit.mBoundShaderState.mDomainShaderRHI,
					psoInit.mBoundShaderState.mGeometryShaderRHI,
					psoInit.mBoundShaderState.mPixelShaderRHI
				).getReference());
			RHISetDepthStencilState(fallbackGraphicsState->mInitializer.mDepthStencilState, 0);
			RHISetRasterizerState(fallbackGraphicsState->mInitializer.mRasterizerState);
			RHISetBlendState(fallbackGraphicsState->mInitializer.mBlendState, LinearColor(1.0f, 1.0f, 1.0f, 1.0f));
			if (GSupportsDepthBoundsTest)
			{
				RHIEnableDepthBoundsTest(fallbackGraphicsState->mInitializer.bDepthBounds);
			}
		}
	};
}