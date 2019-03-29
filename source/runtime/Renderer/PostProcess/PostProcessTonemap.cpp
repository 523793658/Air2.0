#include "PostProcess/PostProcessTonemap.h"
#include "RHIUtilities.h"
#include "PostProcess/SceneFilterRendering.h"
#include "StaticBoundShaderState.h"
#include "GlobalShader.h"
#include "ShaderParameterUtils.h"
namespace Air
{
	typedef enum {
		TonemapperGammaOnly = (1 << 0),
	}TonemapperOption;

	static uint32 TonemapperConfBitmaskPC[1] = {
		TonemapperGammaOnly + 0
	};

	static uint32 tonemapperIsDefined(uint32 configBitmask, TonemapperOption option)
	{
		return (configBitmask & option) ? 1 : 0;
	}



	template<uint32 ConfigIndex>
	class PostProcessTonemapPS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(PostProcessTonemapPS, Global);

		static bool shouldCache(EShaderPlatform platform)
		{
			return isFeatureLevelSupported(platform, ERHIFeatureLevel::ES2);
		}

		static void modifyCompilationEnvironment(EShaderPlatform platform, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(platform, outEnvironment);

			uint32 configBitmask = TonemapperConfBitmaskPC[ConfigIndex];
			outEnvironment.setDefine(TEXT("USE_GAMMA_ONLY"), tonemapperIsDefined(configBitmask, TonemapperGammaOnly));
		}

		PostProcessTonemapPS(){}
	public:
		PostProcessPassParameters mPostprocessParameter;
		ShaderParameter mTexScale;

		ShaderParameter mInverseGamma;
	public:
		PostProcessTonemapPS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mPostprocessParameter.bind(initializer.mParameterMap);
			mInverseGamma.bind(initializer.mParameterMap, TEXT("InverseGamma"));
		}

		virtual bool serialize(Archive& ar) override
		{
			bool bShaderHasOutdatedParameters = GlobalShader::serialize(ar);
			ar << mPostprocessParameter;
			return bShaderHasOutdatedParameters;

		}

		void setPS(const RenderingCompositePassContext& context)
		{
			//const PostProcessSettings& settings = context.mView.mfianl

			const SceneViewFamily& viewFamily = *(context.mView.mFamily);
			const PixelShaderRHIParamRef shaderRHI = getPixelShader();

			GlobalShader::setParameters(context.mRHICmdList, shaderRHI, context.mView);

			{
				SamplerStateRHIParamRef filters[] =
				{
					TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp, 0, 1>::getRHI(),
					TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp, 0, 1>::getRHI(),
					TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp, 0, 1>::getRHI(),
					TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp, 0, 1>::getRHI(),
				};

				mPostprocessParameter.setPS(shaderRHI, context, 0, eFC_0000, filters);
			}
			//setShaderValue(context.mRHICmdList, shaderRHI, )
		
			{
				const PooledRenderTargetDesc* inputDesc = context.mPass->getInputDesc(ePId_Input0);
				float2 texScaleValue = float2(inputDesc->mExtent) / float2(context.mView.mViewRect.size());

				setShaderValue(context.mRHICmdList, shaderRHI, mTexScale, texScaleValue);
			}

			{
				float gamma = 2.2f;
			
				
			}

			{
				float3 invDisplayGammaValue;
				invDisplayGammaValue.x = 1.0f / viewFamily.mRenderTarget->getDisplayGamma();
				invDisplayGammaValue.y = 2.2f / viewFamily.mRenderTarget->getDisplayGamma();
				invDisplayGammaValue.z = 1.0f;

				setShaderValue(context.mRHICmdList, shaderRHI, mInverseGamma, invDisplayGammaValue);
			}
		}

		static const TCHAR* getSourceFilename()
		{
			return TEXT("PostProcessTonemap");
		}

		static const TCHAR* getFunctionName()
		{
			return TEXT("MainPS");
		}

	};

