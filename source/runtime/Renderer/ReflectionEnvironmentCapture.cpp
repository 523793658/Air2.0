#include "ScenePrivate.h"
#include "RHIUtilities.h"
#include "Classes/Components/SkyLightComponent.h"
#include "StaticBoundShaderState.h"
#include "ScreenRendering.h"
#include "PostProcess/SceneFilterRendering.h"
#include "OneColorShader.h"
#include "PostProcess/RenderTargetPool.h"
#include "ReflectionEnvironmentCapture.h"
#include "PostProcess/PostProcessing.h"
namespace Air
{

	SceneRenderTargetItem& getEffectiveRenderTarget(SceneRenderTargets& sceneContext, bool bDownsamplePass, int32 targetMipIndex)
	{
		int32 scratchTextureIndex = targetMipIndex % 2;
		if (!bDownsamplePass)
		{
			scratchTextureIndex = 1 - scratchTextureIndex;
		}
		return sceneContext.mReflectionColorScratchCubemap[scratchTextureIndex]->getRenderTargetItem();
	}

	void clearScratchCubmaps(RHICommandList& RHICmdList, int32 targetSize)
	{
		SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);
		sceneContext.allocateReflectionTargets(RHICmdList, targetSize);

		SceneRenderTargetItem& RT0 = sceneContext.mReflectionColorScratchCubemap[0]->getRenderTargetItem();
		int32 numMips = (int32)RT0.mTargetableTexture->getNumMips();

