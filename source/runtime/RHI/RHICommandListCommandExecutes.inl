#if !defined(INTERNAL_DECORATOR)
#define INTERNAL_DECORATOR(Method) cmdList.getContext().Method
#endif

#if !defined(INTERNAL_DECORATOR_COMPUTE)
#define INTERNAL_DECORATOR_COMPUTE(Method) cmdList.getComputeContext().Method
#endif


void RHICommandBeginDrawingViewport::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIBeginDrawingViewport)(mViewport, mRenderTargetRHI);
}

void RHICommandEndDrawingViewport::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIEndDrawingViewport)(viewport, bPresent, bLockToVsync);
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

void RHICommandTransitionTextures::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHITransitionResources)(transitionType, &textures[0], numTextures);
}

void RHICommandTransitionTexturesArray::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHITransitionResources)(mTransitionType, &mTextures[0], mTextures.size());
}

void RHICommandSetRenderTargetsAndClear::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetRenderTargetsAndClear)(mRenderTargetInfo);
}

void RHICommandClearColorTextures::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIClearColorTextures)(mNumClearColors, mTextures, mColorArray, mExcludeRect);
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

void RHICommandSetBlendState::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetBlendState)(mState, mBlendFactor);
}

void RHICommandSetRasterizerState::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetRasterizerState)(mState);
}
void RHICommandSetDepthStencilState::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetDepthStencilState)(mState, mStencilRef);
}

void RHICommandSetStencilRef::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetStencilRef)(mStencilRef);
}

void RHICommandSetViewport::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetViewport)(mX, mY, mZ, mWidth, mHeight, mDepth);
}
void RHICommandBuildLocalBoundShaderState::execute(RHICommandListBase& cmdList)
{
	BOOST_ASSERT(!isValidRef(mWorkArea.mComputedBSS->mBSS));
	if (mWorkArea.mComputedBSS->mUseCount)
	{
		mWorkArea.mComputedBSS->mBSS = RHICreateBoundShaderState(mWorkArea.mArgs.mVertexDeclarationRHI,
			mWorkArea.mArgs.mVertexShaderRHI,
			mWorkArea.mArgs.mHullShaderRHI,
			mWorkArea.mArgs.mDomainShaderRHI,
			mWorkArea.mArgs.mGeometryShaderRHI,
			mWorkArea.mArgs.mPixelShaderRHI);
	}
}

void RHICommandSetLocalBoundShaderState::execute(RHICommandListBase& cmdList)
{
	BOOST_ASSERT(mLocalBoundShaderState.mWorkArea->mComputedBSS->mUseCount > 0 && isValidRef(mLocalBoundShaderState.mWorkArea->mComputedBSS->mBSS));
	INTERNAL_DECORATOR(RHISetBoundShaderState)(mLocalBoundShaderState.mWorkArea->mComputedBSS->mBSS);
	if (--mLocalBoundShaderState.mWorkArea->mComputedBSS->mUseCount == 0)
	{
		mLocalBoundShaderState.mWorkArea->mComputedBSS->~ComputedBSS();
	}
}

template<typename TShaderRHIParamRef, ECmdList cmdListType>
void RHICommandSetShaderConstantBuffer<TShaderRHIParamRef, cmdListType>::execute(RHICommandListBase & cmdList)
{
	INTERNAL_DECORATOR(RHISetShaderConstantBuffer)(mShader, mBaseIndex, mConstantBuffer);
}

template struct RHICommandSetShaderConstantBuffer<VertexShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderConstantBuffer<HullShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderConstantBuffer<DomainShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderConstantBuffer<GeometryShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderConstantBuffer<PixelShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderConstantBuffer<ComputeShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderConstantBuffer<ComputeShaderRHIParamRef, ECmdList::ECompute>;

template<typename TShaderRHIParamRef>
void RHICommandSetLocalConstantBuffer<TShaderRHIParamRef>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetShaderConstantBuffer)(mShader, mBaseIndex, mLocalConstantBuffer.mWorkArea->mComputedConstantBuffer->mConstantBuffer);
	if (--mLocalConstantBuffer.mWorkArea->mComputedConstantBuffer->mUseCount == 0)
	{
		mLocalConstantBuffer.mWorkArea->mComputedConstantBuffer->~ComputedConstantBuffer();
	}
}
template struct RHICommandSetLocalConstantBuffer<VertexShaderRHIParamRef>;
template struct RHICommandSetLocalConstantBuffer<HullShaderRHIParamRef>;
template struct RHICommandSetLocalConstantBuffer<DomainShaderRHIParamRef>;
template struct RHICommandSetLocalConstantBuffer<GeometryShaderRHIParamRef>;
template struct RHICommandSetLocalConstantBuffer<PixelShaderRHIParamRef>;
template struct RHICommandSetLocalConstantBuffer<ComputeShaderRHIParamRef>;


