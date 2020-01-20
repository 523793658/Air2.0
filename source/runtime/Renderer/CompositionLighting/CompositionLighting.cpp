#include "CompositionLighting/CompositionLighting.h"
#include "PostProcess/RenderingCompositionGraph.h"
#include "PostProcess/PostProcessing.h"
#include "CompositionLighting/PostProcessAmbient.h"
namespace Air
{
	CompositionLighting GCompositionLighting;

	bool isAmbientCubmapPassRequired(const SceneView& view)
	{
		Scene* scene = (Scene*)view.mFamily->mScene;
		return view.mFinalPostProcessSettings.mContributingCubemaps.size() != 0 && !isAnyForwardShadingEnabled(view.getShaderPlatform());
	}



	void CompositionLighting::processAfterBasePass(RHICommandListImmediate& RHICmdList, ViewInfo& view)
	{
		BOOST_ASSERT(isInRenderingThread());
		SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);
		sceneContext.getSceneColor()->setDebugName(TEXT("SceneColor"));
		{
			MemMark mark(MemStack::get());
			RenderingCompositePassContext compositeContext(RHICmdList, view);
			PostProcessContext context(RHICmdList, compositeContext.mGraph, view);



			TRefCountPtr<IPooledRenderTarget>& sceneColor = sceneContext.getSceneColor();
			context.mFinalOutput.getOutput()->mRenderTargetDesc = sceneColor->getDesc();
			context.mFinalOutput.getOutput()->mPooledRenderTarget = sceneColor;
			compositeContext.process(context.mFinalOutput.getPass(), TEXT("CompositionLighting_AfterBasePass"));
		}
	}

	void CompositionLighting::gfxWaitForAsyncSSAO(RHICommandListImmediate& RHICmdList)
	{
		if (mAsyncSSAOFence)
		{
			RHICmdList.waitComputeFence(mAsyncSSAOFence);
			mAsyncSSAOFence = nullptr;
		}
	}
}