#include "DeferredShadingRenderer.h"
#include "LightRendering.h"
#include "GlobalShader.h"
#include "StaticBoundShaderState.h"
#include "PostProcess/SceneFilterRendering.h"
#include "TextureResource.h"
#include "RHIStaticStates.h"
namespace Air
{
	class SkyBoxVS : public GlobalShader
	{
		DECLARE_GLOBAL_SHADER(SkyBoxVS);
	public:
		SkyBoxVS()
		{

		}
		SkyBoxVS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
		}
		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			return true;
		}
	};

	class SkyBoxPS : public GlobalShader
	{
		DECLARE_GLOBAL_SHADER(SkyBoxPS);
	public:
		SkyBoxPS()
		{

		}
		SkyBoxPS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
		}
		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			return true;
		}

		GlobalBoundShaderState& getBoundShaderState()
		{
			static GlobalBoundShaderState state;
			return state;
		}
	};

	IMPLEMENT_GLOBAL_SHADER(SkyBoxVS, "SkyBox", "MainVS", SF_Vertex);

	IMPLEMENT_GLOBAL_SHADER(SkyBoxPS, "SkyBox", "MainPS", SF_Pixel);

	void DeferredShadingSceneRenderer::renderSky(RHICommandList& RHICmdList)
	{
		GraphicsPipelineStateInitializer graphicsPSOInit;
		graphicsPSOInit.mBlendState = TStaticBlendState<CW_RGB, BO_Add, BF_One, BF_Zero, BO_Add, BF_Zero, BF_Zero>::getRHI();
		graphicsPSOInit.mRasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::getRHI();
		graphicsPSOInit.mDepthStencilState = TStaticDepthStencilState<true
			, CF_LessEqual>::getRHI();
		bool bStencilDirty = false;
		for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
		{
			ViewInfo& view = mViews[viewIndex];
			TShaderMapRef<SkyBoxVS> vertexShader(view.mShaderMap);
			RHICmdList.setViewport(view.mViewRect.min.x, view.mViewRect.min.y, 0.0f, view.mViewRect.max.x, view.mViewRect.max.y, 1.0f);
			setGraphicsPipelineState(RHICmdList, graphicsPSOInit);
			TShaderMapRef<SkyBoxPS> pixelShader(view.mShaderMap);
			
			//pixelShader->setParameters(RHICmdList, pixelShader->getPixelShader(), view);
			//drawRectangle(RHICmdList, 0, 0, view.mViewRect.width(), view.mViewRect.height(), view.mViewRect.min.x, view.mViewRect.min.y, view.mViewRect.width(), view.mViewRect.height(), view.mViewRect.size(), SceneRenderTargets::get(RHICmdList).getBufferSizeXY(), *vertexShader, EDRF_UseTriangleOptimization);
		}

	}
}