		{
			for (int32 mipIndex = 0; mipIndex < numMips; mipIndex++)
			{
				for (int32 cubeFace = 0; cubeFace < CubeFace_MAX; cubeFace++)
				{
					setRenderTarget(RHICmdList, RT0.mTargetableTexture, mipIndex, cubeFace, nullptr, true);
					RHICmdList.clearColorTexture(RT0.mTargetableTexture, LinearColor(0, 10000, 0, 0), IntRect());
				}
			}
		}
		{
			SceneRenderTargetItem& rt1 = sceneContext.mReflectionColorScratchCubemap[1]->getRenderTargetItem();
			numMips = (int32)rt1.mTargetableTexture->getNumMips();
			for (int32 mipIndex = 0; mipIndex < numMips; mipIndex++)
			{
				for (int32 cubeFace = 0; cubeFace < CubeFace_MAX; cubeFace++)
				{
					setRenderTarget(RHICmdList, rt1.mTargetableTexture, mipIndex, cubeFace, nullptr, true);
					RHICmdList.clearColorTexture(rt1.mTargetableTexture, LinearColor(0, 10000, 0, 0), IntRect());
				}
			}
		}
	}

	class CopyCubemapToCubefacePS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(CopyCubemapToCubefacePS, Global);
	public:	
		static bool shouldCache(EShaderPlatform platform)
		{
			return true;
		}

		CopyCubemapToCubefacePS(const ShaderMetaType::CompiledShaderInitializerType& initializer) :
			GlobalShader(initializer)
		{
			mCubeFace.bind(initializer.mParameterMap, TEXT("CubeFace"));
			mSourceTexture.bind(initializer.mParameterMap, TEXT("SourceTexture"));
			mSourceTextureSampler.bind(initializer.mParameterMap, TEXT("SourceTextureSampler"));
			mSkyLightCaptureParameters.bind(initializer.mParameterMap, TEXT("SkyLightCaptureParameters"));
			mLowerHemisphereColor.bind(initializer.mParameterMap, TEXT("LowerHemisphereColor"));
			mSinCosSourceCubemapRotation.bind(initializer.mParameterMap, TEXT("SinCosSourceCubemapRotation"));

		}

		CopyCubemapToCubefacePS() {}

		void setParameters(RHICommandList& RHICmdList, const Texture* sourceCubemap, uint32 cubeFaceValue, bool bIsSkyLight, bool bLowerHemisphereIsBlack, float sourceCubemapRotation, const LinearColor& lowerHemisphereColorValue)
		{
			const PixelShaderRHIParamRef shaderRHI = getPixelShader();
			setShaderValue(RHICmdList, shaderRHI, mCubeFace, cubeFaceValue);
			setTextureParameter(RHICmdList, shaderRHI, mSourceTexture, mSourceTextureSampler, sourceCubemap);
			setShaderValue(RHICmdList, shaderRHI, mSkyLightCaptureParameters, float3(bIsSkyLight ? 1.0f : 0.0f, 0.0f, bLowerHemisphereIsBlack ? 1.0f : 0.0f));
			setShaderValue(RHICmdList, shaderRHI, mLowerHemisphereColor, lowerHemisphereColorValue);
			setShaderValue(RHICmdList, shaderRHI, mSinCosSourceCubemapRotation, float2(Math::sin(sourceCubemapRotation), Math::cos(sourceCubemapRotation)));
		}

		virtual bool serialize(Archive& ar) override
		{
			bool b = GlobalShader::serialize(ar);
			ar << mCubeFace;
			ar << mSourceTexture;
			ar << mSourceTextureSampler;
			ar << mSkyLightCaptureParameters;
			ar << mLowerHemisphereColor;
			ar << mSinCosSourceCubemapRotation;
			return b;
		}

	private:
		ShaderParameter mCubeFace;
		ShaderResourceParameter mSourceTexture;
		ShaderResourceParameter mSourceTextureSampler;
		ShaderParameter mSkyLightCaptureParameters;
		ShaderParameter mLowerHemisphereColor;
		ShaderParameter mSinCosSourceCubemapRotation;
	};

	IMPLEMENT_SHADER_TYPE(, CopyCubemapToCubefacePS, TEXT("ReflectionEnvironmentShaders"), TEXT("CopyCubemapToCubeFaceColorPS"), SF_Pixel);

	class DownsamplePS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(DownsamplePS, Global);
	public:	
		static bool shouldCache(EShaderPlatform platform)
		{
			return true;
		}

		DownsamplePS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mCubeFace.bind(initializer.mParameterMap, TEXT("CubeFace"));
			mSourceMipIndex.bind(initializer.mParameterMap, TEXT("SourceMipIndex"));
			mSourceTexture.bind(initializer.mParameterMap, TEXT("SourceTexture"));
			mSourceTextureSampler.bind(initializer.mParameterMap, TEXT("SourceTextureSampler"));
		}

		DownsamplePS() {}

		void setParameters(RHICommandList& RHICmdList, int32 cubeFaceValue, int32 sourceMipIndexValue, SceneRenderTargetItem& sourceTextureValue)
		{
			setShaderValue(RHICmdList, getPixelShader(), mCubeFace, cubeFaceValue);
			setShaderValue(RHICmdList, getPixelShader(), mSourceMipIndex, sourceMipIndexValue);
			setTextureParameter(RHICmdList, getPixelShader(), mSourceTexture, mSourceTextureSampler, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::getRHI(), sourceTextureValue.mShaderResourceTexture);
		}

		virtual bool serialize(Archive& ar) override
		{
			bool b = GlobalShader::serialize(ar);
			ar << mCubeFace;
			ar << mSourceMipIndex;
			ar << mSourceTexture;
			ar << mSourceTextureSampler;
			return b;
		}

	private:
		ShaderParameter mCubeFace;
		ShaderParameter mSourceMipIndex;
		ShaderResourceParameter mSourceTexture;
		ShaderResourceParameter mSourceTextureSampler;
	};

	IMPLEMENT_SHADER_TYPE(, DownsamplePS, TEXT("ReflectionEnvironmentShaders"), TEXT("DownsamplePS"), SF_Pixel);

	class ComputeBrightnessPS : public GlobalShader
	{
		DECLARE_SHADER_TYPE(ComputeBrightnessPS, Global)
	public:
		static bool shouldCache(EShaderPlatform platform)
		{
			return true;
		}

		static void modifyCompilationEnvironment(EShaderPlatform platform, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(platform, outEnvironment);
			outEnvironment.setDefine(TEXT("COMPUTEBRIGHTNESS_PIXELSHADER"), 1);
		}

		ComputeBrightnessPS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			mReflectionEnvironmentColorTexture.bind(initializer.mParameterMap, TEXT("ReflectionEnvironmentColorTexture"));
			mReflectionEnvironmentColorTextureSampler.bind(initializer.mParameterMap, TEXT("ReflectionEnvironmentColorTextureSampler"));
			mNumCaptureArrayMips.bind(initializer.mParameterMap, TEXT("NumCaptureArrayMips"));
		}

		ComputeBrightnessPS() {}
	
		void setParameters(RHICommandList& RHICmdList, int32 targetSize)
		{
			const int32 effectiveToMipSize = targetSize;
			const int32 numMips = Math::ceilLogTwo(effectiveToMipSize) + 1;
			SceneRenderTargetItem& cubemap = getEffectiveRenderTarget(SceneRenderTargets::get(RHICmdList), true, numMips - 1);

			if (cubemap.isValid())
			{
				setTextureParameter(RHICmdList,
					getPixelShader(),
					mReflectionEnvironmentColorTexture,
					mReflectionEnvironmentColorTextureSampler,
					TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::getRHI(), cubemap.mShaderResourceTexture);
			}
			setShaderValue(RHICmdList, getPixelShader(), mNumCaptureArrayMips, Math::ceilLogTwo(targetSize) + 1);
		}

		virtual bool serialize(Archive& ar) override {
			bool b = GlobalShader::serialize(ar);
			ar << mReflectionEnvironmentColorTexture;
			ar << mReflectionEnvironmentColorTextureSampler;
			ar << mNumCaptureArrayMips;
			return b;
		}

	private:
		ShaderResourceParameter mReflectionEnvironmentColorTexture;
		ShaderResourceParameter mReflectionEnvironmentColorTextureSampler;
		ShaderParameter mNumCaptureArrayMips;
	};

	IMPLEMENT_SHADER_TYPE(, ComputeBrightnessPS, TEXT("ReflectionEnvironmentShaders"), TEXT("ComputeBrightnessMain"), SF_Pixel);

	class CubeFilterPS : public DownsamplePS
	{
		DECLARE_SHADER_TYPE(CubeFilterPS, Global);
	public:
		static bool shouldCache(EShaderPlatform platform)
		{
			return true;
		}

		static void modifyCompilationEnvironment(EShaderPlatform platform, ShaderCompilerEnvironment& outEnvironment)
		{
			DownsamplePS::modifyCompilationEnvironment(platform, outEnvironment);
		}

		CubeFilterPS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:DownsamplePS(initializer)
		{
			mCubemapMaxMipParameter.bind(initializer.mParameterMap, TEXT("CubemapMaxMip"));
		}

		CubeFilterPS() {}

		void setParameters(RHICommandList& RHICmdList, int32 numMips, int32 cubeFaceValue, int32 sourceMipIndexValue, SceneRenderTargetItem& sourceTextureValue)
		{
			DownsamplePS::setParameters(RHICmdList, cubeFaceValue, sourceMipIndexValue, sourceTextureValue);
			setShaderValue(RHICmdList, getPixelShader(), mCubemapMaxMipParameter, numMips - 1.0f);
		}

		virtual bool serialize(Archive& ar) override
		{
			bool b = DownsamplePS::serialize(ar);
			ar << mCubemapMaxMipParameter;
			return b;
		}

	private:
		ShaderParameter mCubemapMaxMipParameter;
	};


	template<uint32 bNormalize>
	class TCubeFilterPS : public CubeFilterPS
	{
		DECLARE_SHADER_TYPE(TCubeFilterPS, Global)
	public:
		static void modifyCompilationEnvironment(EShaderPlatform platform, ShaderCompilerEnvironment& outEnvironment)
		{
			CubeFilterPS::modifyCompilationEnvironment(platform, outEnvironment);
			outEnvironment.setDefine(TEXT("NORMALIZE"), bNormalize);
		}

		TCubeFilterPS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:CubeFilterPS(initializer)
		{}

		TCubeFilterPS() {}
	};

	IMPLEMENT_SHADER_TYPE(template<>, TCubeFilterPS<0>, TEXT("ReflectionEnvironmentShaders"), TEXT("FilterPS"), SF_Pixel);
	IMPLEMENT_SHADER_TYPE(template<>, TCubeFilterPS<1>, TEXT("ReflectionEnvironmentShaders"), TEXT("FilterPS"), SF_Pixel);

	GlobalBoundShaderState mCopyFromCubemapToCubemapBoundShaderState;

	static GlobalBoundShaderState s_DownsampleBoundShaderState;

	int32 GDiffuseIrradianceCubemapSize = 32;



	SceneRenderTargetItem& getEffectiveSourceTexture(SceneRenderTargets& sceneContext, bool bDownsamplePass, int32 targetMipIndex)
	{
		int32 scratchTextureIndex = targetMipIndex % 2;
		if (bDownsamplePass)
		{
			scratchTextureIndex = 1 - scratchTextureIndex;
		}
		return sceneContext.mReflectionColorScratchCubemap[scratchTextureIndex]->getRenderTargetItem();
	}

	void copyCubemapToScratchCubemap(RHICommandList& RHICmdList, ERHIFeatureLevel::Type featureLevel, RTextureCube* sourceCubemap, int32 cubemapSize, bool bIsSkyLight, bool bLowerHeisphereIsBlack, float sourceCubemapRotation, const LinearColor& lowerhemisphereColor)
	{
		BOOST_ASSERT(sourceCubemap);
		const int32 effectiveSize = cubemapSize;
		SceneRenderTargetItem& effectiveColorRT = SceneRenderTargets::get(RHICmdList).mReflectionColorScratchCubemap[0]->getRenderTargetItem();
		for (uint32 cubeFace = 0; cubeFace < CubeFace_MAX; cubeFace++)
		{
			setRenderTarget(RHICmdList, effectiveColorRT.mTargetableTexture, 0, cubeFace, nullptr, true);

			const Texture* sourceCubemapResource = sourceCubemap->mResource;
			const int2 sourceDimensions(sourceCubemapResource->getWidth(), sourceCubemapResource->getHeight());
			const IntRect viewRect(0, 0, effectiveSize, effectiveSize);
			RHICmdList.setViewport(0, 0, 0.0f, effectiveSize, effectiveSize, 1.0f);
			RHICmdList.setRasterizerState(TStaticRasterizerState<FM_Solid, CM_None>::getRHI());
			RHICmdList.setDepthStencilState(TStaticDepthStencilState<false, CF_Always>::getRHI());
			RHICmdList.setBlendState(TStaticBlendState<>::getRHI());

			TShaderMapRef<ScreenVS> vertexShader(getGlobalShaderMap(featureLevel));
			TShaderMapRef<CopyCubemapToCubefacePS> pixelShader(getGlobalShaderMap(featureLevel));

			setGlobalBoundShaderState(RHICmdList, featureLevel, mCopyFromCubemapToCubemapBoundShaderState, GFilterVertexDeclaration.mVertexDeclarationRHI, *vertexShader, *pixelShader);
			pixelShader->setParameters(RHICmdList, sourceCubemapResource, cubeFace, bIsSkyLight, bLowerHeisphereIsBlack, sourceCubemapRotation, lowerhemisphereColor);

			drawRectangle(RHICmdList, viewRect.min.x, viewRect.min.y,
				viewRect.width(), viewRect.height(),
				0, 0, sourceDimensions.x, sourceDimensions.y,
				int2(viewRect.width(), viewRect.height()),
				sourceDimensions,
				*vertexShader);
			RHICmdList.copyToResolveTarget(effectiveColorRT.mTargetableTexture, effectiveColorRT.mShaderResourceTexture, true, ResolveParams(ResolveRect(), (ECubeFace)cubeFace));
		}
	}

	void fullyResolveReflectionScratchCubes(RHICommandListImmediate& RHICmdList)
	{
		SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);
		TextureRHIRef& scratch0 = sceneContext.mReflectionColorScratchCubemap[0]->getRenderTargetItem().mTargetableTexture;
		TextureRHIRef& scratch1 = sceneContext.mReflectionColorScratchCubemap[1]->getRenderTargetItem().mTargetableTexture;
		ResolveParams resolveParams(ResolveRect(), CubeFace_PosX, -1, -1, -1);
		RHICmdList.copyToResolveTarget(scratch0, scratch0, true, resolveParams);
		RHICmdList.copyToResolveTarget(scratch1, scratch1, true, resolveParams);
	}

	float computeSingleAverageBrightnessFromCubemap(RHICommandListImmediate& RHICmdList, ERHIFeatureLevel::Type featureLevel, int32 targetSize)
	{
		TRefCountPtr<IPooledRenderTarget> reflectionBrightnessTarget;
		PooledRenderTargetDesc desc(PooledRenderTargetDesc::create2DDesc(int2(1, 1), PF_FloatRGBA, ClearValueBinding::None, TexCreate_None, TexCreate_RenderTargetable, false));
		GRenderTargetPool.findFreeElement(RHICmdList, desc, reflectionBrightnessTarget, TEXT("ReflectionBrightness"));

		TextureRHIRef& brightnessTarget = reflectionBrightnessTarget->getRenderTargetItem().mTargetableTexture;
		setRenderTarget(RHICmdList, brightnessTarget, nullptr, true);
		RHICmdList.setRasterizerState(TStaticRasterizerState<FM_Solid, CM_None>::getRHI());
		RHICmdList.setDepthStencilState(TStaticDepthStencilState<false, CF_Always>::getRHI());
		RHICmdList.setBlendState(TStaticBlendState<>::getRHI());

		auto shaderMap = getGlobalShaderMap(featureLevel);
		TShaderMapRef<PostProcessVS> vertexShader(shaderMap);
		TShaderMapRef<ComputeBrightnessPS> pixelShader(shaderMap);

		static GlobalBoundShaderState boundShaderState;

		setGlobalBoundShaderState(RHICmdList, featureLevel, boundShaderState, GFilterVertexDeclaration.mVertexDeclarationRHI, *vertexShader, *pixelShader);
		pixelShader->setParameters(RHICmdList, targetSize);

		drawRectangle(RHICmdList,
			0, 0,
			1, 1,
			0, 0,
			1, 1,
			int2(1, 1),
			int2(1, 1),
			*vertexShader);

		RHICmdList.copyToResolveTarget(brightnessTarget, brightnessTarget, true, ResolveParams());

		SceneRenderTargetItem& effectiveRT = reflectionBrightnessTarget->getRenderTargetItem();
		BOOST_ASSERT(effectiveRT.mShaderResourceTexture->getFormat() == PF_FloatRGBA);

		TArray<Float16Color> surfaceData;
		RHICmdList.readSurfaceFloatData(effectiveRT.mShaderResourceTexture, IntRect(0, 0, 1, 1), surfaceData, CubeFace_PosX, 0, 0);

		float averageBrightness = surfaceData[0].R.getFloat();
		return averageBrightness;
	}

	void computeAverageBrightness(RHICommandListImmediate& RHICmdList, ERHIFeatureLevel::Type featureLevel, int32 cubemapSize, float& outAverageBrightness)
	{
		const int32 effecitveToMipSize = cubemapSize;
		const int32 numMips = Math::ceilLogTwo(effecitveToMipSize) + 1;
		fullyResolveReflectionScratchCubes(RHICmdList);

		auto shaderMap = getGlobalShaderMap(featureLevel);

		SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);

		{
			for (int32 mipIndex = 1; mipIndex < numMips; mipIndex++)
			{
				const int32 sourceMipIndex = Math::max(mipIndex - 1, 0);
				const int32 mipSize = 1 << (numMips - mipIndex - 1);

				SceneRenderTargetItem& effectiveRT = getEffectiveRenderTarget(sceneContext, true, mipIndex);
				SceneRenderTargetItem& effectiveSource = getEffectiveSourceTexture(sceneContext, true, mipIndex);
				BOOST_ASSERT(effectiveRT.mTargetableTexture != effectiveSource.mTargetableTexture);
				for (int32 cubeFace = 0; cubeFace < CubeFace_MAX; cubeFace++)
				{
					setRenderTarget(RHICmdList, effectiveRT.mTargetableTexture, mipIndex, cubeFace, nullptr, true);

					const IntRect viewRect(0, 0, mipSize, mipSize);
					RHICmdList.setViewport(0, 0, 0.0f, mipSize, mipSize, 1.0f);
					RHICmdList.setRasterizerState(TStaticRasterizerState<FM_Solid, CM_None>::getRHI());
					RHICmdList.setDepthStencilState(TStaticDepthStencilState<false, CF_Always>::getRHI());
					RHICmdList.setBlendState(TStaticBlendState<>::getRHI());

					TShaderMapRef<ScreenVS> vertexShader(getGlobalShaderMap(featureLevel));
					TShaderMapRef<DownsamplePS> pixelShader(getGlobalShaderMap(featureLevel));

					setGlobalBoundShaderState(RHICmdList, featureLevel, s_DownsampleBoundShaderState, GFilterVertexDeclaration.mVertexDeclarationRHI, *vertexShader, *pixelShader);

					pixelShader->setParameters(RHICmdList, cubeFace, sourceMipIndex, effectiveSource);
					drawRectangle(RHICmdList,
						viewRect.min.x, viewRect.min.y,
						viewRect.width(), viewRect.height(),
						viewRect.min.x, viewRect.min.y,
						viewRect.width(), viewRect.height(),
						int2(viewRect.width(), viewRect.height()),
						int2(mipSize, mipSize),
						*vertexShader);
					RHICmdList.copyToResolveTarget(effectiveRT.mTargetableTexture, effectiveRT.mShaderResourceTexture, true, ResolveParams(ResolveRect(), (ECubeFace)cubeFace, mipIndex));
				}
			}
		}
		outAverageBrightness = computeSingleAverageBrightnessFromCubemap(RHICmdList, featureLevel, cubemapSize);
	}

	void filterReflectionEnvironment(RHICommandListImmediate& RHICmdList, ERHIFeatureLevel::Type featureLevel, int32 cubemapSize, SHVectorRGB3* outIrradianceEnvironmentMap)
	{
		const int32 effectiveTopMipSize = cubemapSize;
		const int32 numMips = Math::ceilLogTwo(effectiveTopMipSize) + 1;
		SceneRenderTargetItem& effectiveColorRT = SceneRenderTargets::get(RHICmdList).mReflectionColorScratchCubemap[0]->getRenderTargetItem();

		for (uint32 cubeface = 0; cubeface < CubeFace_MAX; cubeface++)
		{
			setRenderTarget(RHICmdList, effectiveColorRT.mTargetableTexture, 0, cubeface, nullptr, true);

			const int2 sourceDimension(cubemapSize, cubemapSize);
			const IntRect viewRect(0, 0, effectiveTopMipSize, effectiveTopMipSize);

			RHICmdList.setViewport(0, 0, 0.0f, effectiveTopMipSize, effectiveTopMipSize, 1.0f);
			RHICmdList.setRasterizerState(TStaticRasterizerState<FM_Solid, CM_None>::getRHI());
			RHICmdList.setDepthStencilState(TStaticDepthStencilState<false, CF_Always>::getRHI());
			RHICmdList.setBlendState(TStaticBlendState<CW_RGBA, BO_Add, BF_Zero, BF_DestAlpha, BO_Add, BF_Zero, BF_One>::getRHI());

			TShaderMapRef<ScreenVS> vertexShader(getGlobalShaderMap(featureLevel));
			TShaderMapRef<OneColorPS> pixelShader(getGlobalShaderMap(featureLevel));

			static GlobalBoundShaderState boundShaderState;
			setGlobalBoundShaderState(RHICmdList, featureLevel, boundShaderState, GFilterVertexDeclaration.mVertexDeclarationRHI, *vertexShader, *pixelShader);
			LinearColor unusedColors[1] = { LinearColor::Black };
			pixelShader->setColors(RHICmdList, unusedColors, ARRAY_COUNT(unusedColors));
			drawRectangle(RHICmdList,
				viewRect.min.x, viewRect.min.y,
				viewRect.width(), viewRect.height(),
				0, 0,
				sourceDimension.x, sourceDimension.y,
				int2(viewRect.width(), viewRect.height()),
				sourceDimension,
				*vertexShader);
			RHICmdList.copyToResolveTarget(effectiveColorRT.mTargetableTexture, effectiveColorRT.mShaderResourceTexture, true, ResolveParams(ResolveRect(), (ECubeFace)cubeface));
		}
		int32 diffuseConvolutionSourceMip = INDEX_NONE;
		SceneRenderTargetItem* diffuseConvolutionSource = nullptr;

		auto shaderMap = getGlobalShaderMap(featureLevel);
		SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);

		{
			for (int32 mipIndex = 1; mipIndex < numMips; mipIndex++)
			{
				const int32 sourceMipIndex = Math::max(mipIndex - 1, 0);
				const int32 mipSize = 1 << (numMips - mipIndex - 1);

				SceneRenderTargetItem& effectiveRT = getEffectiveRenderTarget(sceneContext, true, mipIndex);
				SceneRenderTargetItem& effectiveSource = getEffectiveSourceTexture(sceneContext, true, mipIndex);
				BOOST_ASSERT(effectiveRT.mTargetableTexture != effectiveSource.mShaderResourceTexture);
				for (int32 cubeFace = 0; cubeFace < CubeFace_MAX; cubeFace++)
				{
					setRenderTarget(RHICmdList, effectiveRT.mTargetableTexture, mipIndex, cubeFace, nullptr, true);

					const IntRect viewRect(0, 0, mipSize, mipSize);
					RHICmdList.setViewport(0, 0, 0.0f, mipSize, mipSize, 1.0f);
					RHICmdList.setRasterizerState(TStaticRasterizerState<FM_Solid, CM_None>::getRHI());
					RHICmdList.setDepthStencilState(TStaticDepthStencilState<false, CF_Always>::getRHI());
					RHICmdList.setBlendState(TStaticBlendState<>::getRHI());
					TShaderMapRef<ScreenVS> vertexShader(getGlobalShaderMap(featureLevel));
					TShaderMapRef<DownsamplePS> pixelShader(getGlobalShaderMap(featureLevel));

					setGlobalBoundShaderState(RHICmdList, featureLevel, s_DownsampleBoundShaderState, GFilterVertexDeclaration.mVertexDeclarationRHI, *vertexShader, *pixelShader);

					pixelShader->setParameters(RHICmdList, cubeFace, sourceMipIndex, effectiveSource);
					drawRectangle(RHICmdList,
						viewRect.min.x, viewRect.min.y,
						viewRect.width(), viewRect.height(),
						viewRect.min.x, viewRect.min.y,
						viewRect.width(), viewRect.height(),
						int2(viewRect.width(), viewRect.height()),
						int2(mipSize, mipSize),
						*vertexShader);
					RHICmdList.copyToResolveTarget(effectiveRT.mTargetableTexture, effectiveRT.mShaderResourceTexture, true, ResolveParams(ResolveRect(), (ECubeFace)cubeFace));
				}
				if (mipSize == GDiffuseIrradianceCubemapSize)
				{
					diffuseConvolutionSourceMip = mipIndex;
					diffuseConvolutionSource = &effectiveRT;
				}
			}
		}
		if (outIrradianceEnvironmentMap)
		{
			BOOST_ASSERT(diffuseConvolutionSource != nullptr);
			computeDiffuseIrradiance(RHICmdList, featureLevel, diffuseConvolutionSource->mShaderResourceTexture, diffuseConvolutionSourceMip, outIrradianceEnvironmentMap);
		}
		{
			for (int32 mipIndex = 0; mipIndex < numMips; mipIndex++)
			{
				SceneRenderTargetItem& effectiveRT = getEffectiveRenderTarget(sceneContext, false, mipIndex);
				SceneRenderTargetItem& effectiveSource = getEffectiveSourceTexture(sceneContext, false, mipIndex);

				BOOST_ASSERT(effectiveRT.mTargetableTexture != effectiveSource.mShaderResourceTexture);
				const int32 mipSize = 1 << (numMips - mipIndex - 1);

				for (int32 cubeface = 0; cubeface < CubeFace_MAX; cubeface++)
				{
					setRenderTarget(RHICmdList, effectiveRT.mTargetableTexture, mipIndex, cubeface, nullptr, true);

					const IntRect viewRect(0, 0, mipSize, mipSize);
					RHICmdList.setViewport(0, 0, 0.0f, mipSize, mipSize, 1.0f);
					RHICmdList.setRasterizerState(TStaticRasterizerState<FM_Solid, CM_None>::getRHI());
					RHICmdList.setDepthStencilState(TStaticDepthStencilState<false, CF_Always>::getRHI());
					RHICmdList.setBlendState(TStaticBlendState<>::getRHI());

					TShaderMapRef<ScreenVS> vertexShader(getGlobalShaderMap(featureLevel));
					TShaderMapRef<TCubeFilterPS<1>> captureCubemapArrayPixelShader(getGlobalShaderMap(featureLevel));

					CubeFilterPS* pixelShader;

					pixelShader = *TShaderMapRef<TCubeFilterPS<0>>(shaderMap);

					static GlobalBoundShaderState boundShaderState;
					setGlobalBoundShaderState(RHICmdList, featureLevel, boundShaderState, GFilterVertexDeclaration.mVertexDeclarationRHI, *vertexShader, pixelShader);

					pixelShader->setParameters(RHICmdList, numMips, cubeface, mipIndex, effectiveSource);

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
		}
	}

	void copyToSkyTexture(RHICommandList& RHICmdList, Scene* scene, Texture* processedTexture)
	{
		if (processedTexture->mTextureRHI)
		{
			const int32 effectiveTopMipSize = processedTexture->getWidth();
			const int32 numMips = Math::ceilLogTwo(effectiveTopMipSize) + 1;
			SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);
			for (int32 mipIndex = 0; mipIndex < numMips; mipIndex++)
			{
				SceneRenderTargetItem& effectiveSource = getEffectiveRenderTarget(sceneContext, false, mipIndex);
				for (int32 cubeface = 0; cubeface < CubeFace_MAX; cubeface++)
				{
					RHICmdList.copyToResolveTarget(effectiveSource.mShaderResourceTexture, processedTexture->mTextureRHI, true, ResolveParams(ResolveRect(), (ECubeFace)cubeface, mipIndex, 0, 0));
				}
			}
		}
	}

	void Scene::updateSkyCaptureContents(const SkyLightComponent* captureComponent, bool bCaptureEmissiveOnly, std::shared_ptr<RTextureCube> sourceCubemap, Texture* outProcessedTexture, float& outAverageBrightness, SHVectorRGB3& outIrradianceEnvironmentMap)
	{
		if (GSupportsRenderTargetFormat_PF_FloatRGBA || getFeatureLevel() >= ERHIFeatureLevel::SM4)
		{
			{
				mWorld = getWorld();
				if (mWorld)
				{
					mWorld->sendAllEndOfFrameUpdates();
				}
			}

			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
				ClearCommand,
				int32, cubemapSize, captureComponent->mCubemapResolution,
				{
					clearScratchCubmaps(RHICmdList, cubemapSize);
				}
			);

			if (captureComponent->mSourceType == SLS_CapturedScene)
			{
				bool bStaticSceneOnly = captureComponent->mMobility != EComponentMobility::Movable;
				//后续实现
			}
			else if (captureComponent->mSourceType == SLS_SpecifiedCubmap)
			{
				ENQUEUE_UNIQUE_RENDER_COMMAND_SIXPARAMETER(
					CopyCubemapCommand,
					std::shared_ptr<RTextureCube>, sourceTexture, sourceCubemap,
					int32, cubemapSize, captureComponent->mCubemapResolution,
					bool, bLowerHemisphereIsBlack, captureComponent->bLowerHemisphereIsBlack,
					float, sourceCubemapRotation, captureComponent->mSourceCubemapAngle * (PI / 180.f),
					ERHIFeatureLevel::Type, featureLevel, getFeatureLevel(),
					LinearColor, lowerHemisphereColor, captureComponent->lowerHemisphereColor,
					{
						copyCubemapToScratchCubemap(RHICmdList, featureLevel, sourceTexture.get(), cubemapSize, true, bLowerHemisphereIsBlack, sourceCubemapRotation, lowerHemisphereColor);
					}
				);
			}
			else
			{
				BOOST_ASSERT(false);
			}
			ENQUEUE_UNIQUE_RENDER_COMMAND_FOURPARAMETER(
				FilterCommand,
				int32, cubemapSize, captureComponent->mCubemapResolution,
				float&, averageBrightness, outAverageBrightness,
				SHVectorRGB3*, irradianceEnvironmentMap, &outIrradianceEnvironmentMap,
				ERHIFeatureLevel::Type, featureLevel, getFeatureLevel(),
				{
					computeAverageBrightness(RHICmdList, featureLevel, cubemapSize, averageBrightness);
					filterReflectionEnvironment(RHICmdList, featureLevel, cubemapSize, irradianceEnvironmentMap);
				}
			);

			if (outProcessedTexture)
			{
				ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
					CopyCommand,
					Scene*, scene, this,
					Texture*, processedTexture, outProcessedTexture,
					{
						copyToSkyTexture(RHICmdList, scene, processedTexture);
					}
				);
			}
		}

	}
}