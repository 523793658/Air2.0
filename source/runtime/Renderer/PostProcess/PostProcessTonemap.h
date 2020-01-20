#include "CoreMinimal.h"
#include "ShaderPermutation.h"
#include "ShaderParameterUtils.h"
#include "PostProcess/RenderingCompositionGraph.h"
namespace Air
{
	static float grainHalton(int32 index, int32 base)
	{
		float result = 0.0f;
		float invBase = 1.0f / base;
		float fraction = invBase;
		while (index > 0)
		{
			result += (index % base) * fraction;
			index /= base;
			fraction *= invBase;
		}
		return result;
	}

	static void grainRandomFromFrame(float3* RESTRICT const constant, uint32 frameNumber)
	{
		constant->x = grainHalton(frameNumber & 1023, 2);
		constant->y = grainHalton(frameNumber % 1023, 3);
	}

	class RCPassPostProcessTonemap : public TRenderingCompositePassBase<4, 1>
	{
	public:

		RCPassPostProcessTonemap(const ViewInfo& inView, bool bInDoGammaOnly, bool bDoEyeAdaptation, bool bHDROutput);

		bool bDoGammaOnly;

		bool bDoScreenPercentageInTonemapper;

		virtual void release() override { delete this; }

		virtual void process(RenderingCompositePassContext& context) override;

		virtual PooledRenderTargetDesc computeOutputDesc(EPassOutputId inPassOutputId) const override;
	private:
		uint32 mConfigIndexPC;

		const ViewInfo& mView;

		bool bHDROutput;

		bool bDoEyeAdaptation{ false };

		void setShader(const RenderingCompositePassContext& context);
	};

	class PostProcessTonemapVS : public GlobalShader
	{
		DECLARE_GLOBAL_SHADER(PostProcessTonemapVS);
	
		SHADER_PERMUTATION_BOOL(TonemapperVSSwitchAxis, "NEEDTOSWITCHVERTICLEAXIS");
		SHADER_PERMUTATION_BOOL(TonemapperVSUseAutoExposure, "EYEADAPTATION_EXPOSURE_FIX");

		using PermutationDomain = TShaderPermutationDomain<TonemapperVSSwitchAxis, TonemapperVSUseAutoExposure>;


		
		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			PermutationDomain permutationVector(parameters.mPermutationId);
			if (permutationVector.get<TonemapperVSSwitchAxis>() && !RHINeedsToSwitchVerticalAxis(parameters.mPlatform))
			{
				return false;
			}

			return true;
		}

		PostProcessTonemapVS() {}
	public:
		PostProcessPassParameters mPostprocessParameter;
		ShaderResourceParameter mEyeAdaptation;
		ShaderParameter mGrainRandomFull;
		ShaderParameter mScreenPosToScenePixel;
		ShaderParameter mDefaultEyeExposure;

		PostProcessTonemapVS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mPostprocessParameter.bind(initializer.mParameterMap);
			mEyeAdaptation.bind(initializer.mParameterMap, TEXT("EyeAdaptation"));
			mGrainRandomFull.bind(initializer.mParameterMap, TEXT("GrainRandomFull"));
			mDefaultEyeExposure.bind(initializer.mParameterMap, TEXT("DefaultEyeExposure"));
			mScreenPosToScenePixel.bind(initializer.mParameterMap, TEXT("ScreenPosToScenePixel"));
		}

		static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(parameters, outEnvironment);
			//outEnvironment.setDefine(TEXT(")
		}

		void setVS(const RenderingCompositePassContext& context, const PermutationDomain& permutationVector)
		{
			RHIVertexShader* shaderRHI = getVertexShader();
			GlobalShader::setParameters<ViewConstantShaderParameters>(context.mRHICmdList, shaderRHI, context.mView.mViewConstantBuffer);

			mPostprocessParameter.setVS(shaderRHI, context, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::getRHI());

			float3 grainRandomFullValue;
			{
				uint8 frameIndexMod8 = 0;
				if (context.mView.mState)
				{
					frameIndexMod8 = context.mView.mViewState->getFrameIndex(8);
				}
				grainRandomFromFrame(&grainRandomFullValue, frameIndexMod8);
			}

			setShaderValue(context.mRHICmdList, shaderRHI, mGrainRandomFull, grainRandomFullValue);

			if (false)
			{

			}
			else
			{
				setTextureParameter(context.mRHICmdList, shaderRHI, mEyeAdaptation, GWhiteTexture->mTextureRHI);
			}
			/*if (!permutationVector.get<TonemapperVSUseAutoExposure>())
			{
				float fixedExposure = RCPassPostProcessEyeAdaptation::
			}*/
			{
				int2 viewportOffset = context.mSceneColorViewRect.min;
				int2 viewportExtent = context.mSceneColorViewRect.size();
				float4 screenPosToScenePixelValue(
					viewportExtent.x * 0.5f,
					-viewportExtent.y * 0.5f,
					viewportExtent.x * 0.5f - 0.5f + viewportOffset.x,
					viewportExtent.y * 0.5f - 0.5f + viewportOffset.y);
				setShaderValue(context.mRHICmdList, shaderRHI, mScreenPosToScenePixel, screenPosToScenePixelValue);
			}
		}

		virtual bool serialize(Archive& ar) override
		{
			bool bShaderHasOutdatedParameters = GlobalShader::serialize(ar);
			ar << mPostprocessParameter << mGrainRandomFull << mEyeAdaptation << mDefaultEyeExposure << mScreenPosToScenePixel;
			return bShaderHasOutdatedParameters;
		}
	};
}