#pragma once
#include "PostProcess/RenderingCompositionGraph.h"
namespace Air
{
	class RCPassPostProcessInput : public TRenderingCompositePassBase<0, 1>
	{
	public:
		RCPassPostProcessInput(TRefCountPtr<IPooledRenderTarget>& inData);

		virtual void process(RenderingCompositePassContext& context) override;

		virtual void release() override { delete this; }

		virtual PooledRenderTargetDesc computeOutputDesc(EPassOutputId inPassOutputId) const override;

	protected:
		TRefCountPtr<IPooledRenderTarget> mData;
	};

	class RCPassPostProcessInputSingleUse : public RCPassPostProcessInput
	{
		RCPassPostProcessInputSingleUse(TRefCountPtr<IPooledRenderTarget>& inData)
			:RCPassPostProcessInput(inData)
		{}

		virtual void process(RenderingCompositePassContext& context) override;

		virtual void release() override { delete this; }
	};
}