#include "PostProcess/SceneRenderTargets.h"
#include "SceneView.h"
#include "RenderUtils.h"
#include "HAL/PlatformProperties.h"
#include "AirEngine.h"
#include "PostProcess/RenderTargetPool.h"
#include "RenderResource.h"
#include "SystemTextures.h"
#include "RHIUtilities.h"
#include "RHIStaticStates.h"
#include "GlobalShader.h"
#include "StaticBoundShaderState.h"
#include "RHICommandList.h"
#include "HdrCustomResolveShaders.h"
namespace Air
{
	static TGlobalResource<SceneRenderTargets> mSceneRenderTargetSingleton;
	extern int32 GDiffuseIrradianceCubemapSize;

	IMPLEMENT_CONSTANT_BUFFER_STRUCT(GBufferResourceStruct, TEXT("GBuffers"));

	SceneRenderTargets& SceneRenderTargets::get(RHICommandList& RHICmdList)
	{
		BOOST_ASSERT(isInRenderingThread() && !RHICmdList.getRenderThreadContext(RHICommandListBase::ERenderThreadContext::SceneRenderTargets) && !TaskGraphInterface::get().isThreadProcessingTasks(ENamedThreads::RenderThread_Local));
		return mSceneRenderTargetSingleton;
	}

	SceneRenderTargets& SceneRenderTargets::get(RHICommandListImmediate& RHICmdList)
	{
		BOOST_ASSERT(isInRenderingThread() && !RHICmdList.getRenderThreadContext(RHICommandListBase::ERenderThreadContext::SceneRenderTargets) && !TaskGraphInterface::get().isThreadProcessingTasks(ENamedThreads::RenderThread_Local));

		return mSceneRenderTargetSingleton;
	}

	SceneRenderTargets& SceneRenderTargets::get(RHIAsyncComputeCommandListImmediate& RHICmdList)
	{
		BOOST_ASSERT(isInRenderingThread() && !RHICmdList.getRenderThreadContext(RHICommandListBase::ERenderThreadContext::SceneRenderTargets) && !TaskGraphInterface::get().isThreadProcessingTasks(ENamedThreads::RenderThread_Local));

		return mSceneRenderTargetSingleton;
	}

	void SceneRenderTargets::releaseSceneColor()
	{
		for (auto i = 0; i < (int32)ESceneColorFormatType::Num; ++i)
		{
			mSceneColor[i].safeRelease();
		}
		if (mSceneMonoColor)
		{
			mSceneMonoColor.safeRelease();
			mSceneMonoDepthZ.safeRelease();
		}
	}

	void SceneRenderTargets::allocate(RHICommandList& RHICmdList, const SceneViewFamily& viewFamily)
	{
		BOOST_ASSERT(isInRenderingThread());
		const auto newFeatureLevel = viewFamily.mScene->getFeatureLevel();
		mCurrentShadingPath = viewFamily.mScene->getShadingPath();
		bRequireSceneColorAlpha = false;
		for (int32 viewIndex = 0; viewIndex < viewFamily.mViews.size(); viewIndex++)
		{
			if (viewFamily.mViews[viewIndex]->bIsPlanarRefelction || viewFamily.mViews[viewIndex]->bIsSeneCapture)
			{
				bRequireSceneColorAlpha = true;
			}
		}

		int2 DesiredBufferSize = computeDesiredSize(viewFamily);
		
		quantizeSceneBufferSize(DesiredBufferSize.x, DesiredBufferSize.y);
		if ((mBufferSize.x != DesiredBufferSize.x) ||
			(mBufferSize.y != DesiredBufferSize.y))
		{
			setBufferSize(DesiredBufferSize.x, DesiredBufferSize.y);
			updateRHI();
		}
		mCurrentFeatureLevel = newFeatureLevel;
		allocateRenderTargets(RHICmdList);
	}

	int2 SceneRenderTargets::computeDesiredSize(const SceneViewFamily& viewFamily)
	{
		enum ESizingMethods { RequestedSize, ScreenRes, Grow, VisibleSizingMethodsCount };
		ESizingMethods sceneTargetsSizingMethod = Grow;
		bool bIsSceneCapture = false;
		bool bIsReflectionCapture = false;
		for (int32 viewIndex = 0, viewCount = viewFamily.mViews.size(); viewIndex < viewCount; ++viewIndex)
		{
			const SceneView* view = viewFamily.mViews[viewIndex];
			bIsSceneCapture |= view->bIsSeneCapture;
			bIsReflectionCapture |= view->bIsRefectionCapture;
		}
		if (!PlatformProperties::supportsWindowedMode())
		{
			sceneTargetsSizingMethod = RequestedSize;
		}
		else
		{
		}
		int2 desiredBufferSize = int2::Zero;
		switch (sceneTargetsSizingMethod)
		{
		case RequestedSize:
			desiredBufferSize = int2(viewFamily.mFamilySizeX, viewFamily.mFamilySizeY);
			break;
		case ScreenRes:
			desiredBufferSize = int2(GSystemResolution.mResX, GSystemResolution.mResY);
			break;
		case Grow:
			desiredBufferSize = int2(std::max<uint32>(getBufferSize().x, viewFamily.mFamilySizeX), std::max<uint32>(getBufferSize().y, viewFamily.mFamilySizeY));
			break;
		default:
			break;
		}
		{
			mLargestDesiredSizeThisFrame = mLargestDesiredSizeThisFrame.componentMax(desiredBufferSize);
			uint32 frameNumber = viewFamily.mFrameNumber;

			if (mThisFrameNumber != frameNumber)
			{
				mThisFrameNumber = frameNumber;
				mLargestDesiredSizeLastFrame = mLargestDesiredSizeThisFrame;
				mLargestDesiredSizeThisFrame = int2(0, 0);
			}
			desiredBufferSize = desiredBufferSize.componentMax(mLargestDesiredSizeLastFrame);
		}
		return desiredBufferSize;
	}

