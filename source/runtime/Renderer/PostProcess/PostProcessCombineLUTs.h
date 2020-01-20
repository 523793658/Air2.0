#pragma once
#include "CoreMinimal.h"
#include "PostProcess/RenderingCompositionGraph.h"
namespace Air
{
	bool pipelineVolumeTextureLUTSupportGuaranteedAtRuntime(EShaderPlatform platform);

	class RCPassPostProcessCombineLUTs : public TRenderingCompositePassBase<0, 1>
	{
	public:
		RCPassPostProcessCombineLUTs(EShaderPlatform inShaderPlatform, bool bInAllocateOutput, bool inIsComputePass, bool bInNeedFloatOutput)
			:mShaderPlatform(inShaderPlatform)
			, bAllocateOutput(bInAllocateOutput)
			, bNeedFloatOutput(bInNeedFloatOutput)
		{
			bIsComputePass = inIsComputePass;
			bPreferAsyncCompute = false;
		}

		virtual void process(RenderingCompositePassContext& context) override;

		virtual void release() override { delete this; }

		virtual PooledRenderTargetDesc computeOutputDesc(EPassOutputId inPassOutputId) const override;

		uint32 generateFinalTable(const FinalPostProcessSettings& settings, Texture* outTetures[], float outWeights[], uint32 maxCount) const;

		uint32 findIndex(const FinalPostProcessSettings& settings, Texture* tex)const;

		virtual RHIComputeFence* getComputePassEndFence() const override { return mAsyncEndFence; }

	private:
		template<typename TRHICmdList>
		void dispatchCS(TRHICmdList& RHICmdList, RenderingCompositePassContext& context, const IntRect& destRect, RHIUnorderedAccessView* destUAV, int32 blendCount, Texture* textures[], float weights[]);

		ComputeFenceRHIRef mAsyncEndFence;
		EShaderPlatform mShaderPlatform;
		bool bAllocateOutput;
		bool bNeedFloatOutput;
	};
}