#define VARIATION1(A) typedef PostProcessTonemapPS<A> PostProcessTonemapPS##A; \
	IMPLEMENT_SHADER_TYPE2(PostProcessTonemapPS##A, SF_Pixel);

		VARIATION1(0)
		VARIATION1(1)
		VARIATION1(2)
		VARIATION1(3)
		VARIATION1(4)
		VARIATION1(5)
		VARIATION1(6)
		VARIATION1(7)
		VARIATION1(8)
		VARIATION1(9)
		VARIATION1(10)
		VARIATION1(11)
		VARIATION1(12)
		VARIATION1(13)
		VARIATION1(14)
#undef VARIATION1

	namespace PostProcessTonemapUtil
	{
		template<uint32 ConfigIndex, bool bDoEyeAdaptation>
		static inline void setShaderTempl(const RenderingCompositePassContext& context)
		{
			typedef TPostProcessTonemapVS<bDoEyeAdaptation> VertexShaderType;
			typedef PostProcessTonemapPS<ConfigIndex> PixelShaderType;

			TShaderMapRef<PixelShaderType> pixelShader(context.getShaderMap());
			TShaderMapRef<VertexShaderType> vertexShader(context.getShaderMap());

			static GlobalBoundShaderState boundShaderState;

			setGlobalBoundShaderState(context.mRHICmdList, context.getFeatureLevel(), boundShaderState, GFilterVertexDeclaration.mVertexDeclarationRHI, *vertexShader, *pixelShader);

			vertexShader->setVS(context);
			pixelShader->setPS(context);
		}

		template<uint32 ConfigIndex>
		static inline void setShaderTempl(const RenderingCompositePassContext& context, bool bDoEyeAdaptation)
		{
			if (bDoEyeAdaptation)
			{
				setShaderTempl<ConfigIndex, true>(context);
			}
			else
			{
				setShaderTempl<ConfigIndex, false>(context);
			}
		}
	}


	PooledRenderTargetDesc RCPassPostProcessTonemap::computeOutputDesc(EPassOutputId inPassOutputId) const
	{
		PooledRenderTargetDesc ret = getInput(ePId_Input0)->getOutput()->mRenderTargetDesc;
		ret.reset();
		ret.mFormat = bHDROutput ? GRHIHDRDisplayOutputFormat : PF_B8G8R8A8;
		ret.mDebugName = TEXT("Tonemap");
		ret.mClearValue = ClearValueBinding(LinearColor(0, 0, 0, 0));
		return ret;
	}

	RCPassPostProcessTonemap::RCPassPostProcessTonemap(const ViewInfo& inView, bool bInDoGammaOnly, bool binDoEyeAdaptation, bool bInHDROutput)
		:bDoGammaOnly(bInDoGammaOnly)
		,bDoScreenPercentageInTonemapper(false)
		,bDoEyeAdaptation(binDoEyeAdaptation)
		,bHDROutput(bInHDROutput)
		,mView(inView)
	{
		mConfigIndexPC = 0;
	}

	void RCPassPostProcessTonemap::process(RenderingCompositePassContext& context)
	{
		const PooledRenderTargetDesc* inputDesc = getInputDesc(ePId_Input0);
		if (!inputDesc)
		{
			return;
		}

		const SceneViewFamily& viewFamily = *(mView.mFamily);
		IntRect srcRect = mView.mViewRect;

		IntRect destRect = bDoScreenPercentageInTonemapper ? mView.mUnscaledViewRect : mView.mViewRect;

		int2 srcSize = inputDesc->mExtent;

		const SceneRenderTargetItem& destRenderTarget = mPassOutputs[0].requestSurface(context);

		const EShaderPlatform shaderPlatform = GShaderPlatformForFeatureLevel[context.getFeatureLevel()];

		setRenderTarget(context.mRHICmdList, destRenderTarget.mTargetableTexture, TextureRHIParamRef(), ESimpleRenderTargetMode::EUninitializedColorAndDepth);
		if (viewFamily.mRenderTarget->getRenderTargetTexture() != destRenderTarget.mTargetableTexture)
		{
			context.mRHICmdList.clearColorTexture(destRenderTarget.mTargetableTexture, LinearColor::Black, destRect);
		}

		context.setViewportAndCallRHI(destRect, 0.0f, 1.0f);

		context.mRHICmdList.setBlendState(TStaticBlendState<>::getRHI());
		context.mRHICmdList.setRasterizerState(TStaticRasterizerState<>::getRHI());
		context.mRHICmdList.setDepthStencilState(TStaticDepthStencilState<false, CF_Always>::getRHI());
		switch (mConfigIndexPC)
		{
			using namespace PostProcessTonemapUtil;
		case 0: setShaderTempl<0>(context, bDoEyeAdaptation); break;
		case 1: setShaderTempl<0>(context, bDoEyeAdaptation); break;
		case 2: setShaderTempl<0>(context, bDoEyeAdaptation); break;
		case 3: setShaderTempl<0>(context, bDoEyeAdaptation); break;
		case 4: setShaderTempl<0>(context, bDoEyeAdaptation); break;
		case 5: setShaderTempl<0>(context, bDoEyeAdaptation); break;
		case 6: setShaderTempl<0>(context, bDoEyeAdaptation); break;
		case 7: setShaderTempl<0>(context, bDoEyeAdaptation); break;
		case 8: setShaderTempl<0>(context, bDoEyeAdaptation); break;
		case 9: setShaderTempl<0>(context, bDoEyeAdaptation); break;
		case 10: setShaderTempl<0>(context, bDoEyeAdaptation); break;
		case 11: setShaderTempl<0>(context, bDoEyeAdaptation); break;
		case 12: setShaderTempl<0>(context, bDoEyeAdaptation); break;
		case 13: setShaderTempl<0>(context, bDoEyeAdaptation); break;
		case 14: setShaderTempl<0>(context, bDoEyeAdaptation); break;
		default:
			BOOST_ASSERT(false);
		}

		SceneRenderTargets& sceneContext = SceneRenderTargets::get(context.mRHICmdList);
		Shader* vertexShader;
		if (bDoEyeAdaptation)
		{

		}
		else
		{
			TShaderMapRef<TPostProcessTonemapVS<false>> vertexShaderMapRef(context.getShaderMap());
			vertexShader = *vertexShaderMapRef;
		}

		drawPostProcessPass(
			context.mRHICmdList,
			0, 0,
			destRect.width(), destRect.height(),
			mView.mViewRect.min.x, mView.mViewRect.min.y,
			mView.mViewRect.width(), mView.mViewRect.height(),
			destRect.size(),
			sceneContext.getBufferSizeXY(),
			vertexShader,
			mView.mStereoPass,
			context.hasHmdMesh(),
			EDRF_UseTriangleOptimization);
		context.mRHICmdList.copyToResolveTarget(destRenderTarget.mTargetableTexture, destRenderTarget.mShaderResourceTexture, false, ResolveParams());

		if (context.mView.mFamily->mViews[context.mView.mFamily->mViews.size() - 1] == &context.mView && !GIsEditor)
		{
			sceneContext.setSceneColor(nullptr);
		}
	}

	IMPLEMENT_SHADER_TYPE(template<>, TPostProcessTonemapVS<true>, TEXT("PostProcessTonemap"), TEXT("MainVS"), SF_Vertex);
	IMPLEMENT_SHADER_TYPE(template<>, TPostProcessTonemapVS<false>, TEXT("PostProcessTonemap"), TEXT("MainVS"), SF_Vertex);


}