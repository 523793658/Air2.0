#include "PostProcess/PostProcessing.h"
#include "PostProcess/RenderTargetPool.h"
#include "PostProcess/RenderingCompositionGraph.h"
#include "PostProcess/PostProcessTonemap.h"
#include "PostProcess/PostProcessInput.h"
namespace Air
{
	PostProcessing GPostProcessing;

	PostProcessContext::PostProcessContext(RHICommandListImmediate& inRHICmdList, RenderingCompositionGraph& inGraph, const ViewInfo& inView)
		:mRHICmdList(inRHICmdList)
		,mGraph(inGraph)
		,mView(inView)
		,mSceneColor(nullptr)
		,mSceneDepth(nullptr)
	{
		SceneRenderTargets& sceneContext = SceneRenderTargets::get(inRHICmdList);
		if (sceneContext.isSceneColorAllocated())
		{
			mSceneColor = mGraph.registerPass(new(MemStack::get()) RCPassPostProcessInput(sceneContext.getSceneColor()));
		}

		mSceneDepth = mGraph.registerPass(new(MemStack::get()) RCPassPostProcessInput(sceneContext.mSceneDepthZ));

		mFinalOutput = RenderingCompositeOutputRef(mSceneColor);
	}

	void overrideRenderTarget(RenderingCompositeOutputRef it, TRefCountPtr<IPooledRenderTarget>& rt, PooledRenderTargetDesc& desc)
	{
		for (;;)
		{
			it.getOutput()->mPooledRenderTarget = rt;
			it.getOutput()->mRenderTargetDesc = desc;
			if (!it.getPass()->frameBufferBlendingWithInput0())
			{
				break;
			}
			it = *it.getPass()->getInput(ePId_Input0);
		}
	}

	static void addGammaOnlyTonemapper(PostProcessContext& context)
	{
		RenderingCompositePass* postProcessTonemap = context.mGraph.registerPass(new (MemStack::get()) RCPassPostProcessTonemap(context.mView, true, false, false));
		postProcessTonemap->setInput(ePId_Input0, context.mFinalOutput);
		context.mFinalOutput = RenderingCompositeOutputRef(postProcessTonemap);
	}


	void PostProcessing::process(RHICommandListImmediate& RHICmdList, const ViewInfo& view, TRefCountPtr<IPooledRenderTarget>& velocityRT)
	{
		const auto featureLevel = view.getFeatureLevel();

		GRenderTargetPool.addPhaseEvent(TEXT("PostProcessing"));

		{
			MemMark mark(MemStack::get());
			RenderingCompositePassContext compositeContext(RHICmdList, view);
			PostProcessContext context(RHICmdList, compositeContext.mGraph, view);

			const bool bHDROutputEnabled = GRHISupportsHDROutputs;


			if (allowFullPostProcessing(view, featureLevel))
			{

			}
			else
			{
				addGammaOnlyTonemapper(context);
			}

			if (view.mFamily->mViews[view.mFamily->mViews.size() - 1] == &view)
			{
				SceneRenderTargets::get(RHICmdList).adjustGBufferRefCount(RHICmdList, -1);
			}

			{
				TRefCountPtr<IPooledRenderTarget> temp;
				SceneRenderTargetItem item;
				item.mTargetableTexture = (TextureRHIRef&)view.mFamily->mRenderTarget->getRenderTargetTexture();
				item.mShaderResourceTexture = (TextureRHIRef&)view.mFamily->mRenderTarget->getRenderTargetTexture();

				PooledRenderTargetDesc desc;
				if (view.mFamily->mRenderTarget->getRenderTargetTexture())
				{
					desc.mExtent.x = view.mFamily->mRenderTarget->getRenderTargetTexture()->getWidth();
					desc.mExtent.y = view.mFamily->mRenderTarget->getRenderTargetTexture()->getHeight();
				}
				else
				{
					desc.mExtent = view.mFamily->mRenderTarget->getSizeXY();
				}

				desc.mFormat = bHDROutputEnabled ? GRHIHDRDisplayOutputFormat : PF_B8G8R8A8;

				desc.mNumMips = 1;
				desc.mDebugName = TEXT("FinalPostProcessColor");
				GRenderTargetPool.createUntrackedElement(desc, temp, item);
				overrideRenderTarget(context.mFinalOutput, temp, desc);

				compositeContext.process(context.mFinalOutput.getPass(), TEXT("PostProcessing"));
			}
		}
	}

	bool PostProcessing::allowFullPostProcessing(const ViewInfo& view, ERHIFeatureLevel::Type featureLevel)
	{
		return view.mFamily->mEngineShowFlags.PostProcessing && featureLevel >= ERHIFeatureLevel::SM4;
	}

	IMPLEMENT_SHADER_TYPE(, PostProcessVS, TEXT("PostProcessBloom"), TEXT("MainPostprocessCommonVS"), SF_Vertex);
}