	void SceneRenderTargets::allocateRenderTargets(RHICommandList& RHICmdList)
	{
		if (mBufferSize.x > 0 && mBufferSize.y > 0 && (!areShadingPathRenderTargetsAllocated(getSceneColorFormatType()) || !areRenderTargetClearsValid(getSceneColorFormatType())))
		{
			if ((EShadingPath)mCurrentShadingPath == EShadingPath::Mobile)
			{
				allocateMobileRenderTargets(RHICmdList);
			}
			else
			{
				allocateDeferredShadingPathRenderTarget(RHICmdList);
			}
		}
	}

	bool SceneRenderTargets::areShadingPathRenderTargetsAllocated(ESceneColorFormatType inSceneColorFormatType) const
	{
		switch (inSceneColorFormatType)
		{
		case Air::ESceneColorFormatType::Mobile:
		{
			return (mSceneColor[(int32)ESceneColorFormatType::Mobile] != nullptr);
		}
		case Air::ESceneColorFormatType::HighEnd:
		{
			return (mSceneColor[(int32)ESceneColorFormatType::HighEnd] != nullptr);
		}
		case Air::ESceneColorFormatType::HighEndWithAlpha:
		{
			return (mSceneColor[(int32)ESceneColorFormatType::HighEndWithAlpha] != nullptr);
		}
		default:
		{
			BOOST_ASSERT(false);
			return false;
		}
		}
	}

	void SceneRenderTargets::allocateMobileRenderTargets(RHICommandList& RHICmdList)
	{

	}

	void SceneRenderTargets::allocateDeferredShadingPathRenderTarget(RHICommandList& RHICmdList)
	{
		allocateCommonDepthTargets(RHICmdList);
		{
			int2 smallDepthZSize(std::max<uint32>(mBufferSize.x / mSmallColorDepthDownsampleFactor, 1), std::max<uint32>(mBufferSize.y / mSmallColorDepthDownsampleFactor, 1));

			PooledRenderTargetDesc desc(PooledRenderTargetDesc::create2DDesc(smallDepthZSize, PF_DepthStencil, ClearValueBinding::None, TexCreate_None, TexCreate_DepthStencilTargetable, true));
			GRenderTargetPool.findFreeElement(RHICmdList, desc, mSmallDepthZ, TEXT("SmallDepthZ"));

			if (mCurrentFeatureLevel >= ERHIFeatureLevel::SM4)
			{

			}
		}
	}
	void SceneRenderTargets::allocateCommonDepthTargets(RHICommandList& RHICmdList)
	{
		if (mSceneDepthZ && !(mSceneDepthZ->getRenderTargetItem().mTargetableTexture->getClearBinding() == mDefaultDepthClear))
		{
			uint32 stencilCurrent, stencilNew;
			float depthCurrent, depthNew;
			mSceneDepthZ->getRenderTargetItem().mTargetableTexture->getClearBinding().getDepthStencil(depthCurrent, stencilCurrent);
			mDefaultDepthClear.getDepthStencil(depthNew, stencilNew);
			mSceneDepthZ.safeRelease();
		}
		if (!mSceneDepthZ)
		{
			PooledRenderTargetDesc desc(PooledRenderTargetDesc::create2DDesc(mBufferSize, PF_DepthStencil, mDefaultDepthClear, TexCreate_None, TexCreate_DepthStencilTargetable, false));
			desc.mNumSamples = getNumSceneColorMSAASamples(mCurrentFeatureLevel);
			desc.mFlags |= TexCreate_FastVRAM;
			GRenderTargetPool.findFreeElement(RHICmdList, desc, mSceneDepthZ, TEXT("SceneDepthZ"));
			mSceneStencilSRV = RHICreateShaderResourceView((Texture2DRHIRef&)mSceneDepthZ->getRenderTargetItem().mTargetableTexture, 0, 1, PF_X24_G8);
		}
		if (!mAuxiliarySceneDepthZ && !GSupportsDepthFetchDuringDepthTest)
		{
			PooledRenderTargetDesc desc(PooledRenderTargetDesc::create2DDesc(mBufferSize, PF_DepthStencil, mDefaultDepthClear, TexCreate_None, TexCreate_DepthStencilTargetable, false));
			desc.bAutoWritable = false;
			GRenderTargetPool.findFreeElement(RHICmdList, desc, mAuxiliarySceneDepthZ, TEXT("AuxiliarySceneDepthZ"));
		}
	}
	void SceneRenderTargets::updateRHI()
	{

	}

