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
					RHIRenderPassInfo RPInfo(RT0.mTargetableTexture, ERenderTargetActions::Clear_Store, nullptr, mipIndex, cubeFace);
					RHICmdList.beginRenderPass(RPInfo, TEXT("ClearCubeFace"));
					RHICmdList.endRenderPass();
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
					RHIRenderPassInfo RPInfo(rt1.mTargetableTexture, ERenderTargetActions::Clear_Store, nullptr, mipIndex, cubeFace);
					RHICmdList.beginRenderPass(RPInfo, TEXT("ClearCubeFace"));
					RHICmdList.endRenderPass();
				}
			}
		}
	}

	class CopyCubemapToCubefacePS : public GlobalShader
	{
		DECLARE_GLOBAL_SHADER(CopyCubemapToCubefacePS);
	public:	
		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
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
			RHIPixelShader* shaderRHI = getPixelShader();
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

	IMPLEMENT_GLOBAL_SHADER(CopyCubemapToCubefacePS, "ReflectionEnvironmentShaders", "CopyCubemapToCubeFaceColorPS", SF_Pixel);

	class DownsamplePS : public GlobalShader
	{
		DECLARE_GLOBAL_SHADER(DownsamplePS);
	public:	
		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
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

	
		ShaderParameter mCubeFace;
		ShaderParameter mSourceMipIndex;
		ShaderParameter mNumMips;
		ShaderResourceParameter mSourceTexture;
		ShaderResourceParameter mSourceTextureSampler;
	};

	IMPLEMENT_GLOBAL_SHADER(DownsamplePS, "ReflectionEnvironmentShaders", "DownsamplePS", SF_Pixel);

	class ComputeBrightnessPS : public GlobalShader
	{
		DECLARE_GLOBAL_SHADER(ComputeBrightnessPS)
	public:
		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			return true;
		}

		static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			GlobalShader::modifyCompilationEnvironment(parameters, outEnvironment);
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
	
		void setParameters(RHICommandList& RHICmdList, int32 targetSize, SceneRenderTargetItem& cubemap)
		{
			const int32 effectiveToMipSize = targetSize;
			const int32 numMips = Math::ceilLogTwo(effectiveToMipSize) + 1;
		

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

	IMPLEMENT_GLOBAL_SHADER(ComputeBrightnessPS, "ReflectionEnvironmentShaders", "ComputeBrightnessMain", SF_Pixel);

	class CubeFilterPS : public DownsamplePS
	{
		DECLARE_GLOBAL_SHADER(CubeFilterPS);
	public:
		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameters)
		{
			return true;
		}

		static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			DownsamplePS::modifyCompilationEnvironment(parameters, outEnvironment);
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
		DECLARE_GLOBAL_SHADER(TCubeFilterPS)
	public:
		static void modifyCompilationEnvironment(const GlobalShaderPermutationParameters& parameters, ShaderCompilerEnvironment& outEnvironment)
		{
			CubeFilterPS::modifyCompilationEnvironment(parameters, outEnvironment);
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
		RHICmdList.transitionResource(EResourceTransitionAccess::EWritable, effectiveColorRT.mTargetableTexture);

		const Texture* sourceCubemapResource = sourceCubemap->mResource;
		if (sourceCubemapResource == nullptr)
		{
			return;
		}

		for (uint32 cubeFace = 0; cubeFace < CubeFace_MAX; cubeFace++)
		{
			RHIRenderPassInfo RPInfo(effectiveColorRT.mTargetableTexture, ERenderTargetActions::DontLoad_Store, nullptr, 0, cubeFace);
			RHICmdList.beginRenderPass(RPInfo, TEXT("CopyCubemapToScratchCubemapRP"));
			const int2 sourceDimensions(sourceCubemapResource->getWidth(), sourceCubemapResource->getHeight());
			const IntRect viewRect(0, 0, effectiveSize, effectiveSize);
			RHICmdList.setViewport(0, 0, 0.0f, effectiveSize, effectiveSize, 1.0f);

			GraphicsPipelineStateInitializer graphicsPSOInit;
			RHICmdList.applyCachedRenderTargets(graphicsPSOInit);
			graphicsPSOInit.mRasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::getRHI();
			graphicsPSOInit.mDepthStencilState = TStaticDepthStencilState<false, CF_Always>::getRHI();
			graphicsPSOInit.mBlendState = TStaticBlendState<>::getRHI();

			TShaderMapRef<ScreenVS> vertexShader(getGlobalShaderMap(featureLevel));

			TShaderMapRef<CopyCubemapToCubefacePS> pixelShader(getGlobalShaderMap(featureLevel));

			graphicsPSOInit.mBoundShaderState.mVertexDeclarationRHI = GFilterVertexDeclaration.mVertexDeclarationRHI;
			graphicsPSOInit.mBoundShaderState.mVertexShaderRHI = GETSAFERHISHADER_VERTEX(*vertexShader);
			graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(*pixelShader);
			graphicsPSOInit.mPrimitiveType = PT_TriangleList;

			setGraphicsPipelineState(RHICmdList, graphicsPSOInit);
			pixelShader->setParameters(RHICmdList, sourceCubemapResource, cubeFace, bIsSkyLight, bLowerHeisphereIsBlack, sourceCubemapRotation, lowerhemisphereColor);

			drawRectangle(RHICmdList, viewRect.min.x, viewRect.min.y, viewRect.width(), viewRect.height(), 0, 0, sourceDimensions.x, sourceDimensions.y, int2(viewRect.width(), viewRect.height()), sourceDimensions, *vertexShader);
			RHICmdList.endRenderPass();
		}
	}

	void fullyResolveReflectionScratchCubes(RHICommandListImmediate& RHICmdList)
	{
		SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);
		TextureRHIRef& scratch0 = sceneContext.mReflectionColorScratchCubemap[0]->getRenderTargetItem().mTargetableTexture;
		TextureRHIRef& scratch1 = sceneContext.mReflectionColorScratchCubemap[1]->getRenderTargetItem().mTargetableTexture;
		ResolveParams resolveParams(ResolveRect(), CubeFace_PosX, -1, -1, -1);
		RHICmdList.copyToResolveTarget(scratch0, scratch0, resolveParams);
		RHICmdList.copyToResolveTarget(scratch1, scratch1, resolveParams);
	}

	float computeSingleAverageBrightnessFromCubemap(RHICommandListImmediate& RHICmdList, ERHIFeatureLevel::Type featureLevel, int32 targetSize, SceneRenderTargetItem& cubemap)
	{
		TRefCountPtr<IPooledRenderTarget> reflectionBrightnessTarget;
		PooledRenderTargetDesc desc(PooledRenderTargetDesc::create2DDesc(int2(1, 1), PF_FloatRGBA, ClearValueBinding::None, TexCreate_None, TexCreate_RenderTargetable, false));
		GRenderTargetPool.findFreeElement(RHICmdList, desc, reflectionBrightnessTarget, TEXT("ReflectionBrightness"));

		TextureRHIRef& brightnessTarget = reflectionBrightnessTarget->getRenderTargetItem().mTargetableTexture;
		
		RHIRenderPassInfo RPInfo(brightnessTarget, ERenderTargetActions::Load_Store);
		transitionRenderPassTargets(RHICmdList, RPInfo);
		RHICmdList.beginRenderPass(RPInfo, TEXT("ReflectionBrightness"));
		{
			GraphicsPipelineStateInitializer graphicsPSOInit;
			RHICmdList.applyCachedRenderTargets(graphicsPSOInit);
			graphicsPSOInit.mRasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::getRHI();
			graphicsPSOInit.mDepthStencilState = TStaticDepthStencilState<false, CF_Always>::getRHI();
			graphicsPSOInit.mBlendState = TStaticBlendState<>::getRHI();
			auto shaderMap = getGlobalShaderMap(featureLevel);
			TShaderMapRef<PostProcessVS> vertexShader(shaderMap);
			TShaderMapRef<ComputeBrightnessPS> pixelShader(shaderMap);

			graphicsPSOInit.mBoundShaderState.mVertexDeclarationRHI = GFilterVertexDeclaration.mVertexDeclarationRHI;
			graphicsPSOInit.mBoundShaderState.mVertexShaderRHI = GETSAFERHISHADER_VERTEX(*vertexShader);
			graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(*pixelShader);
			graphicsPSOInit.mPrimitiveType = PT_TriangleList;

			setGraphicsPipelineState(RHICmdList, graphicsPSOInit);

			pixelShader->setParameters(RHICmdList, targetSize, cubemap);

			drawRectangle(RHICmdList, 0, 0, 1, 1, 0, 0, 1, 1, int2(1, 1), int2(1, 1), *vertexShader);
		}

		RHICmdList.endRenderPass();
		RHICmdList.copyToResolveTarget(brightnessTarget, brightnessTarget, ResolveParams());
	
		SceneRenderTargetItem& effectiveRT = reflectionBrightnessTarget->getRenderTargetItem();
		BOOST_ASSERT(effectiveRT.mShaderResourceTexture->getFormat() == PF_FloatRGBA);

		TArray<Float16Color> surfaceData;
		RHICmdList.readSurfaceFloatData(effectiveRT.mShaderResourceTexture, IntRect(0, 0, 1, 1), surfaceData, CubeFace_PosX, 0, 0);

		float averageBrightness = surfaceData[0].R.getFloat();
		return averageBrightness;
	}

	void createCubeMips(RHICommandListImmediate& RHICmdList, ERHIFeatureLevel::Type featureLevel, int32 numMips, SceneRenderTargetItem& cubemap)
	{
		RHITexture* cubeRef = cubemap.mTargetableTexture.getReference();

		auto* shaderMap = getGlobalShaderMap(featureLevel);

		GraphicsPipelineStateInitializer graphicsPSOInit;
		graphicsPSOInit.mRasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::getRHI();
		graphicsPSOInit.mDepthStencilState = TStaticDepthStencilState<false, CF_Always>::getRHI();
		graphicsPSOInit.mBlendState = TStaticBlendState<>::getRHI();

		for (int32 mipIndex = 1; mipIndex < numMips; mipIndex++)
		{
			const int32 mipSize = 1 << (numMips - mipIndex - 1);
			for (int32 cubeFace = 0; cubeFace < CubeFace_MAX; cubeFace++)
			{
				RHIRenderPassInfo RPInfo(cubemap.mTargetableTexture, ERenderTargetActions::DontLoad_Store, nullptr, mipIndex, cubeFace);
				RPInfo.bGeneratingMips = true;
				RHICmdList.beginRenderPass(RPInfo, TEXT("CreateCubeMaps"));
				RHICmdList.applyCachedRenderTargets(graphicsPSOInit);

				const IntRect viewRect(0, 0, mipSize, mipSize);
				RHICmdList.setViewport(0, 0, 0.0f, mipSize, mipSize, 1.0f);

				TShaderMapRef<ScreenVS> vertexShader(shaderMap);
				TShaderMapRef<CubeFilterPS> pixelShader(shaderMap);

				graphicsPSOInit.mBoundShaderState.mVertexDeclarationRHI = GFilterVertexDeclaration.mVertexDeclarationRHI;
				graphicsPSOInit.mBoundShaderState.mVertexShaderRHI = GETSAFERHISHADER_VERTEX(*vertexShader);
				graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(*pixelShader);
				graphicsPSOInit.mPrimitiveType = PT_TriangleList;

				setGraphicsPipelineState(RHICmdList, graphicsPSOInit);

				{
					RHIPixelShader* shaderRHI = pixelShader->getPixelShader();
					setShaderValue(RHICmdList, shaderRHI, pixelShader->mCubeFace, cubeFace);
					setShaderValue(RHICmdList, shaderRHI, pixelShader->mSourceMipIndex, mipIndex);
					setShaderValue(RHICmdList, shaderRHI, pixelShader->mNumMips, numMips);

					setSRVParameter(RHICmdList, shaderRHI, pixelShader->mSourceTexture, cubemap.MipSRVs[mipIndex - 1]);
					setSamplerParameter(RHICmdList, shaderRHI, pixelShader->mSourceTextureSampler, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::getRHI());
				}

				drawRectangle(RHICmdList, viewRect.min.x, viewRect.min.y,
					viewRect.width(), viewRect.height(),
					viewRect.min.x, viewRect.min.y,
					viewRect.width(), viewRect.height(),
					int2(viewRect.width(), viewRect.height()),
					int2(mipSize, mipSize),
					*vertexShader);
				RHICmdList.endRenderPass();
			}
		}

		RHICmdList.transitionResources(EResourceTransitionAccess::EReadable, &cubeRef, 1);
	}

	void computeAverageBrightness(RHICommandListImmediate& RHICmdList, ERHIFeatureLevel::Type featureLevel, int32 cubemapSize, float& outAverageBrightness)
	{
		const int32 effecitveToMipSize = cubemapSize;
		const int32 numMips = Math::ceilLogTwo(effecitveToMipSize) + 1;
		fullyResolveReflectionScratchCubes(RHICmdList);

		SceneRenderTargetItem& downSampledCube = SceneRenderTargets::get(RHICmdList).mReflectionColorScratchCubemap[0]->getRenderTargetItem();

		createCubeMips(RHICmdList, featureLevel, numMips, downSampledCube);

		outAverageBrightness = computeSingleAverageBrightnessFromCubemap(RHICmdList, featureLevel, cubemapSize, downSampledCube);
	}

	void filterReflectionEnvironment(RHICommandListImmediate& RHICmdList, ERHIFeatureLevel::Type featureLevel, int32 cubemapSize, SHVectorRGB3* outIrradianceEnvironmentMap)
	{
		const int32 effectiveTopMipSize = cubemapSize;
		const int32 numMips = Math::ceilLogTwo(effectiveTopMipSize) + 1;
		SceneRenderTargetItem& effectiveColorRT = SceneRenderTargets::get(RHICmdList).mReflectionColorScratchCubemap[0]->getRenderTargetItem();


		GraphicsPipelineStateInitializer graphicsPSOInit;
		graphicsPSOInit.mRasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::getRHI();
		graphicsPSOInit.mDepthStencilState = TStaticDepthStencilState<false, CF_Always>::getRHI();
		graphicsPSOInit.mBlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_Zero, BF_DestAlpha, BO_Add, BF_Zero, BF_One>::getRHI();


		RHICmdList.transitionResource(EResourceTransitionAccess::EWritable, effectiveColorRT.mTargetableTexture);

		for (uint32 cubeface = 0; cubeface < CubeFace_MAX; cubeface++)
		{
			RHIRenderPassInfo RPInfo(effectiveColorRT.mTargetableTexture, ERenderTargetActions::Load_Store, nullptr, 0, cubeface);

			RHICmdList.beginRenderPass(RPInfo, TEXT("FileterReflectionEnvironmentRP"));
			RHICmdList.applyCachedRenderTargets(graphicsPSOInit);

			const int2 sourceDimension(cubemapSize, cubemapSize);
			const IntRect viewRect(0, 0, effectiveTopMipSize, effectiveTopMipSize);
			RHICmdList.setViewport(0, 0, 0.0f, effectiveTopMipSize, effectiveTopMipSize, 1.0f);

			TShaderMapRef<ScreenVS> vertexShader(getGlobalShaderMap(featureLevel));
			TShaderMapRef<OneColorPS> pixelShader(getGlobalShaderMap(featureLevel));

			graphicsPSOInit.mBoundShaderState.mVertexDeclarationRHI = GFilterVertexDeclaration.mVertexDeclarationRHI;
			graphicsPSOInit.mBoundShaderState.mVertexShaderRHI = GETSAFERHISHADER_VERTEX(*vertexShader);
			graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(*pixelShader);
			graphicsPSOInit.mPrimitiveType = PT_TriangleList;
		
			setGraphicsPipelineState(RHICmdList, graphicsPSOInit);

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
			RHICmdList.endRenderPass();
		}

		RHICmdList.transitionResource(EResourceTransitionAccess::EReadable, effectiveColorRT.mTargetableTexture);

		auto shaderMap = getGlobalShaderMap(featureLevel);
		SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);

		SceneRenderTargetItem& downSampledCube = SceneRenderTargets::get(RHICmdList).mReflectionColorScratchCubemap[0]->getRenderTargetItem();
		SceneRenderTargetItem& filteredCube = SceneRenderTargets::get(RHICmdList).mReflectionColorScratchCubemap[1]->getRenderTargetItem();
		createCubeMips(RHICmdList, featureLevel, numMips, downSampledCube);

		if (outIrradianceEnvironmentMap)
		{
			const int32 numDiffuseMips = Math::ceilLogTwo(GDiffuseIrradianceCubemapSize) + 1;
			const int32 diffuseConvolutionSourceMip = numMips - numDiffuseMips;
			
			computeDiffuseIrradiance(RHICmdList, featureLevel, downSampledCube.mShaderResourceTexture, diffuseConvolutionSourceMip, outIrradianceEnvironmentMap);
		}
		{
			graphicsPSOInit.mRasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::getRHI();
			graphicsPSOInit.mDepthStencilState = TStaticDepthStencilState<false, CF_Always>::getRHI();
			graphicsPSOInit.mBlendState = TStaticBlendState<>::getRHI();

			RHICmdList.transitionResource(EResourceTransitionAccess::EWritable, filteredCube.mTargetableTexture);

			for (int32 mipIndex = 0; mipIndex < numMips; mipIndex++)
			{
				
				const int32 mipSize = 1 << (numMips - mipIndex - 1);

				for (int32 cubeface = 0; cubeface < CubeFace_MAX; cubeface++)
				{
					RHIRenderPassInfo RPInfo(filteredCube.mTargetableTexture, ERenderTargetActions::DontLoad_Store, nullptr, mipIndex, cubeface);
					RHICmdList.beginRenderPass(RPInfo, TEXT("FilterMips"));
					RHICmdList.applyCachedRenderTargets(graphicsPSOInit);
					const IntRect viewRect(0, 0, mipSize, mipSize);
					RHICmdList.setViewport(0, 0, 0.0f, mipSize, mipSize, 1.0f);

					TShaderMapRef<ScreenVS> vertexShader(getGlobalShaderMap(featureLevel));
					TShaderMapRef<TCubeFilterPS<1>> captureCubemapArrayPixelShader(getGlobalShaderMap(featureLevel));

					CubeFilterPS* pixelShader;

					pixelShader = *TShaderMapRef<TCubeFilterPS<0>>(shaderMap);

					BOOST_ASSERT(pixelShader);
					graphicsPSOInit.mBoundShaderState.mVertexDeclarationRHI = GFilterVertexDeclaration.mVertexDeclarationRHI;
					graphicsPSOInit.mBoundShaderState.mVertexShaderRHI = GETSAFERHISHADER_VERTEX(*vertexShader);
					graphicsPSOInit.mBoundShaderState.mPixelShaderRHI = GETSAFERHISHADER_PIXEL(pixelShader);
					graphicsPSOInit.mPrimitiveType = PT_TriangleList;

					setGraphicsPipelineState(RHICmdList, graphicsPSOInit);

					{
						RHIPixelShader* shaderRHI = pixelShader->getPixelShader();
						setShaderValue(RHICmdList, shaderRHI, pixelShader->mCubeFace, cubeface);
						setShaderValue(RHICmdList, shaderRHI, pixelShader->mNumMips, numMips);
						setShaderValue(RHICmdList, shaderRHI, pixelShader->mSourceMipIndex, mipIndex);
						setTextureParameter(RHICmdList, shaderRHI, pixelShader->mSourceTexture, pixelShader->mSourceTextureSampler, TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::getRHI(), downSampledCube.mShaderResourceTexture);
					}

					drawRectangle(RHICmdList,
						viewRect.min.x, viewRect.min.y,
						viewRect.width(), viewRect.height(),
						viewRect.min.x, viewRect.min.y,
						viewRect.width(), viewRect.height(),
						int2(viewRect.width(), viewRect.height()),
						int2(mipSize, mipSize),
						*vertexShader);

					RHICmdList.endRenderPass();
				}
			}

			RHICmdList.copyToResolveTarget(filteredCube.mTargetableTexture, filteredCube.mShaderResourceTexture, ResolveParams());
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
					RHICmdList.copyToResolveTarget(effectiveSource.mShaderResourceTexture, processedTexture->mTextureRHI, ResolveParams(ResolveRect(), (ECubeFace)cubeface, mipIndex, 0, 0));
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
			int32 cubemapSize = captureComponent->mCubemapResolution;

			ENQUEUE_RENDER_COMMAND(
				ClearCommand)([cubemapSize](RHICommandListImmediate& RHICmdList)
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
				int32 cubemapSize = captureComponent->mCubemapResolution;
				bool bLowerHemisphereIsBlack = captureComponent->bLowerHemisphereIsBlack;
				float sourceCubemapRotation = captureComponent->mSourceCubemapAngle * (PI / 180.f);
				ERHIFeatureLevel::Type featureLevel = getFeatureLevel();

				LinearColor lowerHemisphereColor = captureComponent->lowerHemisphereColor;

				ENQUEUE_RENDER_COMMAND(
					CopyCubemapCommand)([sourceCubemap, cubemapSize, bLowerHemisphereIsBlack, sourceCubemapRotation, featureLevel, lowerHemisphereColor](RHICommandListImmediate& RHICmdList)
					{
						copyCubemapToScratchCubemap(RHICmdList, featureLevel, sourceCubemap.get(), cubemapSize, true, bLowerHemisphereIsBlack, sourceCubemapRotation, lowerHemisphereColor);
					}
				);
			}
			else
			{
				BOOST_ASSERT(false);
			}

			cubemapSize = captureComponent->mCubemapResolution;

			ERHIFeatureLevel::Type featureLevel = getFeatureLevel();

			ENQUEUE_RENDER_COMMAND(
				FilterCommand)([cubemapSize, &outAverageBrightness, &outIrradianceEnvironmentMap,
					featureLevel](RHICommandListImmediate& RHICmdList)
				{
					computeAverageBrightness(RHICmdList, featureLevel, cubemapSize, outAverageBrightness);
					filterReflectionEnvironment(RHICmdList, featureLevel, cubemapSize, &outIrradianceEnvironmentMap);
				}
			);

			if (outProcessedTexture)
			{
				ENQUEUE_RENDER_COMMAND(
					CopyCommand)([this, outProcessedTexture](RHICommandListImmediate& RHICmdList)
					{
						copyToSkyTexture(RHICmdList, this, outProcessedTexture);
					}
				);
			}
		}

	}
}