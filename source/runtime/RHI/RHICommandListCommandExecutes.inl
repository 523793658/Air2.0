#if !defined(INTERNAL_DECORATOR)
#define INTERNAL_DECORATOR(Method) cmdList.getContext().Method
#endif

#if !defined(INTERNAL_DECORATOR_COMPUTE)
#define INTERNAL_DECORATOR_COMPUTE(Method) cmdList.getComputeContext().Method
#endif

#if !defined(INTERNAL_DECORATOR_CONTEXT_PARAM1)
#define INTERNAL_DECORATOR_CONTEXT(Method) cmdList.getComputeContext().Method
#endif

void RHICommandSubmitCommandsHint::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR_CONTEXT(RHISubmitCommandsHint)();
}

void RHICommandEndDrawingViewport::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIEndDrawingViewport)(viewport, bPresent, bLockToVsync);
}

void RHICommandBeginTransitions::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR_COMPUTE(RHIBeginTransitions)(mTransitions);
	for (const RHITransition* transition : mTransitions)
	{
		transition->markBegin(cmdList.getPipeline());
	}
}

void RHICommandResourceTransition::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR_COMPUTE(RHIBeginTransitions)(makeArrayView((const RHITransition**)&mTransition, 1));
	INTERNAL_DECORATOR_COMPUTE(RHIEndTransitions)(makeArrayView((const RHITransition**)&mTransition, 1));

	GDynamicRHI->RHIReleaseTransition(mTransition);
	mTransition->~RHITransition();
}

void RHICommandEndTransitions::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR_COMPUTE(RHIEndTransitions)(mTransitions);
	for (const RHITransition* transition : mTransitions)
	{
		transition->markEnd(cmdList.getPipeline());
	}
}

void RHICommandBeginScene::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIBeginScene)();
}

void RHICommandEndScene::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIEndScene)();
}

void RHICommandBeginFrame::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIBeginFrame)();
}

void RHICommandEndFrame::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIEndFrame)();
}

void RHICommandSetRenderTargetsAndClear::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetRenderTargetsAndClear)(mRenderTargetInfo);
}


void RHICommandBindClearMRTValues::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIBindClearMRTValues)(bClearColor, bClearDepth, bClearStencil);
}

void RHICommandBuildLocalConstantBuffer::execute(RHICommandListBase& cmdList)
{
	BOOST_ASSERT(!isValidRef(mWorkArea.mComputedConstantBuffer->mConstantBuffer));
	BOOST_ASSERT(mWorkArea.mLayout);
	BOOST_ASSERT(mWorkArea.mContents);
	if (mWorkArea.mComputedConstantBuffer->mUseCount)
	{
		mWorkArea.mComputedConstantBuffer->mConstantBuffer = RHICreateConstantBuffer(mWorkArea.mContents, *mWorkArea.mLayout, ConstantBuffer_SingleFrame);
	}
	mWorkArea.mLayout = nullptr;
	mWorkArea.mContents = nullptr;
}

void RHICommandSetStencilRef::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetStencilRef)(mStencilRef);
}

void RHICommandSetViewport::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetViewport)(mX, mY, mZ, mWidth, mHeight, mDepth);
}

void RHICommandSetBlendFactor::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetBlendFactor)(mBlendFactor);
}


template<typename TRHIShader, ECmdList CmdListType>
void RHICommandSetShaderResourceViewParameter<TRHIShader, CmdListType>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetShaderResourceViewParameter)(mShader, mSamplerIndex, mSRV);
}

template struct RHICommandSetShaderResourceViewParameter<RHIVertexShader, ECmdList::EGfx>;
template struct RHICommandSetShaderResourceViewParameter<RHIHullShader, ECmdList::EGfx>;
template struct RHICommandSetShaderResourceViewParameter<RHIDomainShader, ECmdList::EGfx>;
template struct RHICommandSetShaderResourceViewParameter<RHIGeometryShader, ECmdList::EGfx>;
template struct RHICommandSetShaderResourceViewParameter<RHIPixelShader, ECmdList::EGfx>;
template struct RHICommandSetShaderResourceViewParameter<RHIComputeShader, ECmdList::EGfx>;
template<> void RHICommandSetShaderResourceViewParameter<RHIComputeShader, ECmdList::ECompute>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR_COMPUTE(RHISetShaderResourceViewParameter)(mShader, mSamplerIndex, mSRV);
};