	void SceneRenderTargets::getGBufferADesc(PooledRenderTargetDesc& desc) const
	{
		const bool bHighPrecisionGBuffers = (mCurrentGBufferFormat >= EGBufferFormat::Force16BitsPerChannel);
		const bool bEnface8BitPerChannal = (mCurrentGBufferFormat == EGBufferFormat::Force8BitsPerChannel);
		{
			EPixelFormat normalGBufferFormat = bHighPrecisionGBuffers ? PF_FloatRGBA : PF_A2B10G10R10;
			if (bEnface8BitPerChannal)
			{
				normalGBufferFormat = PF_B8G8R8A8;
			}
			else if (mCurrentGBufferFormat == EGBufferFormat::HighPrecisionNormals)
			{
				normalGBufferFormat = PF_FloatRGBA;
			}

			desc = PooledRenderTargetDesc::create2DDesc(mBufferSize, normalGBufferFormat, ClearValueBinding::Transparent, TexCreate_None, TexCreate_RenderTargetable, false);
		}
	}

	bool SceneRenderTargets::areRenderTargetClearsValid(ESceneColorFormatType inSceneColorFormatType) const
	{
		switch (inSceneColorFormatType)
		{
		case Air::ESceneColorFormatType::Mobile:
		{
			const TRefCountPtr<IPooledRenderTarget>& sceneColorTarget = getSceneColorForCurrentShadingPath();
			const bool bColorValid = sceneColorTarget && (sceneColorTarget->getRenderTargetItem().mTargetableTexture->getClearBinding() == mDefaultDepthClear);
			const bool bDepthValid = mSceneDepthZ && (mSceneDepthZ->getRenderTargetItem().mTargetableTexture->getClearBinding() == mDefaultDepthClear);
			return bColorValid && bDepthValid;
		}
		default:
			return true;
		}
	}
	ESceneColorFormatType SceneRenderTargets::getSceneColorFormatType() const
	{
		if (mCurrentShadingPath == EShadingPath::Mobile)
		{
			return ESceneColorFormatType::Mobile;
		}
		else if (mCurrentShadingPath == EShadingPath::Deferred && (bRequireSceneColorAlpha || mCurrentSceneColorFormat == PF_FloatRGBA))
		{
			return ESceneColorFormatType::HighEndWithAlpha;
		}
		else if (mCurrentShadingPath == EShadingPath::Deferred && !bRequireSceneColorAlpha)
		{
			return ESceneColorFormatType::HighEnd;
		}
		BOOST_ASSERT(false);
		return ESceneColorFormatType::Num;
	}
	int2 const SceneRenderTargets::getBufferSize() const
	{
		return mBufferSize;
	}

	void SceneRenderTargets::setBufferSize(int32 width, int32 height)
	{
		mBufferSize.x = width;
		mBufferSize.y = height;
	}

	void SceneRenderTargets::destroyAllSnapshots()
	{

	}

	void SceneRenderTargets::beginRenderingGBuffer(RHICommandList& RHICmdList, ERenderTargetLoadAction colorLoadAction, ERenderTargetLoadAction depthLoadAction, bool bBindQuadOverdrawBuffers, const LinearColor& clearColor /* = LinearColor(0, 0, 0, 1) */)
	{
		if (isAnyForwardShadingEnabled(getFeatureLevelShaderPlatform(mCurrentFeatureLevel)))
		{
			beginRenderingSceneColor(RHICmdList, colorLoadAction == ERenderTargetLoadAction::EClear ? ESimpleRenderTargetMode::EClearColorExistingDepth : ESimpleRenderTargetMode::EUninitializedColorExistingDepth);
			return;
		}
		allocSceneColor(RHICmdList);

		if (mCurrentFeatureLevel >= ERHIFeatureLevel::SM4)
		{
			bool bClearColor = colorLoadAction == ERenderTargetLoadAction::EClear;
			bool bClearDepth = depthLoadAction == ERenderTargetLoadAction::EClear;

			const TextureRHIRef& sceneColorTexture = getSceneColorSurface();
			bool bShaderClear = false;
			if (bClearColor)
			{
				if (!sceneColorTexture->hasClearValue() || (clearColor != sceneColorTexture->getClearColor()))
				{
					colorLoadAction = ERenderTargetLoadAction::ENoAction;
					bShaderClear = true;
				}
				else
				{
					bGBufferFastCleared = true;
				}
			}
			int32 VolocityRTIndex;
			RHIRenderTargetView renderTargets[MaxSimultaneousRenderTargets];
			int32 MRTCount = getGBufferRenderTargets(colorLoadAction, renderTargets, VolocityRTIndex);
			BOOST_ASSERT(renderTargets[0].mTexture == sceneColorTexture);

			RHIDepthRenderTargetView depthView(getSceneDepthSurface(), depthLoadAction, ERenderTargetStoreAction::EStore);
			RHISetRenderTargetsInfo info(MRTCount, renderTargets, depthView);
			if (bClearDepth)
			{
				bSceneDepthCleared = true;
			}
			//if(bBindQuadOverdrawBuffers && )
			RHICmdList.setRenderTargetAndClear(info);
			if (bShaderClear)
			{
				LinearColor clearColors[MaxSimultaneousRenderTargets];
				TextureRHIParamRef textures[MaxSimultaneousRenderTargets];
				clearColors[0] = clearColor;
				textures[0] = renderTargets[0].mTexture;
				for (int32 i = 0; i < MRTCount; ++i)
				{
					clearColors[i] = renderTargets[i].mTexture->getClearColor();
					textures[i] = renderTargets[i].mTexture;
				}
				RHICmdList.clearColorTextures(MRTCount, textures, clearColors, IntRect());
			}

			bool bBindClearColor = !bClearColor && bGBufferFastCleared;
			bool bBindClearDepth = !bClearDepth && bSceneDepthCleared;
			RHICmdList.bindClearMRTValues(bBindClearColor, bBindClearDepth, bBindClearDepth);
		}
	}

