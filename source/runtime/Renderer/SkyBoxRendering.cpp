#include "DeferredShadingRenderer.h"
#include "LightRendering.h"
#include "GlobalShader.h"
#include "StaticBoundShaderState.h"
#include "PostProcess/SceneFilterRendering.h"
#include "TextureResource.h"
namespace Air
{
	class SkyBoxVS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(SkyBoxVS, Global);
	public:
		SkyBoxVS()
		{

		}
		SkyBoxVS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
		}
		static bool shouldCache(EShaderPlatform platform)
		{
			return true;
		}
	};

	class SkyBoxPS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(SkyBoxPS, Global);
	public:
		SkyBoxPS()
		{

		}
		SkyBoxPS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
		}
		static bool shouldCache(EShaderPlatform platform)
		{
			return true;
		}

		GlobalBoundShaderState& getBoundShaderState()
		{
			static GlobalBoundShaderState state;
			return state;
		}
	};

	IMPLEMENT_SHADER_TYPE(, SkyBoxVS, TEXT("SkyBox"), TEXT("MainVS"), SF_Vertex);
	IMPLEMENT_SHADER_TYPE(, SkyBoxPS, TEXT("SkyBox"), TEXT("MainPS"),
		SF_Pixel);

	void DeferredShadingSceneRenderer::renderSky(RHICommandList& RHICmdList)
	{
		RHICmdList.setBlendState(TStaticBlendState<CW_RGB, BO_Add, BF_One, BF_Zero, BO_Add, BF_Zero, BF_Zero>::getRHI());

		bool bStencilDirty = false;
		for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
		{
			ViewInfo& view = mViews[viewIndex];
			TShaderMapRef<SkyBoxVS> vertexShader(view.mShaderMap);
			RHICmdList.setViewport(view.mViewRect.min.x, view.mViewRect.min.y, 0.0f, view.mViewRect.max.x, view.mViewRect.max.y, 1.0f);
			RHICmdList.setRasterizerState(TStaticRasterizerState<FM_Solid, CM_None>::getRHI());
			RHICmdList.setDepthStencilState(TStaticDepthStencilState<true
				, CF_LessEqual>::getRHI());
			TShaderMapRef<SkyBoxPS> pixelShader(view.mShaderMap);
			setGlobalBoundShaderState(RHICmdList, view.getFeatureLevel(), pixelShader->getBoundShaderState(), getVertexDeclarationVector4(), *vertexShader, *pixelShader);
			pixelShader->setParameters(RHICmdList, pixelShader->getPixelShader(), view);
			drawRectangle(RHICmdList, 0, 0, view.mViewRect.width(), view.mViewRect.height(), view.mViewRect.min.x, view.mViewRect.min.y, view.mViewRect.width(), view.mViewRect.height(), view.mViewRect.size(), SceneRenderTargets::get(RHICmdList).getBufferSizeXY(), *vertexShader, EDRF_UseTriangleOptimization);
		}

	}
}