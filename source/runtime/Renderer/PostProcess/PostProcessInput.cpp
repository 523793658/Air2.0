#include "PostProcess/PostProcessInput.h"
namespace Air
{
	RCPassPostProcessInput::RCPassPostProcessInput(TRefCountPtr<IPooledRenderTarget>& inData)
		:mData(inData)
	{}

	void RCPassPostProcessInput::process(RenderingCompositePassContext& context)
	{
		mPassOutputs[0].mPooledRenderTarget = mData;
	}

	PooledRenderTargetDesc RCPassPostProcessInput::computeOutputDesc(EPassOutputId inPassOutputId) const
	{
		BOOST_ASSERT(mData);
		PooledRenderTargetDesc ret = mData->getDesc();
		return ret;
	}

	void RCPassPostProcessInputSingleUse::process(RenderingCompositePassContext& context)
	{
		mPassOutputs[0].mPooledRenderTarget = mData;
		mData.safeRelease();
	}
}