	int32 SceneRenderTargets::getGBufferRenderTargets(ERenderTargetLoadAction colorLoadAction, RHIRenderTargetView outRenderTargets[MaxSimultaneousRenderTargets], int32& outVelocityRTIndex)
	{
		int32 MRTCount = 0;
		outRenderTargets[MRTCount++] = RHIRenderTargetView(getSceneColorSurface(), 0, -1, colorLoadAction, ERenderTargetStoreAction::EStore);
		outRenderTargets[MRTCount++] = RHIRenderTargetView(mGBufferA->getRenderTargetItem().mTargetableTexture, 0, -1, colorLoadAction, ERenderTargetStoreAction::EStore);

		outRenderTargets[MRTCount++] = RHIRenderTargetView(mGBufferB->getRenderTargetItem().mTargetableTexture, 0, -1, colorLoadAction, ERenderTargetStoreAction::EStore);

		outRenderTargets[MRTCount++] = RHIRenderTargetView(mGBufferC->getRenderTargetItem().mTargetableTexture, 0, -1, colorLoadAction, ERenderTargetStoreAction::EStore);

		if (bAllocateVelocityGBuffer)
		{
			outVelocityRTIndex = MRTCount;
			BOOST_ASSERT(outVelocityRTIndex == 4);
			outRenderTargets[MRTCount++] = RHIRenderTargetView(mGBufferVelocity->getRenderTargetItem().mTargetableTexture, 0, -1, colorLoadAction, ERenderTargetStoreAction::EStore);
		}
		else
		{
			outVelocityRTIndex = -1;
		}
		outRenderTargets[MRTCount++] = RHIRenderTargetView(mGBufferD->getRenderTargetItem().mTargetableTexture, 0, -1, colorLoadAction, ERenderTargetStoreAction::EStore);
		if (bAllowStaticLighting)
		{
			BOOST_ASSERT(MRTCount == (bAllocateVelocityGBuffer ? 6 : 5));
			outRenderTargets[MRTCount++] = RHIRenderTargetView(mGBufferE->getRenderTargetItem().mTargetableTexture, 0, -1, colorLoadAction, ERenderTargetStoreAction::EStore);
		}
		BOOST_ASSERT(MRTCount <= MaxSimultaneousRenderTargets);
		return MRTCount;
	}

	void SceneRenderTargets::preallocGBufferTargets()
	{
		bAllocateVelocityGBuffer = false;
	}

