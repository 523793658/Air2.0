#include "PostProcess/PostProcessTonemap.h"
#include "PostProcess/SceneFilterRendering.h"
#include "StaticBoundShaderState.h"
#include "GlobalShader.h"
#include "ShaderParameterUtils.h"
#include "PostProcessCombineLUTs.h"
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

	namespace TonemapperPermutation
	{
		SHADER_PERMUTATION_BOOL(TonemapperBloomDim, "USE_BLOOM");
		SHADER_PERMUTATION_BOOL(TonemapperGammaOnlyDim, "USE_GAMMA_ONLY");


		using CommonDomain = TShaderPermutationDomain<
			TonemapperBloomDim,
			TonemapperGammaOnlyDim>;


		FORCEINLINE_DEBUGGABLE bool shouldCompileCommonPermutation(const GlobalShaderPermutationParameters& parameters, const CommonDomain& permutationVector)
		{
			if (permutationVector.get<TonemapperGammaOnlyDim>())
			{
				return !permutationVector.get<TonemapperBloomDim>();
			}
			return true;
		}

		SHADER_PERMUTATION_BOOL(TonemapperColorFringeDim, "USE_COLOR_FRINGE");

		using DesktopDomain = TShaderPermutationDomain<CommonDomain,
			TonemapperColorFringeDim>;

		DesktopDomain remapPermutation(DesktopDomain permutationVector, ERHIFeatureLevel::Type featureLevel)
		{
			CommonDomain commonPermutationVector = permutationVector.get<CommonDomain>();
			if (commonPermutationVector.get<TonemapperGammaOnlyDim>())
			{
				return permutationVector;
			}

			permutationVector.set<CommonDomain>(commonPermutationVector);
			return permutationVector;
		}

		bool shouldCompleDesktopPermutation(const GlobalShaderPermutationParameters& parameters, DesktopDomain permutationVector)
		{
			auto commonPermutationVector = permutationVector.get<CommonDomain>();

			if (remapPermutation(permutationVector, getMaxSupportedFeatureLevel(parameters.mPlatform)) != permutationVector)
			{
				return false;
			}

			if (!shouldCompileCommonPermutation(parameters, commonPermutationVector))
			{
				return false;
			}

			if (commonPermutationVector.get<TonemapperGammaOnlyDim>())
			{
				return !permutationVector.get<TonemapperColorFringeDim>();
			}
			return true;
		}
	}



	class PostProcessTonemapPS : public GlobalShader
	{
		DECLARE_GLOBAL_SHADER(PostProcessTonemapPS);

		using PermutationDomain = TonemapperPermutation::DesktopDomain;

		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			if (!isFeatureLevelSupported(parameters.mPlatform, ERHIFeatureLevel::ES3_1))
			{
				return false;
			}
			return TonemapperPermutation::shouldCompleDesktopPermutation(parameters, PermutationDomain(parameters.mPermutationId));
		}

		static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			const int32 useValumeLut = pipelineVolumeTextureLUTSupportGuaranteedAtRuntime(parameters.mPlatform) ? 1 : 0;
			outEnvironment.setDefine(TEXT("USE_VOLUME_LUT"), useValumeLut);
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
			RHIPixelShader* shaderRHI = getPixelShader();

			GlobalShader::setParameters<ViewConstantShaderParameters>(context.mRHICmdList, shaderRHI, context.mView.mViewConstantBuffer);

			{
				RHISamplerState* filters[] =
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



	namespace PostProcessTonemapUtil
	{
		

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
		
	}

	IMPLEMENT_GLOBAL_SHADER(PostProcessTonemapVS, "PostProcessTonemap", "MainVS", SF_Vertex);
	IMPLEMENT_GLOBAL_SHADER(PostProcessTonemapPS, "PostProcessTonemap", "MainVS", SF_Vertex);


}