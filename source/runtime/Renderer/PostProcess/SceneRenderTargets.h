#pragma once
#include "RendererMininal.h"
#include "RenderResource.h"
#include "RendererInterface.h"
#include "RenderUtils.h"
#include "RHI.h"
namespace Air
{
	class ViewInfo;

	class SceneRenderer;

	BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(GBufferResourceStruct, )
		SHADER_PARAMETER_TEXTURE(Texture2D, GBufferATexture)
		SHADER_PARAMETER_TEXTURE(Texture2D, GBufferBTexture)
		SHADER_PARAMETER_TEXTURE(Texture2D, GBufferCTexture)
		SHADER_PARAMETER_TEXTURE(Texture2D, GBufferDTexture)
		SHADER_PARAMETER_TEXTURE(Texture2D, GBufferETexture)
		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, GBufferATextureNonMS)
		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, GBufferBTextureNonMS)
		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, GBufferCTextureNonMS)
		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, GBufferDTextureNonMS)
		SHADER_PARAMETER_TEXTURE(Texture2D<float4>, GBufferETextureNonMS)
		SHADER_PARAMETER_TEXTURE(Texture2DMS<float4>, GBufferATextureMS)
		SHADER_PARAMETER_TEXTURE(Texture2DMS<float4>, GBufferBTextureMS)
		SHADER_PARAMETER_TEXTURE(Texture2DMS<float4>, GBufferCTextureMS)
		SHADER_PARAMETER_TEXTURE(Texture2DMS<float4>, GBufferDTextureMS)
		SHADER_PARAMETER_TEXTURE(Texture2DMS<float4>, GBufferETextureMS)
		SHADER_PARAMETER_SAMPLER(SamplerState, GBufferATextureSampler)
		SHADER_PARAMETER_SAMPLER(SamplerState, GBufferBTextureSampler)
		SHADER_PARAMETER_SAMPLER(SamplerState, GBufferCTextureSampler)
		SHADER_PARAMETER_SAMPLER(SamplerState, GBufferDTextureSampler)
		SHADER_PARAMETER_SAMPLER(SamplerState, GBufferETextureSampler)
	END_GLOBAL_SHADER_PARAMETER_STRUCT(GBufferResourceStruct)

	enum class ESceneColorFormatType
	{
		Mobile,
		HighEnd,
		HighEndWithAlpha,
		Num
	};

	inline int32 getTranslucencyLightingVolumeDim()
	{
		extern int32 GTranslucencyLightingVolumeDim;
		return Math::clamp(GTranslucencyLightingVolumeDim, 4, 2048);
	}

	static const int32 NumTranslucentVolumeRenderTargetSets = (TVC_Max + 1);

	class RENDERER_API SceneRenderTargets : public RenderResource
	{
	public:
		static SceneRenderTargets& get(RHICommandList& RHICmdList);

		static SceneRenderTargets& get(RHICommandListImmediate& RHICmdList);

		static SceneRenderTargets& get(RHIAsyncComputeCommandListImmediate& RHICmdList);

		void releaseSceneColor();

		void allocate(RHICommandList& RHICmdList, const SceneRenderer* sceneRenderer);

		void setBufferSize(int32 width, int32 height);

		const int2 getBufferSize() const;

		int2 getBufferSizeXY() const { return mBufferSize; }

		const TextureRHIRef& getSceneColorTexture() const;

		const Texture2DRHIRef* getActualDepthTexture() const;

		const Texture2DRHIRef& getSceneDepthTexture() const { return (const Texture2DRHIRef&)mSceneDepthZ->getRenderTargetItem().mShaderResourceTexture; }

		const TextureRHIRef& getEffectiveLightAttenuationTexture(bool bReceiveDynamicShadows) const
		{
			if (bLightAttenuationEnabled && bReceiveDynamicShadows)
			{
				return getLightAttenuationTexture();
			}
			else
			{
				return GWhiteTexture->mTextureRHI;
			}
		}

		const Texture2DRHIRef& getAuxiliarySceneDepthSurface() const
		{
			BOOST_ASSERT(!GSupportsDepthFetchDuringDepthTest);
			return (const Texture2DRHIRef&)mAuxiliarySceneDepthZ->getRenderTargetItem().mTargetableTexture;
		}

		const Texture2DRHIRef& getAuxiliarySceneDepthTexture() const
		{
			BOOST_ASSERT(!GSupportsDepthFetchDuringDepthTest);
			return (const Texture2DRHIRef&)mAuxiliarySceneDepthZ->getRenderTargetItem().mShaderResourceTexture;
		}
		bool isSeparateTranslucencyPass() { return bSeparateTranslucencyPass; }

		bool isSeparateTranslucencyDepthValid()
		{
			return mSeparateTranslucencyDepthRT != nullptr;
		}

		void getSeparateTranslucencyDimensions(int2& outScaleSize, float& outScale)
		{
			outScaleSize = mSeparateTranslucencyBufferSize;
			outScale = mSeparateTranslucencyScale;
		}

		const Texture2DRHIRef& getSeparateTranslucencyDepthSurface()
		{
			return (const Texture2DRHIRef&)mSeparateTranslucencyDepthRT->getRenderTargetItem().mTargetableTexture;
		}

		ESceneColorFormatType getSceneColorFormatType() const;

		bool areRenderTargetClearsValid(ESceneColorFormatType inSceneColorFormatType) const;

		int32 getMSAACount() const { return mCurrentMSAACount; }

		void destroyAllSnapshots();

		void beginRenderingGBuffer(RHICommandList& RHICmdList, ERenderTargetLoadAction colorLoadAction, ERenderTargetLoadAction depthLoadAction, FExclusiveDepthStencil::Type depthStencilAccess, bool bBindQuadOverdrawBuffers, bool bClearQuadOverdrawBuffers = false, const LinearColor& clearColor = LinearColor(0, 0, 0, 1), bool bIsWireframe = false);
		void finishGBufferPassAndResolve(RHICommandListImmediate& RHICmdList);


		void beginRenderingSceneColor(RHICommandList& RHICmdList, ESimpleRenderTargetMode renderTargetMode = ESimpleRenderTargetMode::EUninitializedColorExistingDepth, FExclusiveDepthStencil depthStencilAccess = FExclusiveDepthStencil::DepthWrite_StencilWrite, bool bTransitionWritable = true);

		void preallocGBufferTargets();

		const TRefCountPtr<IPooledRenderTarget>& getLightAttenuation() const;

		const TextureRHIRef& getLightAttenuationTexture() const
		{
			return *(TextureRHIRef*)&getLightAttenuation()->getRenderTargetItem().mShaderResourceTexture;
		}

		void allocGBufferTargets(RHICommandList& RHICmdList);

		void allocateReflectionTargets(RHICommandList& RHICmdList, int32 targetSize);

		RHIConstantBuffer* getGBufferResourcesConstantBuffer() const
		{
			return mGBufferResourcesConstantBuffer;
		}

		//前面多个视图都渲染到了同一个renderTarget上，后处理之前需要把各自的部分拷贝出来

		ERHIFeatureLevel::Type getCurrentFeatureLevel() const { return mCurrentFeatureLevel; }

		bool isSceneColorAllocated() const;
		
		void setSceneColor(IPooledRenderTarget* in);

		void adjustGBufferRefCount(RHICommandList& RHICmdList, int delta);

		void resolveSceneDepthTexture(RHICommandList& RHICmdList, const ResolveRect& resolveRect);

		void resolveSceneDepthToAuxiliaryTexture(RHICommandList& RHICmdList);

		bool isDownsampleTranslucencyDepthValid()
		{
			return mDownsampledTranslucencyDepthRT != nullptr;
		}

		void clearTranslucentVolumeLighting(RHICommandListImmediate& RHICmdList, int32 viewIndex);

		const Texture2DRHIRef& getDownsampledTranslucencyDepthSurface()
		{
			return (const Texture2DRHIRef&)mDownsampledTranslucencyDepthRT->getRenderTargetItem().mTargetableTexture;
		}
	public:
		int2 computeDesiredSize(const SceneViewFamily& viewFamily);
		void updateRHI();
		void allocateRenderTargets(RHICommandList& RHICmdList);
		bool areShadingPathRenderTargetsAllocated(ESceneColorFormatType inSceneColorFormatType) const;

		void allocateMobileRenderTargets(RHICommandList& RHICmdList);

		void allocateDeferredShadingPathRenderTarget(RHICommandList& RHICmdList);

		void allocateCommonDepthTargets(RHICommandList& RHICmdList);

		const TRefCountPtr<IPooledRenderTarget>& getSceneColorForCurrentShadingPath() const
		{
			BOOST_ASSERT(mCurrentShadingPath < EShadingPath::Num);
			return mSceneColor[(int32)getSceneColorFormatType()];
		}

		TRefCountPtr<IPooledRenderTarget>& getSceneColorForCurrentShadingPath()
		{
			BOOST_ASSERT(mCurrentShadingPath < EShadingPath::Num);
			return mSceneColor[(int32)getSceneColorFormatType()];
		}

		EPixelFormat getSceneColorFormat() const;

		uint16 getNumSceneColorMSAASamples(ERHIFeatureLevel::Type inFeatureLevel);

		void allocSceneColor(RHICommandList& RHICmdList);

		const TextureRHIRef& getSceneColorSurface() const;

		const Texture2DRHIRef& getSceneDepthSurface() const 
		{
			return (const Texture2DRHIRef&)mSceneDepthZ->getRenderTargetItem().mTargetableTexture;
		}

		int32 getGBufferRenderTargets(ERenderTargetLoadAction colorLoadAction, RHIRenderTargetView outRenderTargets[MaxSimultaneousRenderTargets], int32& outVelocityRTIndex);

		const TRefCountPtr<IPooledRenderTarget>& getSceneColor() const;

		TRefCountPtr<IPooledRenderTarget>& getSceneColor();

		void setLightAttenuationMode(bool bEnabled) { bLightAttenuationEnabled = bEnabled; }

		void setLightAttenuation(IPooledRenderTarget* in)
		{
			mLightAttenuation = in;
		}

		void finishRenderingLightAttenuation(RHICommandList& RHICmdList);

		template<int32 NumRenderTargets>
		static void clearVolumeTextures(RHICommandList& RHICmdList, ERHIFeatureLevel::Type featureLevel, RHITexture** renderTargets, const LinearColor* clearColors);
	private:
		void releaseGBufferTargets();

		int32 fillGBufferRenderPassInfo(ERenderTargetLoadAction colorLoadAction, RHIRenderPassInfo& outRenderPassInfo, int32& outVelocityRTIndex);
		void setQuadOverdrawUAV(RHICommandList& RHICmdList, bool bBindQuadOverdrawBuffers, bool bClearQuadOverdrawBuffers, RHIRenderPassInfo& info);
	public:
		TRefCountPtr<IPooledRenderTarget> mSceneColor[(int32)ESceneColorFormatType::Num];

		TRefCountPtr<IPooledRenderTarget> mLightAttenuation;

		TRefCountPtr<IPooledRenderTarget> mSceneMonoColor;
		TRefCountPtr<IPooledRenderTarget> mSceneMonoDepthZ;

		TRefCountPtr<IPooledRenderTarget> mSceneDepthZ;
		TRefCountPtr<IPooledRenderTarget> mSmallDepthZ;
		TRefCountPtr<RHIShaderResourceView> mSceneStencilSRV;
		TRefCountPtr<IPooledRenderTarget> mAuxiliarySceneDepthZ;
		TRefCountPtr<IPooledRenderTarget> mSeparateTranslucencyRT;
		TRefCountPtr<IPooledRenderTarget> mSeparateTranslucencyDepthRT;

		TRefCountPtr<IPooledRenderTarget> mDownsampledTranslucencyDepthRT;

		TRefCountPtr<IPooledRenderTarget> mGBufferA;
		TRefCountPtr<IPooledRenderTarget> mGBufferB;
		TRefCountPtr<IPooledRenderTarget> mGBufferC;
		TRefCountPtr<IPooledRenderTarget> mGBufferD;
		TRefCountPtr<IPooledRenderTarget> mGBufferE;
		TRefCountPtr<IPooledRenderTarget> mGBufferVelocity;

		TRefCountPtr<IPooledRenderTarget> mReflectionColorScratchCubemap[2];

		TRefCountPtr<IPooledRenderTarget> mDiffuseIrradianceScratchCubemap[2];

		TRefCountPtr<IPooledRenderTarget> mSkySHIrradianceMap;

		TRefCountPtr<IPooledRenderTarget> mSceneVelocity;

		TArray<TRefCountPtr<IPooledRenderTarget>, TInlineAllocator<NumTranslucentVolumeRenderTargetSets>> mTranslucencyLightingVolumeAmbient;
		TArray<TRefCountPtr<IPooledRenderTarget>, TInlineAllocator<NumTranslucentVolumeRenderTargetSets>> mTranslucencyLightingVolumeDirectional;

		

		TRefCountPtr<IPooledRenderTarget> mScreenSpaceAO;

		ConstantBufferRHIRef mGBufferResourcesConstantBuffer;

		EShadingPath	mCurrentShadingPath{ EShadingPath::Num };
		ERHIFeatureLevel::Type mCurrentFeatureLevel{ ERHIFeatureLevel::Num };
		int2 mBufferSize{ 0, 0 };
		int2 mSeparateTranslucencyBufferSize{ 0 ,0 };
		float mSeparateTranslucencyScale{ 1 };
		int2 mLargestDesiredSizeThisFrame;
		int2 mLargestDesiredSizeLastFrame;
		int32 mCurrentGBufferFormat{ 0 };
		uint32 mSmallColorDepthDownsampleFactor{ 1 };
		bool bRequireSceneColorAlpha{ false };
		bool bGBufferFastCleared;
		bool bAllocateVelocityGBuffer{ false };
		bool bAllowStaticLighting{ false };
		bool bSceneDepthCleared;
		bool bSeparateTranslucencyPass{ false };
		bool bScreenSpaceAOIsValid{ false };
		uint32 mThisFrameNumber;
		int32 mCurrentSceneColorFormat{ 0 };
		int32 mCurrentMSAACount;
		ClearValueBinding mDefaultColorClear{ ClearValueBinding::Black };
		ClearValueBinding mDefaultDepthClear{ ClearValueBinding::DepthFar };
	protected:
		SceneRenderTargets()
			:mLargestDesiredSizeThisFrame(0, 0),
			mLargestDesiredSizeLastFrame(0, 0),
			mThisFrameNumber(0),
			mBufferSize(0, 0)
		{}

		SceneRenderTargets(const ViewInfo& inView, const SceneRenderTargets& snapshotSource);

		void getGBufferADesc(PooledRenderTargetDesc& desc) const;

	private:
		bool bLightAttenuationEnabled;
		int32 mGBufferRefCount{ 0 };
		int32 mQuadOverdrawIndex;
	};


}