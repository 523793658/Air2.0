#include "ReflectionEnvironmentCapture.h"
#include "GlobalShader.h"
#include "sceneRendering.h"
#include "RHIUtilities.h"
#include "RHIStaticStates.h"
#include "ShaderParameterUtils.h"
#include "ScreenRendering.h"
#include "StaticBoundShaderState.h"
#include "PostProcess/SceneFilterRendering.h"
namespace Air
{



	extern int32 GDiffuseIrradianceCubemapSize;

	class CopyDiffuseIrradiancePS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(CopyDiffuseIrradiancePS, Global);
	public:
		static bool shouldCache(EShaderPlatform platform)
		{
			return true;
		}

		CopyDiffuseIrradiancePS(const ShaderMetaType::CompiledShaderInitializerType& initializer) :
			GlobalShader(initializer)
		{
			mCubeFace.bind(initializer.mParameterMap, TEXT("CubeFace"));
			mSourceMipIndex.bind(initializer.mParameterMap, TEXT("SourceMipIndex"));
			mSourceTexture.bind(initializer.mParameterMap, TEXT("SourceTexture"));
			mSourceTextureSampler.bind(initializer.mParameterMap, TEXT("SourceTextureSampler"));
			mCoefficientMask0.bind(initializer.mParameterMap, TEXT("CoefficientMask0"));
			mCoefficientMask1.bind(initializer.mParameterMap, TEXT("CoefficientMask1"));
			mCoefficientMask2.bind(initializer.mParameterMap, TEXT("CoefficientMask2"));
			mNumSamples.bind(initializer.mParameterMap, TEXT("NumSamples"));
		}

		CopyDiffuseIrradiancePS() {}

		void setParameters(RHICommandList& RHICmdList, int32 cubeFaceValue, int32 sourceMipIndexValue, int32 ceofficientIndex, int32 faceResolution, TextureRHIRef& sourceTextureValue)
		{
			setShaderValue(RHICmdList, getPixelShader(), mCubeFace, cubeFaceValue);
			setShaderValue(RHICmdList, getPixelShader(), mSourceMipIndex, sourceMipIndexValue);
			setTextureParameter(RHICmdList, getPixelShader(), mSourceTexture, mSourceTextureSampler, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::getRHI(),
				sourceTextureValue);

			const float4 mask0(ceofficientIndex == 0, ceofficientIndex == 1, ceofficientIndex == 2, ceofficientIndex == 3);
			const float4 mask1(ceofficientIndex == 4, ceofficientIndex == 5, ceofficientIndex == 6, ceofficientIndex == 7);
			const float mask2 = ceofficientIndex == 8;
			setShaderValue(RHICmdList, getPixelShader(), mCoefficientMask0, mask0);
			setShaderValue(RHICmdList, getPixelShader(), mCoefficientMask1, mask1);
			setShaderValue(RHICmdList, getPixelShader(), mCoefficientMask2, mask2);
			setShaderValue(RHICmdList, getPixelShader(), mNumSamples, faceResolution * faceResolution * 6);
		}

		virtual bool serialize(Archive& ar) override
		{
			bool b = GlobalShader::serialize(ar);

			ar << mCubeFace;
			ar << mSourceMipIndex;
			ar << mSourceTexture;
			ar << mSourceTextureSampler;
			ar << mCoefficientMask0;
			ar << mCoefficientMask1;
			ar << mCoefficientMask2;
			ar << mNumSamples;
			return b;
		}