	void SceneRenderTargets::allocGBufferTargets(RHICommandList& RHICmdList)
	{
		BOOST_ASSERT(mGBufferRefCount == 0);
		if (mGBufferA)
		{
			return;
		}

		const EShaderPlatform shaderPlatform = getFeatureLevelShaderPlatform(mCurrentFeatureLevel);
		const bool bUseGBuffer = isUsingGBuffers(shaderPlatform);
		const bool bCanReadGBufferConstants = (bUseGBuffer || isSimpleForwardShadingEnable(shaderPlatform)) && mCurrentFeatureLevel >= ERHIFeatureLevel::SM4;
		if (bUseGBuffer)
		{
			const bool bHighPrecisionGBuffers = (mCurrentGBufferFormat >= EGBufferFormat::Force16BitsPerChannel);
			const bool bEnforce8BitPerChannael = (mCurrentGBufferFormat == EGBufferFormat::Force8BitsPerChannel);
			{
				PooledRenderTargetDesc desc;
				getGBufferADesc(desc);
				GRenderTargetPool.findFreeElement(RHICmdList, desc, mGBufferA, TEXT("GBufferA"));
			}
			{
				const EPixelFormat specularGBufferFormat = bHighPrecisionGBuffers ? PF_FloatRGBA : PF_B8G8R8A8;
				PooledRenderTargetDesc desc(PooledRenderTargetDesc::create2DDesc(mBufferSize, specularGBufferFormat, ClearValueBinding::Transparent, TexCreate_None, TexCreate_RenderTargetable, false));
				GRenderTargetPool.findFreeElement(RHICmdList, desc, mGBufferB, TEXT("GBufferB"));
			}
			{
				const EPixelFormat DiffuseGBufferFormat = bHighPrecisionGBuffers ? PF_FloatRGBA : PF_B8G8R8A8;
				PooledRenderTargetDesc desc(PooledRenderTargetDesc::create2DDesc(mBufferSize, DiffuseGBufferFormat, ClearValueBinding::Transparent, TexCreate_SRGB, TexCreate_RenderTargetable, false));
				GRenderTargetPool.findFreeElement(RHICmdList, desc, mGBufferC, TEXT("GBufferC"));
			}

			{
				PooledRenderTargetDesc desc(PooledRenderTargetDesc::create2DDesc(mBufferSize, PF_B8G8R8A8, ClearValueBinding::Transparent, TexCreate_None, TexCreate_RenderTargetable, false));
				GRenderTargetPool.findFreeElement(RHICmdList, desc, mGBufferD, TEXT("GBufferD"));
			}

		}

		if (bCanReadGBufferConstants)
		{
			const SceneRenderTargetItem& GBufferAToUse = mGBufferA ? mGBufferA->getRenderTargetItem() : GSystemTextures.mBlackDummy->getRenderTargetItem();
			const SceneRenderTargetItem& GBufferBToUse = mGBufferB? mGBufferB->getRenderTargetItem() : GSystemTextures.mBlackDummy->getRenderTargetItem();
			const SceneRenderTargetItem& GBufferCToUse = mGBufferC ? mGBufferC->getRenderTargetItem() : GSystemTextures.mBlackDummy->getRenderTargetItem();
			const SceneRenderTargetItem& GBufferDToUse = mGBufferD ? mGBufferD->getRenderTargetItem() : GSystemTextures.mBlackDummy->getRenderTargetItem();
			const SceneRenderTargetItem& GBufferEToUse = mGBufferE ? mGBufferE->getRenderTargetItem() : GSystemTextures.mBlackDummy->getRenderTargetItem();

			GBufferResourceStruct _GBufferResourceStruct;
			_GBufferResourceStruct.GBufferATexture = GBufferAToUse.mShaderResourceTexture;

			_GBufferResourceStruct.GBufferBTexture = GBufferBToUse.mShaderResourceTexture;

			_GBufferResourceStruct.GBufferCTexture = GBufferCToUse.mShaderResourceTexture;

			_GBufferResourceStruct.GBufferDTexture = GBufferDToUse.mShaderResourceTexture;

			_GBufferResourceStruct.GBufferETexture = GBufferEToUse.mShaderResourceTexture;

			_GBufferResourceStruct.GBufferATextureNonMS = GBufferAToUse.mShaderResourceTexture;

			_GBufferResourceStruct.GBufferBTextureNonMS = GBufferBToUse.mShaderResourceTexture;

			_GBufferResourceStruct.GBufferCTextureNonMS = GBufferCToUse.mShaderResourceTexture;

			_GBufferResourceStruct.GBufferDTextureNonMS = GBufferDToUse.mShaderResourceTexture;

			_GBufferResourceStruct.GBufferETextureNonMS = GBufferEToUse.mShaderResourceTexture;

			_GBufferResourceStruct.GBufferATextureMS = GBufferAToUse.mTargetableTexture;
			_GBufferResourceStruct.GBufferBTextureMS = GBufferBToUse.mTargetableTexture;
			_GBufferResourceStruct.GBufferCTextureMS = GBufferCToUse.mTargetableTexture;
			_GBufferResourceStruct.GBufferDTextureMS = GBufferDToUse.mTargetableTexture;
			_GBufferResourceStruct.GBufferETextureMS = GBufferEToUse.mTargetableTexture;

			_GBufferResourceStruct.GBufferATextureSampler = TStaticSamplerState<>::getRHI();
			_GBufferResourceStruct.GBufferBTextureSampler = TStaticSamplerState<>::getRHI();
			_GBufferResourceStruct.GBufferCTextureSampler = TStaticSamplerState<>::getRHI();
			_GBufferResourceStruct.GBufferDTextureSampler = TStaticSamplerState<>::getRHI();
			_GBufferResourceStruct.GBufferETextureSampler = TStaticSamplerState<>::getRHI();

			mGBufferResourcesConstantBuffer = GBufferResourceStruct::createConstantBuffer(_GBufferResourceStruct, ConstantBuffer_SingleFrame);
		}
		mGBufferRefCount = 1;
	}

	TRefCountPtr<IPooledRenderTarget> & SceneRenderTargets::getSceneColor()
	{
		if (!getSceneColorForCurrentShadingPath())
		{
			static bool bFirst = true;
			if (bFirst)
			{
				bFirst = false;

				BOOST_ASSERT(getSceneColorForCurrentShadingPath());
			}
			return GSystemTextures.mBlackDummy;
		}
		return getSceneColorForCurrentShadingPath();
	}