void RHICommandSetBoundShaderState::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetBoundShaderState)(mBoundShaderState);
}

void RHICommandEndDrawIndexedPrimitiveUP::execute(RHICommandListBase& cmdList)
{
	void* vertexBuffer = nullptr;
	void* indexBuffer = nullptr;
	INTERNAL_DECORATOR(RHIBeginDrawIndexedPrimitiveUP)(
		mPrimitiveType,
		mNumPrimitive,
		mNumVertices,
		mVertexDataStride,
		vertexBuffer,
		mMinVertexIndex,
		mNumIndices,
		mIndexDataStride,
		indexBuffer);
	Memory::memcpy(vertexBuffer, mOutVertexData, mNumVertices * mVertexDataStride);
	Memory::memcpy(indexBuffer, mOutIndexData, mNumIndices * mIndexDataStride);
	INTERNAL_DECORATOR(RHIEndDrawIndexedPrimitiveUP)();
}

void RHICommandEndDrawPrimitiveUP::execute(RHICommandListBase& cmdList)
{
	void* buffer = nullptr;
	INTERNAL_DECORATOR(RHIBeginDrawPrimitiveUP)(mPrimitiveType, mNumPrimitives, mNumVertices, mVertexDataStride, buffer);
	Memory::memcpy(buffer, mOutVertexData, mNumVertices * mVertexDataStride);
	INTERNAL_DECORATOR(RHIEndDrawPrimitiveUP)();
}

void RHICommandDrawIndexedPrimitive::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIDrawIndexedPrimitive)(mIndexBuffer, mPrimitiveType, mBaseVertexIndex, mFirstInstance, mNumVertex, mStartIndex, mNumPrimitives, mNumInstances);
}

void RHICommandDrawPrimitive::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIDrawPrimitive)(mPrimitiveType, mBaseVertexIndex, mNumPrimitives, mNumInstances);
}

template<typename TShaderRHIParamRef, ECmdList cmdListType>
void RHICommandSetShaderTexture<TShaderRHIParamRef, cmdListType>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetShaderTexture)(mShader, mTextureIndex, mTexture);
}

template struct RHICommandSetShaderTexture<VertexShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderTexture<HullShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderTexture<DomainShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderTexture<GeometryShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderTexture<PixelShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderTexture<ComputeShaderRHIParamRef, ECmdList::EGfx>;
template<> void RHICommandSetShaderTexture<ComputeShaderRHIParamRef, ECmdList::ECompute>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR_COMPUTE(RHISetShaderTexture)(mShader, mTextureIndex, mTexture);
}




template<typename TShaderRHIParamRef, ECmdList cmdListType>
void RHICommandSetShaderSampler<TShaderRHIParamRef, cmdListType>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetShaderSampler)(mShader, mSamplerIndex, mState);
}

template struct RHICommandSetShaderSampler<VertexShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderSampler<HullShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderSampler<DomainShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderSampler<GeometryShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderSampler<PixelShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderSampler<ComputeShaderRHIParamRef, ECmdList::EGfx>;
template<> void RHICommandSetShaderSampler<ComputeShaderRHIParamRef, ECmdList::ECompute>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR_COMPUTE(RHISetShaderSampler)(mShader, mSamplerIndex, mState);
}

void RHICommandSetStreamSource::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetStreamSource)(mStreamIndex, mVertexBuffer, mStride, mOffset);
}

template<typename TShaderRHIParamRef, ECmdList CmdListType>
void RHICommandSetShaderParameter<TShaderRHIParamRef, CmdListType>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHISetShaderParameter)(mShader, mBufferIndex, mBaseIndex, mNumBytes, newValue);
}
template struct RHICommandSetShaderParameter<VertexShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderParameter<HullShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderParameter<DomainShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderParameter<GeometryShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderParameter<PixelShaderRHIParamRef, ECmdList::EGfx>;
template struct RHICommandSetShaderParameter<ComputeShaderRHIParamRef, ECmdList::EGfx>;
template<> void RHICommandSetShaderParameter<ComputeShaderRHIParamRef, ECmdList::ECompute>::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR_COMPUTE(RHISetShaderParameter)(mShader, mBufferIndex, mBaseIndex, mNumBytes, newValue);
}

void RHICommandCopyToResolveTarget::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHICopyToResolveTarget)(mSourceTexture, mDestTexture, bKeepOriginalSurface, mResolveParams);
}

void RHICommandClearDepthStencilTexture::execute(RHICommandListBase& cmdList)
{
	INTERNAL_DECORATOR(RHIClearDepthStencilTexture)(mTexture, mClearDepthStencil, mDepth, mStencil, mExcludeRect);
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