	private:
		ShaderParameter mCubeFace;
		ShaderParameter mSourceMipIndex;
		ShaderResourceParameter mSourceTexture;
		ShaderResourceParameter mSourceTextureSampler;
		ShaderParameter mCoefficientMask0;
		ShaderParameter mCoefficientMask1;
		ShaderParameter mCoefficientMask2;
		ShaderParameter mNumSamples;
	};

	IMPLEMENT_SHADER_TYPE(, CopyDiffuseIrradiancePS, TEXT("ReflectionEnvironmentShaders"), TEXT("DiffuseIrradianceCopyPS"), SF_Pixel);

	class AccumulateDiffuseIrradiancePS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(AccumulateDiffuseIrradiancePS, Global);
	public:
		static bool shouldCache(EShaderPlatform platform)
		{
			return true;
		}

		AccumulateDiffuseIrradiancePS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mCubeFace.bind(initializer.mParameterMap, TEXT("CubeFace"));
			mSourceMipIndex.bind(initializer.mParameterMap, TEXT("SourceMipIndex"));
			mSourceTexture.bind(initializer.mParameterMap, TEXT("SourceTexture"));
			mSourceTextureSampler.bind(initializer.mParameterMap, TEXT("SourceTextureSampler"));
			mSample01.bind(initializer.mParameterMap, TEXT("Sample01"));
			mSample23.bind(initializer.mParameterMap, TEXT("Sample23"));
		}

		AccumulateDiffuseIrradiancePS() {}

		static void modifyCompilationEnvironment(EShaderPlatform platform, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(platform, outEnvironment);
		}

		void setParameters(RHICommandList& RHICmdList, int32 cubefaceValue, int32 numMips, int32 sourceMipIndexValue, int32 coefficientIndex, TextureRHIRef& sourceTextureValue)
		{
			setShaderValue(RHICmdList, getPixelShader(), mCubeFace, cubefaceValue);
			setShaderValue(RHICmdList, getPixelShader(), mSourceMipIndex, sourceMipIndexValue);
			setTextureParameter(RHICmdList,
				getPixelShader(),
				mSourceTexture,
				mSourceTextureSampler,
				TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::getRHI(),
				sourceTextureValue);

			const int32 mipSize = 1 << (numMips - sourceMipIndexValue - 1);
			const float halfSourceTexelSize = 0.5f / mipSize;
			const float4 sample01Value(-halfSourceTexelSize, -halfSourceTexelSize, halfSourceTexelSize, halfSourceTexelSize);
			const float4 sample23Value(-halfSourceTexelSize, halfSourceTexelSize, halfSourceTexelSize, halfSourceTexelSize);
			setShaderValue(RHICmdList, getPixelShader(), mSample01, sample01Value);
			setShaderValue(RHICmdList, getPixelShader(), mSample23, sample23Value);
		}

		virtual bool serialize(Archive& ar) override
		{
			bool b = GlobalShader::serialize(ar);
			ar << mCubeFace;
			ar << mSourceMipIndex;
			ar << mSourceTexture;
			ar << mSourceTextureSampler;
			ar << mSample01;
			ar << mSample23;
			return b;
		}



	private:
		ShaderParameter mCubeFace;
		ShaderParameter mSourceMipIndex;
		ShaderResourceParameter mSourceTexture;
		ShaderResourceParameter mSourceTextureSampler;
		ShaderParameter mSample01;
		ShaderParameter mSample23;
	};

	IMPLEMENT_SHADER_TYPE(, AccumulateDiffuseIrradiancePS, TEXT("ReflectionEnvironmentShaders"), TEXT("DiffuseIrradianceAccumulatePS"), SF_Pixel);

	class AccumulateCubeFacesPS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(AccumulateCubeFacesPS, Global);
	public:
		static bool shouldCache(EShaderPlatform platform)
		{
			return true;
		}

		AccumulateCubeFacesPS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mSourceMipIndex.bind(initializer.mParameterMap, TEXT("SourceMipIndex"));
			mSourceTexture.bind(initializer.mParameterMap, TEXT("SourceTexture"));
			mSourceTextureSampler.bind(initializer.mParameterMap, TEXT("SourceTextureSampler"));
		}

		AccumulateCubeFacesPS() {}

		void setParameters(RHICommandList& RHICmdList, int32 sourceMipIndexValue, TextureRHIRef& sourceTextureValue)
		{
			setShaderValue(RHICmdList, getPixelShader(), mSourceMipIndex, sourceMipIndexValue);
			setTextureParameter(RHICmdList,
				getPixelShader(),
				mSourceTexture,
				mSourceTextureSampler,
				TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::getRHI(),
				sourceTextureValue);
		}

		virtual bool serialize(Archive& ar) override
		{
			bool b = GlobalShader::serialize(ar);
			ar << mSourceMipIndex;
			ar << mSourceTexture;
			ar << mSourceTextureSampler;
			return b;
		}

	private:
		ShaderParameter mSourceMipIndex;
		ShaderResourceParameter mSourceTexture;
		ShaderResourceParameter mSourceTextureSampler;
	};

	IMPLEMENT_SHADER_TYPE(, AccumulateCubeFacesPS, TEXT("ReflectionEnvironmentShaders"), TEXT("AccumulateCubeFacesPS"), SF_Pixel);




	GlobalBoundShaderState s_CopyDiffuseIrradianceShaderState;

	GlobalBoundShaderState s_DiffuseIrradianceAccumulateShaderState;

	GlobalBoundShaderState s_AccumulateCubeFacesBoundShaderState;

	SceneRenderTargetItem& getEffectiveDiffuseIrradianceRenderTarget(SceneRenderTargets& sceneContext, int32 targetMipIndex)
	{
		const int32 scratchTextureIndex = targetMipIndex % 2;
		return sceneContext.mDiffuseIrradianceScratchCubemap[scratchTextureIndex]->getRenderTargetItem();
	}

	SceneRenderTargetItem& getEffectiveDiffuseIrradianceSourceTexture(SceneRenderTargets& sceneContext, int32 targetMipIndex)
	{
		const int32 scratchTextureIndex = 1 - targetMipIndex % 2;
		return sceneContext.mDiffuseIrradianceScratchCubemap[scratchTextureIndex]->getRenderTargetItem();
	}


	void computeDiffuseIrradiance(RHICommandListImmediate& RHICmdList, ERHIFeatureLevel::Type featureLevel, TextureRHIRef lightingSource, int32 lightingSourceMipIndex, SHVectorRGB3* outIrradianceEnvironmentMap)
	{
		auto shaderMap = getGlobalShaderMap(featureLevel);
		SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);
		for (int32 coefficientIndex = 0; coefficientIndex < SHVector3::MaxSHBasis; coefficientIndex++)
		{
			{
				const int32 mipIndex = 0;
				const int32 mipSize = GDiffuseIrradianceCubemapSize;
				SceneRenderTargetItem& effectiveRT = getEffectiveDiffuseIrradianceRenderTarget(sceneContext, mipIndex);

				for (int32 cubeface = 0; cubeface < CubeFace_MAX; cubeface++)
				{
					setRenderTarget(RHICmdList, effectiveRT.mTargetableTexture, 0, cubeface, nullptr, true);

					RHICmdList.clearColorTexture(effectiveRT.mTargetableTexture, LinearColor(0, 0, 0, 0), IntRect());

					const IntRect viewRect(0, 0, mipSize, mipSize);
					RHICmdList.setViewport(0, 0, 0.0f, mipSize, mipSize, 1.0f);
					RHICmdList.setRasterizerState(TStaticRasterizerState<FM_Solid, CM_None>::getRHI());
					RHICmdList.setDepthStencilState(TStaticDepthStencilState<false, CF_Always>::getRHI());
					RHICmdList.setBlendState(TStaticBlendState<>::getRHI());

					TShaderMapRef<CopyDiffuseIrradiancePS> pixelShader(shaderMap);

					TShaderMapRef<ScreenVS> vertexShader(getGlobalShaderMap(featureLevel));

					setGlobalBoundShaderState(RHICmdList, featureLevel, s_CopyDiffuseIrradianceShaderState, GFilterVertexDeclaration.mVertexDeclarationRHI, *vertexShader, *pixelShader);

					pixelShader->setParameters(RHICmdList, cubeface, lightingSourceMipIndex, coefficientIndex, mipSize, lightingSource);

					drawRectangle(RHICmdList,
						viewRect.min.x, viewRect.min.y,
						viewRect.width(), viewRect.height(),
						viewRect.min.x, viewRect.min.y,
						viewRect.width(), viewRect.height(),
						int2(viewRect.width(), viewRect.height()),
						int2(mipSize, mipSize),
						*vertexShader);

					RHICmdList.copyToResolveTarget(effectiveRT.mTargetableTexture, effectiveRT.mShaderResourceTexture, true, ResolveParams(ResolveRect(), (ECubeFace)cubeface));
				}
			}
			const int32 numMips = Math::ceilLogTwo(GDiffuseIrradianceCubemapSize) + 1;

			{
				for (int32 mipIndex = 1; mipIndex < numMips; mipIndex++)
				{
					const int32 sourceMipIndex = Math::max(mipIndex - 1, 0);
					const int32 mipSize = 1 << (numMips - mipIndex - 1);

					SceneRenderTargetItem& effectiveRT = getEffectiveDiffuseIrradianceRenderTarget(sceneContext, mipIndex);
					SceneRenderTargetItem& effectiveSource = getEffectiveDiffuseIrradianceSourceTexture(sceneContext, mipIndex);
					BOOST_ASSERT(effectiveRT.mTargetableTexture != effectiveSource.mShaderResourceTexture);

					for (int32 cubeface = 0; cubeface < CubeFace_MAX; cubeface++)
					{
						setRenderTarget(RHICmdList, effectiveRT.mTargetableTexture, mipIndex, cubeface, nullptr, true);

						const IntRect viewRect(0, 0, mipSize, mipSize);
						RHICmdList.setViewport(0, 0, 0.0f, mipSize, mipSize, 1.0f);
						RHICmdList.setRasterizerState(TStaticRasterizerState<FM_Solid, CM_None>::getRHI());
						RHICmdList.setDepthStencilState(TStaticDepthStencilState<false, CF_Always>::getRHI());
						RHICmdList.setBlendState(TStaticBlendState<>::getRHI());
						TShaderMapRef<AccumulateDiffuseIrradiancePS> pixelShader(shaderMap);

						TShaderMapRef<ScreenVS> vertexShader(getGlobalShaderMap(featureLevel));

						setGlobalBoundShaderState(RHICmdList, featureLevel, s_DiffuseIrradianceAccumulateShaderState, GFilterVertexDeclaration.mVertexDeclarationRHI, *vertexShader, *pixelShader);
						pixelShader->setParameters(RHICmdList, cubeface, numMips, sourceMipIndex, coefficientIndex, effectiveSource.mShaderResourceTexture);

						drawRectangle(RHICmdList,
							viewRect.min.x, viewRect.min.y,
							viewRect.width(), viewRect.height(),
							viewRect.min.x, viewRect.min.y,
							viewRect.width(), viewRect.height(),
							int2(viewRect.width(), viewRect.height()),
							int2(mipSize, mipSize),
							*vertexShader);
						RHICmdList.copyToResolveTarget(effectiveRT.mTargetableTexture, effectiveRT.mShaderResourceTexture, true, ResolveParams(ResolveRect(), (ECubeFace)cubeface, mipIndex));
					}
				}
			}
			{
				SceneRenderTargetItem& effectiveRT = SceneRenderTargets::get(RHICmdList).mSkySHIrradianceMap->getRenderTargetItem();

				RHIRenderTargetView RTV(effectiveRT.mTargetableTexture, 0, -1, ERenderTargetLoadAction::ELoad, ERenderTargetStoreAction::EStore);
				RHICmdList.transitionResource(EResourceTransitionAccess::EWritable, effectiveRT.mTargetableTexture);
				RHICmdList.setRenderTargets(1, &RTV, nullptr, 0, nullptr);
				const IntRect viewRect(coefficientIndex, 0, coefficientIndex + 1, 1);
				RHICmdList.setViewport(0, 0, 0.0f, SHVector3::MaxSHBasis, 1, 1.0f);
				RHICmdList.setRasterizerState(TStaticRasterizerState<FM_Solid, CM_None>::getRHI());
				RHICmdList.setDepthStencilState(TStaticDepthStencilState<false, CF_Always>::getRHI());
				RHICmdList.setBlendState(TStaticBlendState<>::getRHI());

				TShaderMapRef<ScreenVS> vertexShader(shaderMap);
				TShaderMapRef<AccumulateCubeFacesPS> pixelShader(shaderMap);

				setGlobalBoundShaderState(RHICmdList, featureLevel, s_AccumulateCubeFacesBoundShaderState, GFilterVertexDeclaration.mVertexDeclarationRHI, *vertexShader, *pixelShader);

				const int32 sourceMipIndex = numMips - 1;

				const int32 mipSize = 1;
				SceneRenderTargetItem& effectiveSource = getEffectiveDiffuseIrradianceRenderTarget(sceneContext, sourceMipIndex);
				pixelShader->setParameters(RHICmdList, sourceMipIndex, effectiveSource.mShaderResourceTexture);
				drawRectangle(RHICmdList,
					viewRect.min.x, viewRect.min.y,
					viewRect.width(), viewRect.height(),
					0, 0, mipSize, mipSize,
					int2(SHVector3::MaxSHBasis, 1),
					int2(mipSize, mipSize), *vertexShader);
				RHICmdList.copyToResolveTarget(effectiveRT.mTargetableTexture, effectiveRT.mShaderResourceTexture, true, ResolveParams());
			}
		}
		{
			SceneRenderTargetItem& effectiveRT = SceneRenderTargets::get(RHICmdList).mSkySHIrradianceMap->getRenderTargetItem();
			BOOST_ASSERT(effectiveRT.mShaderResourceTexture->getFormat() == PF_FloatRGBA);

			TArray<Float16Color> surfaceData;
			RHICmdList.readSurfaceFloatData(effectiveRT.mShaderResourceTexture, IntRect(0, 0, SHVector3::MaxSHBasis, 1), surfaceData, CubeFace_PosX, 0, 0);
			BOOST_ASSERT(surfaceData.size() == SHVector3::MaxSHBasis);
			for (int32 coefficientIndex = 0; coefficientIndex < SHVector3::MaxSHBasis; coefficientIndex++)
			{
				const LinearColor coefficientValue(surfaceData[coefficientIndex]);
				outIrradianceEnvironmentMap->r.V[coefficientIndex] = coefficientValue.R;
				outIrradianceEnvironmentMap->g.V[coefficientIndex] = coefficientValue.G;
				outIrradianceEnvironmentMap->b.V[coefficientIndex] = coefficientValue.B;
			}
		}
	}
}