	const TRefCountPtr<IPooledRenderTarget>& SceneRenderTargets::getSceneColor() const
	{
		if (!getSceneColorForCurrentShadingPath())
		{
			static bool bFirst = true;
			if (bFirst)
			{
				bFirst = false;

				BOOST_ASSERT(getSceneColorForCurrentShadingPath());
			}
			return GSystemTextures.mBlackDummy;
		}
		return getSceneColorForCurrentShadingPath();
	}

	const TextureRHIRef& SceneRenderTargets::getSceneColorSurface() const
	{
		if (!getSceneColorForCurrentShadingPath())
		{
			return GBlackTexture->mTextureRHI;
		}
		return (const TextureRHIRef&)getSceneColor()->getRenderTargetItem().mTargetableTexture;
	}


	inline const TCHAR* getSceneColorTargetName(EShadingPath shadingPath)
	{
		const TCHAR* sceneColorNames[(uint32)EShadingPath::Num] =
		{
			TEXT("SceneColorMobile"),
			TEXT("SceneColorDeferred")
		};
		BOOST_ASSERT((uint32)shadingPath < ARRAY_COUNT(sceneColorNames));
		return sceneColorNames[(uint32)shadingPath];
	}

	void SceneRenderTargets::allocSceneColor(RHICommandList& RHICmdList)
	{
		TRefCountPtr<IPooledRenderTarget>& sceneColorTarget = getSceneColorForCurrentShadingPath();
		if (sceneColorTarget && sceneColorTarget->getRenderTargetItem().mTargetableTexture->hasClearValue() && !(sceneColorTarget->getRenderTargetItem().mTargetableTexture->getClearBinding() == mDefaultColorClear))
		{
			const LinearColor currentClearColor = sceneColorTarget->getRenderTargetItem().mTargetableTexture->getClearBinding().getClearColor();
			const LinearColor newClearColor = mDefaultColorClear.getClearColor();
			sceneColorTarget.safeRelease();
		}
		if (getSceneColorForCurrentShadingPath())
		{
			return;
		}
		EPixelFormat sceneColorBufferFormat = getSceneColorFormat();
		{
			PooledRenderTargetDesc desc(PooledRenderTargetDesc::create2DDesc(mBufferSize, sceneColorBufferFormat, mDefaultColorClear, TexCreate_None, TexCreate_RenderTargetable, false));
			desc.mFlags |= TexCreate_FastVRAM;
			desc.mNumSamples = getNumSceneColorMSAASamples(mCurrentFeatureLevel);
			if (mCurrentFeatureLevel >= ERHIFeatureLevel::SM5 && desc.mNumSamples == 1)
			{
				desc.mTargetableFlags |= TexCreate_UAV;
			}
			GRenderTargetPool.findFreeElement(RHICmdList, desc, getSceneColorForCurrentShadingPath(), getSceneColorTargetName(mCurrentShadingPath));
		}
		BOOST_ASSERT(getSceneColorForCurrentShadingPath());
	}

	void SceneRenderTargets::beginRenderingSceneColor(RHICommandList& RHICmdList, ESimpleRenderTargetMode renderTargetMode, FExclusiveDepthStencil depthStencilAccess, bool bTransitionWritable)
	{
		allocSceneColor(RHICmdList);
		setRenderTarget(RHICmdList, getSceneColorSurface(), getSceneDepthSurface(), renderTargetMode, depthStencilAccess, bTransitionWritable);
	}
	EPixelFormat SceneRenderTargets::getSceneColorFormat() const
	{
		EPixelFormat sceneColorBufferFormat = PF_FloatRGBA;
		if (mCurrentFeatureLevel < ERHIFeatureLevel::SM4)
		{
			sceneColorBufferFormat = GSupportsRenderTargetFormat_PF_FloatRGBA ? PF_FloatRGBA : PF_B8G8R8A8;
			if (!false)
			{
				sceneColorBufferFormat = PF_B8G8R8A8;
			}
		}
		else
		{
			switch (mCurrentSceneColorFormat)
			{
			case 0:
				sceneColorBufferFormat = PF_R8G8B8A8; break;
			case 1:
				sceneColorBufferFormat = PF_A2B10G10R10; break;
			case 2:
				sceneColorBufferFormat = PF_FloatR11G11B10; break;
			case 3:
				sceneColorBufferFormat = PF_FloatRGB; break;
			case 4:
				break;
			case 5:
				sceneColorBufferFormat = PF_A32B32G32R32F; break;
			}
			if (!GPixelFormats[sceneColorBufferFormat].Supported)
			{
				sceneColorBufferFormat = PF_FloatRGBA;
			}
			if (bRequireSceneColorAlpha)
			{
				sceneColorBufferFormat = PF_FloatRGBA;
			}
		}
		return sceneColorBufferFormat;
	}