template<typename TRHIShader, ECmdList cmdListType>
void RHICommandSetShaderConstantBuffer<TRHIShader, cmdListType>::execute(RHICommandListBase & cmdList)
{
	INTERNAL_DECORATOR(RHISetShaderConstantBuffer)(mShader, mBaseIndex, mConstantBuffer);
}

template struct RHICommandSetShaderConstantBuffer<RHIVertexShader, ECmdList::EGfx>;
template struct RHICommandSetShaderConstantBuffer<RHIHullShader, ECmdList::EGfx>;
template struct RHICommandSetShaderConstantBuffer<RHIDomainShader, ECmdList::EGfx>;
template struct RHICommandSetShaderConstantBuffer<RHIGeometryShader, ECmdList::EGfx>;
template struct RHICommandSetShaderConstantBuffer<RHIPixelShader, ECmdList::EGfx>;
template struct RHICommandSetShaderConstantBuffer<RHIComputeShader, ECmdList::EGfx>;
template struct RHICommandSetShaderConstantBuffer<RHIComputeShader, ECmdList::ECompute>;

template<typename TRHIShader>
void RHICommandSetLocalConstantBuffer<TRHIShader>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetShaderConstantBuffer)(mShader, mBaseIndex, mLocalConstantBuffer.mWorkArea->mComputedConstantBuffer->mConstantBuffer);
	if (--mLocalConstantBuffer.mWorkArea->mComputedConstantBuffer->mUseCount == 0)
	{
		mLocalConstantBuffer.mWorkArea->mComputedConstantBuffer->~ComputedConstantBuffer();
	}
}
template struct RHICommandSetLocalConstantBuffer<RHIVertexShader>;
template struct RHICommandSetLocalConstantBuffer<RHIHullShader>;
template struct RHICommandSetLocalConstantBuffer<RHIDomainShader>;
template struct RHICommandSetLocalConstantBuffer<RHIGeometryShader>;
template struct RHICommandSetLocalConstantBuffer<RHIPixelShader>;
template struct RHICommandSetLocalConstantBuffer<RHIComputeShader>;


void RHICommandSetBoundShaderState::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetBoundShaderState)(mBoundShaderState);
}


void RHICommandDrawIndexedPrimitive::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIDrawIndexedPrimitive)(mIndexBuffer, mPrimitiveType, mBaseVertexIndex, mFirstInstance, mNumVertex, mStartIndex, mNumPrimitives, mNumInstances);
}

void RHICommandDrawPrimitive::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIDrawPrimitive)(mBaseVertexIndex, mNumPrimitives, mNumInstances);
}

template<typename TRHIShader, ECmdList cmdListType>
void RHICommandSetShaderTexture<TRHIShader, cmdListType>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetShaderTexture)(mShader, mTextureIndex, mTexture);
}

template struct RHICommandSetShaderTexture<RHIVertexShader, ECmdList::EGfx>;
template struct RHICommandSetShaderTexture<RHIHullShader, ECmdList::EGfx>;
template struct RHICommandSetShaderTexture<RHIDomainShader, ECmdList::EGfx>;
template struct RHICommandSetShaderTexture<RHIGeometryShader, ECmdList::EGfx>;
template struct RHICommandSetShaderTexture<RHIPixelShader, ECmdList::EGfx>;
template struct RHICommandSetShaderTexture<RHIComputeShader, ECmdList::EGfx>;
template<> void RHICommandSetShaderTexture<RHIComputeShader, ECmdList::ECompute>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR_COMPUTE(RHISetShaderTexture)(mShader, mTextureIndex, mTexture);
}




template<typename TRHIShader, ECmdList cmdListType>
void RHICommandSetShaderSampler<TRHIShader, cmdListType>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetShaderSampler)(mShader, mSamplerIndex, mState);
}

template struct RHICommandSetShaderSampler<RHIVertexShader, ECmdList::EGfx>;
template struct RHICommandSetShaderSampler<RHIHullShader, ECmdList::EGfx>;
template struct RHICommandSetShaderSampler<RHIDomainShader, ECmdList::EGfx>;
template struct RHICommandSetShaderSampler<RHIGeometryShader, ECmdList::EGfx>;
template struct RHICommandSetShaderSampler<RHIPixelShader, ECmdList::EGfx>;
template struct RHICommandSetShaderSampler<RHIComputeShader, ECmdList::EGfx>;
template<> void RHICommandSetShaderSampler<RHIComputeShader, ECmdList::ECompute>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR_COMPUTE(RHISetShaderSampler)(mShader, mSamplerIndex, mState);
}

