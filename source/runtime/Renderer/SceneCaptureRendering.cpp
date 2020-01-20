#include "DeferredShadingRenderer.h"
#include "RHIStaticStates.h"
#include "ScreenRendering.h"
#include "PostProcess/SceneFilterRendering.h"
#include "Rendering/SceneRenderTargetParameters.h"
namespace Air
{
	const TCHAR* GShaderSourceModeDefineName[] = {
		TEXT("SOURCE_MODE_SCENE_COLOR_AND_OPACITY"),
		TEXT("SOURCE_MODE_SCENE_COLOR_NO_ALPHA"),
		nullptr,
		TEXT("SOURCE_MODE_SCENE_COLOR_SCENE_DEPTH"),
		TEXT("SOURCE_MODE_SCENE_DEPTH"),
		TEXT("SOURCE_MODE_DEVICE_DEPTH"),
		TEXT("SOURCE_MODE_NORMAL"),
		TEXT("SOURCE_MODE_BASE_COLOR"),
		nullptr };


	template<ESceneCaptureSource CaptureSource>
	class TSceneCapturePS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(TSceneCapturePS, Global);
	public:
		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			return isFeatureLevelSupported(parameters.mPlatform, ERHIFeatureLevel::SM4);
		}

		static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			const TCHAR* defineName = GShaderSourceModeDefineName[CaptureSource];
			if (defineName)
			{
				outEnvironment.setDefine(defineName, 1);
			}
		}

		TSceneCapturePS(const ShaderMetaType::CompiledShaderInitializerType& initializer) :
			GlobalShader(initializer)
		{
			mSceneTextureParameters.bind(initializer);
		}

		TSceneCapturePS() {}

		void setParameters(RHICommandList& RHICmdList, const SceneView& view)
		{
			GlobalShader::setParameters<ViewConstantShaderParameters>(RHICmdList, getPixelShader(), view.mViewConstantBuffer);
			mSceneTextureParameters.set(RHICmdList, getPixelShader(), view.mFeatureLevel, ESceneTextureSetupMode::All);
		}

		virtual bool serialize(Archive& ar) override
		{
			bool bShaderHasOutdataParameters = GlobalShader::serialize(ar);
			ar << mSceneTextureParameters;
			return bShaderHasOutdataParameters;

		}

	private:
		SceneTextureShaderParameters mSceneTextureParameters;
	};

	IMPLEMENT_SHADER_TYPE(template<>, TSceneCapturePS<SCS_SceneColorHDR>, TEXT("SceneCapturePixelShader.hlsl"), TEXT("Main"), SF_Pixel);
	IMPLEMENT_SHADER_TYPE(template<>, TSceneCapturePS<SCS_SceneColorHDRNoAlpha>, TEXT("SceneCapturePixelShader.hlsl"), TEXT("Main"), SF_Pixel);
	IMPLEMENT_SHADER_TYPE(template<>, TSceneCapturePS<SCS_SceneColorSceneDepth>, TEXT("SceneCapturePixelShader.hlsl"), TEXT("Main"), SF_Pixel);
	IMPLEMENT_SHADER_TYPE(template<>, TSceneCapturePS<SCS_SceneDepth>, TEXT("SceneCapturePixelShader.hlsl"), TEXT("Main"), SF_Pixel);
	IMPLEMENT_SHADER_TYPE(template<>, TSceneCapturePS<SCS_DeviceDepth>, TEXT("SceneCapturePixelShader.hlsl"), TEXT("Main"), SF_Pixel);
	IMPLEMENT_SHADER_TYPE(template<>, TSceneCapturePS<SCS_Normal>, TEXT("SceneCapturePixelShader.hlsl"), TEXT("Main"), SF_Pixel);
	IMPLEMENT_SHADER_TYPE(template<>, TSceneCapturePS<SCS_BaseColor>, TEXT("SceneCapturePixelShader.hlsl"), TEXT("Main"), SF_Pixel);


	void DeferredShadingSceneRenderer::copySceneCaptureComponentToTarget(RHICommandListImmediate& RHICmdList)
	{
		ESceneCaptureSource sceneCaptureSource = mViewFamily.mSceneCaptureSource;

		if (isAnyForwardShadingEnabled(mViewFamily.getShaderPlatform()) && (sceneCaptureSource == SCS_Normal || sceneCaptureSource == SCS_BaseColor))
		{
			sceneCaptureSource = SCS_SceneColorHDR;
		}
		if (sceneCaptureSource != SCS_FinalColorLDR && sceneCaptureSource != SCS_FinalColorHDR)
		{
			GraphicsPipelineStateInitializer graphicsPSOInit;
			graphicsPSOInit.mRasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::getRHI();
			graphicsPSOInit.mDepthStencilState = TStaticDepthStencilState<false, CF_Always>::getRHI();

			for (int32 viewIndex = 0; viewIndex < mViews.size(); viewIndex++)
			{
				ViewInfo& view = mViews[viewIndex];
				RHIRenderPassInfo RPInfo(mViewFamily.mRenderTarget->getRenderTargetTexture(), ERenderTargetActions::DontLoad_Store);
				RHICmdList.beginRenderPass(RPInfo, TEXT("ViewCapture"));
				{
					RHICmdList.applyCachedRenderTargets(graphicsPSOInit);
					if (sceneCaptureSource == SCS_SceneColorHDR && mViewFamily.mSceneCaptureCompositeMode == SCCM_Composite)
					{
						graphicsPSOInit.mBlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_SourceAlpha, BO_Add, BF_Zero, BF_SourceAlpha>::getRHI();
					}
					else if (sceneCaptureSource == SCS_SceneColorHDR && mViewFamily.mSceneCaptureCompositeMode == SCCM_Additive)
					{
						graphicsPSOInit.mBlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_Zero, BF_SourceAlpha>::getRHI();
					}
					else
					{
						graphicsPSOInit.mBlendState = TStaticBlendState<>::getRHI();
					}
					TShaderMapRef<ScreenVS> vertexShader(view.mShaderMap);
					graphicsPSOInit.mBoundShaderState.mVertexDeclarationRHI = GFilterVertexDeclaration.mVertexDeclarationRHI;
					graphicsPSOInit.mBoundShaderState.mVertexShaderRHI = GETSAFERHISHADER_VERTEX(*vertexShader);
					graphicsPSOInit.mPrimitiveType = PT_TriangleList;

					if (sceneCaptureSource == SCS_FinalColorHDR)
					{
						TShaderMapRef<TSceneCapturePS<SCS_SceneColorHDR>> pixelShader(view.mShaderMap);
						graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(*pixelShader);
						setGraphicsPipelineState(RHICmdList, graphicsPSOInit);
						pixelShader->setParameters(RHICmdList, view);
					}
					else if (sceneCaptureSource == SCS_SceneColorHDRNoAlpha)
					{
						TShaderMapRef<TSceneCapturePS<SCS_SceneColorHDRNoAlpha>> pixelShader(view.mShaderMap);
						graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(*pixelShader);
						setGraphicsPipelineState(RHICmdList, graphicsPSOInit);
						pixelShader->setParameters(RHICmdList, view);
					}
					else if (sceneCaptureSource == SCS_SceneColorSceneDepth)
					{
						TShaderMapRef<TSceneCapturePS<SCS_SceneColorSceneDepth>> pixelShader(view.mShaderMap);
						graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(*pixelShader);
						setGraphicsPipelineState(RHICmdList, graphicsPSOInit);
						pixelShader->setParameters(RHICmdList, view);
					}
					else if (sceneCaptureSource == SCS_SceneDepth)
					{
						TShaderMapRef<TSceneCapturePS<SCS_SceneDepth>> pixelShader(view.mShaderMap);
						graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(*pixelShader);
						setGraphicsPipelineState(RHICmdList, graphicsPSOInit);
						pixelShader->setParameters(RHICmdList, view);
					}
					else if (sceneCaptureSource == SCS_DeviceDepth)
					{
						TShaderMapRef<TSceneCapturePS<SCS_DeviceDepth>> pixelShader(view.mShaderMap);
						graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(*pixelShader);
						setGraphicsPipelineState(RHICmdList, graphicsPSOInit);
						pixelShader->setParameters(RHICmdList, view);
					}
					else if (sceneCaptureSource == SCS_Normal)
					{
						TShaderMapRef<TSceneCapturePS<SCS_Normal>> pixelShader(view.mShaderMap);
						graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(*pixelShader);
						setGraphicsPipelineState(RHICmdList, graphicsPSOInit);
						pixelShader->setParameters(RHICmdList, view);
					}
					else if (sceneCaptureSource == SCS_BaseColor)
					{
						TShaderMapRef<TSceneCapturePS<SCS_BaseColor>> pixelShader(view.mShaderMap);
						graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(*pixelShader);
						setGraphicsPipelineState(RHICmdList, graphicsPSOInit);
						pixelShader->setParameters(RHICmdList, view);
					}
					else
					{
						BOOST_ASSERT(false);
					}
					vertexShader->setParameters(RHICmdList, view.mViewConstantBuffer);
					drawRectangle(RHICmdList,
						view.mViewRect.min.x, view.mViewRect.min.y,
						view.mViewRect.width(), view.mViewRect.height(),
						view.mViewRect.min.x, view.mViewRect.min.y,
						view.mViewRect.width(), view.mViewRect.height(),
						view.mUnconstrainedViewRect.size(),
						SceneRenderTargets::get(RHICmdList).getBufferSizeXY(),
						*vertexShader,
						EDRF_UseTriangleOptimization
					);
				}
				RHICmdList.endRenderPass();
			}
		}
	}
}