	uint16 SceneRenderTargets::getNumSceneColorMSAASamples(ERHIFeatureLevel::Type inFeatureLevel)
	{
		uint16 numSamples = 1;
		if (inFeatureLevel >= ERHIFeatureLevel::SM4)
		{
			EAntiAliasingMethod method = (EAntiAliasingMethod)0;
			if (isForwardShadingEnabled(inFeatureLevel) && method == AAM_MSAA)
			{
				numSamples = 1;
				if (numSamples != 1 && numSamples != 2 && numSamples != 4)
				{
					numSamples = 1;
				}
			}
		}
		else
		{
			numSamples = 1;
		}
		return numSamples;
	}

	const TRefCountPtr<IPooledRenderTarget>& SceneRenderTargets::getLightAttenuation()const
	{
		if (!mLightAttenuation)
		{
			static bool bFirst = true;
			if (bFirst)
			{
				bFirst = false;
				BOOST_ASSERT(mLightAttenuation);
			}
			return GSystemTextures.mWhiteDummy;
		}
		return mLightAttenuation;
	}

	SceneRenderTargets::SceneRenderTargets(const ViewInfo& inView, const SceneRenderTargets& snapshotSource)
	{

	}

	const TextureRHIRef& SceneRenderTargets::getSceneColorTexture() const
	{
		if (!getSceneColorForCurrentShadingPath())
		{
			return GBlackTexture->mTextureRHI;
		}
		return (const TextureRHIRef&)getSceneColor()->getRenderTargetItem().mShaderResourceTexture;
	}

	const Texture2DRHIRef* SceneRenderTargets::getActualDepthTexture() const
	{
		const Texture2DRHIRef* depthTexture = nullptr;
		if ((mCurrentFeatureLevel >= ERHIFeatureLevel::SM4) || isPCPlatform(GShaderPlatformForFeatureLevel[mCurrentFeatureLevel]))
		{
			if (GSupportsDepthFetchDuringDepthTest)
			{
				depthTexture = &getSceneDepthTexture();
			}
			else
			{
				depthTexture = &getAuxiliarySceneDepthSurface();
			}
		}
		else if (isMobilePlatform(GShaderPlatformForFeatureLevel[mCurrentFeatureLevel]))
		{
			{
				depthTexture = &getSceneDepthTexture();
			}
		}
		BOOST_ASSERT(depthTexture != nullptr);
		return depthTexture;
	}

	void SceneRenderTargets::finishRenderingLightAttenuation(RHICommandList& RHICmdList)
	{
		RHICmdList.copyToResolveTarget(getLightAttenuationTexture(), mLightAttenuation->getRenderTargetItem().mShaderResourceTexture, false, ResolveParams(ResolveRect()));

	}

	void SceneRenderTargets::resolveSceneColor(RHICommandList& RHICmdList, const ResolveRect& resolveRect /* = ResolveRect() */)
	{
		auto& currentSceneColor = getSceneColor();
		uint32 currentNumSamples = currentSceneColor->getDesc().mNumSamples;

		const EShaderPlatform currentShaderPlatform = GShaderPlatformForFeatureLevel[mCurrentFeatureLevel];

		if (currentNumSamples <= 1 || !RHISupportsSeparateMSAAAndResolveTextures(currentShaderPlatform))
		{
			RHICmdList.copyToResolveTarget(getSceneColorSurface(), getSceneColorTexture(), true, ResolveParams(resolveRect));
		}
		else
		{
			setRenderTarget(RHICmdList, getSceneColorTexture(), TextureRHIParamRef());
			if (resolveRect.isValid())
			{
				RHICmdList.setScissorRect(true, resolveRect.X1, resolveRect.Y1, resolveRect.X2, resolveRect.Y2);
			}
			RHICmdList.setBlendState(TStaticBlendState<>::getRHI());
			RHICmdList.setRasterizerState(TStaticRasterizerState<>::getRHI());
			RHICmdList.setDepthStencilState(TStaticDepthStencilState<false, CF_Always>::getRHI());
			RHICmdList.setStreamSource(0, NULL, 0, 0);
			int32 resolveWidth = 0;
			if (currentNumSamples <= 1)
			{
				resolveWidth = 0;
			}
			if (resolveWidth != 0)
			{
			}
			else
			{
				auto shaderMap = getGlobalShaderMap(mCurrentFeatureLevel);
				TShaderMapRef<HdrCustomResolveVS> vertexShader(shaderMap);
				if (currentNumSamples == 2)
				{
					TShaderMapRef<HdrCustomResolve2xPS> pixelShader(shaderMap);
					static GlobalBoundShaderState boundShaderState;
					setGlobalBoundShaderState(RHICmdList, mCurrentFeatureLevel, boundShaderState, getVertexDeclarationVector4(), *vertexShader, *pixelShader);
					pixelShader->setParameters(RHICmdList, currentSceneColor->getRenderTargetItem().mTargetableTexture);
					RHICmdList.drawPrimitive(PT_TriangleList, 0, 1, 1);
				}
				else if(currentNumSamples == 4)
				{
					TShaderMapRef<HdrCustomResolve4xPS> pixelShader(shaderMap);
					static GlobalBoundShaderState boundShaderState;
					setGlobalBoundShaderState(RHICmdList, mCurrentFeatureLevel, boundShaderState, getVertexDeclarationVector4(), *vertexShader, *pixelShader);
					pixelShader->setParameters(RHICmdList, currentSceneColor->getRenderTargetItem().mTargetableTexture);
					RHICmdList.drawPrimitive(PT_TriangleList, 0, 1, 1);

				}
				else if (currentNumSamples == 8)
				{
					TShaderMapRef<HdrCustomResolve8xPS> pixelShader(shaderMap);
					static GlobalBoundShaderState boundShaderState;
					setGlobalBoundShaderState(RHICmdList, mCurrentFeatureLevel, boundShaderState, getVertexDeclarationVector4(), *vertexShader, *pixelShader);
					pixelShader->setParameters(RHICmdList, currentSceneColor->getRenderTargetItem().mTargetableTexture);
					RHICmdList.drawPrimitive(PT_TriangleList, 0, 1, 1);
				}
				else
				{
					BOOST_ASSERT(false);
				}
			}
		}
	}