void RHICommandSetStreamSource::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetStreamSource)(mStreamIndex, mVertexBuffer, mOffset);
}

void RHICommandSetGraphicsPipelineState::execute(RHICommandListBase& cmdList)
{
	extern RHIGraphicsPipelineState* executeSetGraphicsPipelineState(GraphicsPipelineState * graphicsPipelineState);
	RHIGraphicsPipelineState* RHiGraphicsPipelineState = executeSetGraphicsPipelineState(mGraphicsPipelineState);
	INTERNAL_DECORATOR(RHISetGraphicsPipelineState)(RHiGraphicsPipelineState);
}

template<typename TRHIShader, ECmdList CmdListType>
void RHICommandSetShaderParameter<TRHIShader, CmdListType>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetShaderParameter)(mShader, mBufferIndex, mBaseIndex, mNumBytes, newValue);
}
template struct RHICommandSetShaderParameter<RHIVertexShader, ECmdList::EGfx>;
template struct RHICommandSetShaderParameter<RHIHullShader, ECmdList::EGfx>;
template struct RHICommandSetShaderParameter<RHIDomainShader, ECmdList::EGfx>;
template struct RHICommandSetShaderParameter<RHIGeometryShader, ECmdList::EGfx>;
template struct RHICommandSetShaderParameter<RHIPixelShader, ECmdList::EGfx>;
template struct RHICommandSetShaderParameter<RHIComputeShader, ECmdList::EGfx>;
template<> void RHICommandSetShaderParameter<RHIComputeShader, ECmdList::ECompute>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR_COMPUTE(RHISetShaderParameter)(mShader, mBufferIndex, mBaseIndex, mNumBytes, newValue);
}

void RHICommandCopyToResolveTarget::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHICopyToResolveTarget)(mSourceTexture, mDestTexture, mResolveParams);
}

void RHICommandSetScissorRect::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetScissorRect)(bEnable, mMinX, mMinY, mMaxX, mMaxY);
}

void RHICommandSetRenderTargets::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetRenderTargets)(
		mNewNumSimultaneousRenderTargets,
		mNewRenderTargetsRHI,
		&mNewDepthStencilTarget,
		mNumUAVs,
		mUAVs
		);
}

void RHICommandUpdateTextureReference::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIUpdateTextureReference)(mTextureRef, mNewTexture);
}

void RHICommandBeginRenderPass::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIBeginRenderPass)(mInfo, mName);
}

void RHICommandEndRenderPass::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIEndRenderPass)();
}

void RHICommandAutomaticCacheFlushAfterComputeShader::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIAutomaticCacheFlushAfterComputeShader)(bEnable);
}

void RHICommandFlushComputeShaderCache::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIFlushComputeShaderCache)();
}

void RHICommandSetStereoViewport::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetStereoViewport)(mLeftMinX, mRightMinX, mLeftMinY, mRightMinY, mMinZ, mLeftMaxX, mRightMaxX, mLeftMaxY, mRightMaxY, mMaxZ);
}

template<ECmdList CmdListType>
void RHICommandDispatchComputeShader<CmdListType>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR_CONTEXT(RHIDispatchComputeShader)(mThreadGroupCountX, mThreadGroupCountY, mThreadGroupCountZ);
}

template<ECmdList CmdListType>
void RHICommandSetComputeShader<CmdListType>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR_CONTEXT(RHISetComputeShader)(mShader);
}

template<typename TRHIShader, ECmdList CmdListType>
void RHICommandSetUAVParameter<TRHIShader, CmdListType>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR_CONTEXT(RHISetUAVParameter)(mShader, mUAVIndex, mUAV);
}

template<typename TRHIShader, ECmdList CmdListType>
void RHICommandSetUAVParameter_InitialCount<TRHIShader, CmdListType>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR_CONTEXT(RHISetUAVParameter)(mShader, mUAVIndex, mUAV, mInitialCount);
}

void RHICommandBeginDrawingViewport::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIBeginDrawingViewport)(mViewport, mRenderTargetRHI);
}
