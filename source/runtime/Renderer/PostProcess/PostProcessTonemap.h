#include "CoreMinimal.h"
#include "PostProcess/RenderingCompositionGraph.h"
namespace Air
{
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

	template<bool bUseAutoExposure>
	class TPostProcessTonemapVS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(TPostProcessTonemapVS, Global);
		static bool shouldCache(EShaderPlatform platform)
		{
			return true;
		}

		TPostProcessTonemapVS() {}
	public:
		PostProcessPassParameters mPostprocessParameter;
		ShaderResourceParameter mEyeAdaptation;
		ShaderParameter mGrainRandomFull;
		ShaderParameter mFringeUVParams;

		ShaderParameter mDefaultEyeExposure;

		TPostProcessTonemapVS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mPostprocessParameter.bind(initializer.mParameterMap);

		}

		static void modifyCompilationEnvironment(EShaderPlatform platform, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(platform, outEnvironment);
			//outEnvironment.setDefine(TEXT(")
		}

		void setVS(const RenderingCompositePassContext& context)
		{
			const VertexShaderRHIParamRef shaderRHI = getVertexShader();
			GlobalShader::setParameters(context.mRHICmdList, shaderRHI, context.mView);

			mPostprocessParameter.setVS(shaderRHI, context, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::getRHI());

			setTextureParameter(context.mRHICmdList, shaderRHI, mEyeAdaptation, GWhiteTexture->mTextureRHI);

			if (!bUseAutoExposure)
			{
				float defaultEyeExposureValue = 1.0f;
				setShaderValue(context.mRHICmdList, shaderRHI, mDefaultEyeExposure, defaultEyeExposureValue);
			}

			{
				//float offset = context.mView.mfinpos
			}
		}

		virtual bool serialize(Archive& ar) override
		{
			bool bShaderHasOutdatedParameters = GlobalShader::serialize(ar);
			ar << mPostprocessParameter << mGrainRandomFull << mEyeAdaptation << mFringeUVParams << mDefaultEyeExposure;
			return bShaderHasOutdatedParameters;
		}
	};
}