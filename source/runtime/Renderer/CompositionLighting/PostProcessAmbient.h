#pragma once
#include "PostProcess/RenderingCompositionGraph.h"
namespace Air
{
	class RCPassPostProcessAmbient : public TRenderingCompositePassBase<2, 1>
	{
	public:
		virtual const TCHAR* getDebugName() { return TEXT("RCPassPostProcessAmbient"); }

		virtual void process(RenderingCompositePassContext& context) override;

		virtual void release() override { delete this; }

		virtual bool frameBufferBlendingWithInput0() const { return true; }

		virtual PooledRenderTargetDesc computeOutputDesc(EPassOutputId inPassOutputId) const;

	private:

		template<bool bUseClearCoat>
		void render(RenderingCompositePassContext& context);
	};
}