#include "PostProcessAmbient.h"
#include "RHIUtilities.h"
#include "RHIStaticStates.h"
namespace Air
{
	void RCPassPostProcessAmbient::process(RenderingCompositePassContext& context)
	{
		const ViewInfo& view = context.mView;
		const SceneViewFamily& viewFamily = *(view.mFamily);
		IntRect srcRect = view.mViewRect;
		IntRect destRect = view.mViewRect;
		int2 destSize = destRect.size();

		const SceneRenderTargetItem& destRenderTarget = SceneRenderTargets::get(context.mRHICmdList).getSceneColor()->getRenderTargetItem();
		setRenderTarget(context.mRHICmdList, destRenderTarget.mTargetableTexture, TextureRHIRef(), true);

		context.setViewportAndCallRHI(view.mViewRect);
		context.mRHICmdList.setBlendState(TStaticBlendState<CW_RGB, BO_Add, BF_One, BF_One, BO_Add, BF_One, BF_One>::getRHI());
		context.mRHICmdList.setRasterizerState(TStaticRasterizerState<>::getRHI());
		context.mRHICmdList.setDepthStencilState(TStaticDepthStencilState<false, CF_Always>::getRHI());

		Scene* scene = viewFamily.mScene->getRenderScene();
		BOOST_ASSERT(scene);
		//uint32 mNumReflectionCapture = scene->mrefl
	}

	PooledRenderTargetDesc RCPassPostProcessAmbient::computeOutputDesc(EPassOutputId inPassOutputId) const
	{
		return PooledRenderTargetDesc();
	}
}