	void SceneRenderTargets::setSceneColor(IPooledRenderTarget* i)
	{
		BOOST_ASSERT(mCurrentShadingPath < EShadingPath::Num);
		mSceneColor[(int32)getSceneColorFormatType()] = i;
	}

	bool SceneRenderTargets::isSceneColorAllocated() const
	{
		return getSceneColorForCurrentShadingPath() != nullptr;
	}

	void SceneRenderTargets::releaseGBufferTargets()
	{
		mGBufferResourcesConstantBuffer.safeRelease();
		mGBufferA.safeRelease();
		mGBufferB.safeRelease();
		mGBufferC.safeRelease();
		mGBufferD.safeRelease();
		mGBufferE.safeRelease();
		mGBufferVelocity.safeRelease();
	}

	void SceneRenderTargets::adjustGBufferRefCount(RHICommandList& RHICmdList, int delta)
	{
		if (delta > 0 && mGBufferRefCount == 0)
		{
			allocGBufferTargets(RHICmdList);
		}
		else
		{
			mGBufferRefCount += delta;
			if (mGBufferRefCount == 0)
			{
				releaseGBufferTargets();
			}
		}
	}

	void SceneRenderTargets::allocateReflectionTargets(RHICommandList& RHICmdList, int32 targetSize)
	{
		if (GSupportsRenderTargetFormat_PF_FloatRGBA)
		{
			const int32 numReflectionCaptureMips = Math::ceilLogTwo(targetSize) + 1;

			if (mReflectionColorScratchCubemap[0] && mReflectionColorScratchCubemap[0]->getRenderTargetItem().mTargetableTexture->getNumMips() != numReflectionCaptureMips)
			{
				mReflectionColorScratchCubemap[0].safeRelease();
				mReflectionColorScratchCubemap[1].safeRelease();
			}

			bool bSharedReflectionTargetsAllocated = mReflectionColorScratchCubemap[0] != nullptr;

			if (!bSharedReflectionTargetsAllocated)
			{
				uint32 cubeTexFlags = TexCreate_TargetArraySlicesIndependently;
				{
					PooledRenderTargetDesc desc2(PooledRenderTargetDesc::createCubemapDesc(targetSize, PF_FloatRGBA, ClearValueBinding::None, cubeTexFlags, TexCreate_RenderTargetable, false, 1, numReflectionCaptureMips));
					GRenderTargetPool.findFreeElement(RHICmdList, desc2, mReflectionColorScratchCubemap[0], TEXT("ReflectionColorScratchCubemap0"));
					GRenderTargetPool.findFreeElement(RHICmdList, desc2, mReflectionColorScratchCubemap[1], TEXT("ReflectionColorScratchCubemap1"));
				}

				const int32 numDiffuseIrradianceMips = Math::ceilLogTwo(GDiffuseIrradianceCubemapSize) + 1;

				{
					PooledRenderTargetDesc desc2(PooledRenderTargetDesc::createCubemapDesc(GDiffuseIrradianceCubemapSize, PF_FloatRGBA, ClearValueBinding::None, cubeTexFlags, TexCreate_RenderTargetable, false, 1, numDiffuseIrradianceMips));
					GRenderTargetPool.findFreeElement(RHICmdList, desc2, mDiffuseIrradianceScratchCubemap[0], TEXT("DiffuseIrradianceScratchCubemap0"));
					GRenderTargetPool.findFreeElement(RHICmdList, desc2, mDiffuseIrradianceScratchCubemap[1], TEXT("DiffuseIrradianceScratchCubemap1"));
				}
				{
					PooledRenderTargetDesc desc(PooledRenderTargetDesc::create2DDesc(int2(SHVector3::MaxSHBasis, 1), PF_FloatRGBA, ClearValueBinding::None, TexCreate_None, TexCreate_RenderTargetable, false));
					GRenderTargetPool.findFreeElement(RHICmdList, desc, mSkySHIrradianceMap, TEXT("SkySHIrradianceMap"));
				}

			}
		}
	}
}