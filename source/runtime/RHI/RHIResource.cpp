#include "RHIResource.h"

namespace Air
{
	void RHIRenderPassInfo::convertToRenderTargetsInfo(RHISetRenderTargetsInfo& outRTInfo) const
	{
		for (int32 index = 0; index < MaxSimultaneousRenderTargets; +index)
		{
			if (!mColorRenderTargets[index].mRenderTarget)
			{
				break;
			}

			outRTInfo.mColorRenderTarget[index] = mColorRenderTargets[index].mRenderTarget;
			ERenderTargetLoadAction loadAction = getLoadAction(mColorRenderTargets[index].mAction);
			outRTInfo.mColorRenderTarget[index].mLoadAction = loadAction;
			outRTInfo.mColorRenderTarget[index].mStoreAction = getStoreAction(mColorRenderTargets[index].mAction);
			outRTInfo.mColorRenderTarget[index].mArraySliceIndex = mColorRenderTargets[index].mArraySlice;
			outRTInfo.mColorRenderTarget[index].mMipIndex = mColorRenderTargets[index].mMipIndex;
			++outRTInfo.mNumColorRenderTargets;

			outRTInfo.bClearColor |= (loadAction == ERenderTargetLoadAction::EClear);

			BOOST_ASSERT(!outRTInfo.bHasResolveAttachments || mColorRenderTargets[index].mResolveTarget);
			if (mColorRenderTargets[index].mResolveTarget)
			{
				outRTInfo.bHasResolveAttachments = true;
				outRTInfo.mColorResolveRenderTarget[index] = outRTInfo.mColorRenderTarget[index];
				outRTInfo.mColorResolveRenderTarget[index].mTexture = mColorRenderTargets[index].mResolveTarget;
			}
		}

		ERenderTargetActions depthActions = getDepthActions(mDepthStencilRenderTarget.mActions);
		ERenderTargetActions stencilActions = getStencilActions(mDepthStencilRenderTarget.mActions);
		ERenderTargetLoadAction depthLoadAction = getLoadAction(depthActions);
		ERenderTargetStoreAction depthStoreAction = getStoreAction(depthActions);
		ERenderTargetLoadAction stencilLoadAction = getLoadAction(stencilActions);
		ERenderTargetStoreAction stencilStoreAction = getStoreAction(stencilActions);

		outRTInfo.mDepthStencilRenderTarget = RHIDepthRenderTargetView(mDepthStencilRenderTarget.mDepthStencilTarget, depthLoadAction, depthStoreAction, stencilLoadAction, stencilStoreAction, mDepthStencilRenderTarget.mExculusiveDepthStencil);
		outRTInfo.bClearDepth = (depthLoadAction == ERenderTargetLoadAction::EClear);
		outRTInfo.bClearStencil = (stencilLoadAction == ERenderTargetLoadAction::EClear);

		if (mNumUAVs > 0)
		{
			BOOST_ASSERT(mUAVIndex != -1);
			int32 startingUAVIndex = Math::max(mUAVIndex, outRTInfo.mNumColorRenderTargets);
			BOOST_ASSERT((startingUAVIndex + mNumUAVs) < MaxSimultaneousUAVs);
			outRTInfo.mNumColorRenderTargets = startingUAVIndex;
			for (int32 index = 0; index < mNumUAVs; ++index)
			{
				outRTInfo.mUnorderedAccessView[index] = mUAVs[index];
			}
			outRTInfo.mNumUAVs = mNumUAVs;
		}
	}
}