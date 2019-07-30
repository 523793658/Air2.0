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

	static void addPostProcessingAmbientCubemap(PostProcessContext& context, RenderingCompositeOutputRef ambientOcclusion)
	{
		RenderingCompositePass* pass = context.mGraph.registerPass(new(MemStack::get())RCPassPostProcessAmbient());
		pass->setInput(ePId_Input0, context.mFinalOutput);
		pass->setInput(ePId_Input1, ambientOcclusion);

		context.mFinalOutput = RenderingCompositeOutputRef(pass);
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



			if (isAmbientCubmapPassRequired(context.mView))
			{
				addPostProcessingAmbientCubemap(context, nullptr);
			}

			TRefCountPtr<IPooledRenderTarget>& sceneColor = sceneContext.getSceneColor();
			context.mFinalOutput.getOutput()->mRenderTargetDesc = sceneColor->getDesc();
			context.mFinalOutput.getOutput()->mPooledRenderTarget = sceneColor;
			compositeContext.process(context.mFinalOutput.getPass(), TEXT("CompositionLighting_AfterBasePass"));
		}
	}
}