#include "SkyLightComponent.h"
#include "SceneManagement.h"
#include "Classes/Engine/World.h"
#include "SimpleReflection.h"
#include "Misc/ScopeLock.h"
#include "ShaderCompiler.h"

namespace Air
{

	TArray<SkyLightComponent*> SkyLightComponent::mSkyCapturesToUpdates;
	WindowsCriticalSection SkyLightComponent::skyCaptureToUpdateLock;
	
	void SkyTextureCubeResource::initRHI()
	{
		if (getFeatureLevel() >= ERHIFeatureLevel::SM4)
		{
			RHIResourceCreateInfo createInfo;
			mTextureCubeRHI = RHICreateTextureCube(mSize, mFormat, mNumMips, 0, createInfo);
			mTextureRHI = mTextureCubeRHI;
			SamplerStateInitializerRHI samplerStateInitializer
			(
				SF_Trilinear,
				AM_Clamp,
				AM_Clamp,
				AM_Clamp
			);
			mSamplerStateRHI = RHICreateSamplerState(samplerStateInitializer);
		}
	}

	void SkyTextureCubeResource::Release()
	{
		BOOST_ASSERT(isInGameThread());
		BOOST_ASSERT(mNumRefs > 0);
		if (--mNumRefs == 0)
		{
			beginReleaseResource(this);
			beginCleanup(this);
		}
	}

	SkyLightSceneProxy::SkyLightSceneProxy(const class SkyLightComponent* inLightComponent)
		: mLightComponent(std::dynamic_pointer_cast<const SkyLightComponent>(inLightComponent->shared_from_this()))
		, mProcessedTexture(inLightComponent->mProcessedSkyTexture)
		, mBlendDestinationProcessedTexture(inLightComponent->mBlendDestinationProcessedSkyTexture)
		,mSkyDistanceThreshold(inLightComponent->mSkyDistanceThreshold)
		,bCastShadows(inLightComponent->bCastShadows)
		,bWantsStaticShadowing(inLightComponent->mMobility == EComponentMobility::Stationary)
		,bHasStaticLighting(inLightComponent->hasStaticLighting())
		,mLightColor(LinearColor(inLightComponent->mLightColor) * inLightComponent->mIntensity)
		,mIndirectLightingIntensity(inLightComponent->mIndirectLightingIntensity)
		,mOcclusionMaxDistance(inLightComponent->mOcclusionMaxDistance)
		,mContrast(inLightComponent->mContrast)
		,mMinOcclusion(inLightComponent->mMinOcclusion)
		,mOcclusionTint(inLightComponent->mOcclusionTint)
	{
		const SHVectorRGB3* inIrradianceEnvironmentMap = &inLightComponent->mIrradianceEnvironmentMap;
		const SHVectorRGB3* blendDestinationIrradianceEnvironmentMap = &inLightComponent->mBlendDestinationIrradianceEnvironmentMap;
		const float* inAverageBrightness = &inLightComponent->mAverageBrightness;
		const float* blendDestinationAverageBrightness = &inLightComponent->mBlendDestinationAverageBrightness;
		float inBlendFraction = inLightComponent->mBlendFraction;
		ENQUEUE_RENDER_COMMAND(
			InitSkyProxy)([inIrradianceEnvironmentMap, blendDestinationIrradianceEnvironmentMap,
				inAverageBrightness,
				blendDestinationAverageBrightness,
				inBlendFraction,
				this](RHICommandListImmediate& RHICmdList)
			{
				this->initialize(inBlendFraction, inIrradianceEnvironmentMap, blendDestinationIrradianceEnvironmentMap, inAverageBrightness, blendDestinationAverageBrightness);
			}
		);
	}

	SkyLightComponent::SkyLightComponent(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		mIntensity = 1;
		mIndirectLightingIntensity = 1.0f;
		mSkyDistanceThreshold = 150000;
		mMobility = EComponentMobility::Stationary;
		mOcclusionMaxDistance = 1000;
		mMinOcclusion = 0;
		mOcclusionTint = Color::Black;
		mCubemapResolution = 128;
		mAverageBrightness = 1.0f;
		mBlendDestinationAverageBrightness = 1.0f;
	}

	void SkyLightComponent::setLightColor(LinearColor color)
	{
		Color newColor(color.toColor(true));
		if (areDynamicDataChangeAllowed() && mLightColor != newColor)
		{
			mLightColor = newColor;
			updateLimitedRenderingStateFast();
		}
	}

	void SkyLightComponent::updateLimitedRenderingStateFast()
	{
		if (mSceneProxy)
		{
			SkyLightSceneProxy* lightSceneProxy = mSceneProxy;
			LinearColor lightColor = LinearColor(mLightColor) * mIntensity;
			float indirectLightIntensity = mIndirectLightingIntensity;
			ENQUEUE_RENDER_COMMAND(FastUpdateSkyLightCommand)([lightSceneProxy, lightColor, indirectLightIntensity](RHICommandListImmediate& RHICmdList)
				{
					lightSceneProxy->mLightColor = lightColor;
					lightSceneProxy->mIndirectLightingIntensity = indirectLightIntensity;
				});
		}
	}

	void SkyLightComponent::postInitProperties()
	{
		if (!hasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
		{
			ScopeLock lock(&skyCaptureToUpdateLock);
			mSkyCapturesToUpdates.addUnique(this);
		}
		ParentType::postInitProperties();
	}

	void SkyLightComponent::sanitizeCubemapSize()
	{
		static const int32 s_MaxCubemapResolution = 1024;
		static const int32 s_MinCubemapResolution = 64;
		mCubemapResolution = Math::clamp(int32(Math::roundUpToPowerOfTwo(mCubemapResolution)), s_MinCubemapResolution, s_MaxCubemapResolution);
	}

	SkyLightSceneProxy* SkyLightComponent::createSceneProxy() const
	{
		if (mProcessedSkyTexture)
		{
			return new SkyLightSceneProxy(this);
		}
		return nullptr;
	}

	void SkyLightComponent::createRenderState_Concurrent()
	{
		ParentType::createRenderState_Concurrent();
		bool bHidden = false;
		if (!shouldComponentAddToScene())
		{
			bHidden = true;
		}

		const bool bIsValid = mSourceType != SLS_SpecifiedCubmap || mCubemap != nullptr;

		if (bAffectsWorld && bVisible && !bHidden && bIsValid)
		{
			mSceneProxy = createSceneProxy();

			if (mSceneProxy)
			{
				getWorld()->mScene->setSkyLight(mSceneProxy);
			}
		}
	}

	void SkyLightComponent::destroyRenderState_Concurrent()
	{
		ParentType::destroyRenderState_Concurrent();
		if (mSceneProxy)
		{
			getWorld()->mScene->disableSkyLight(mSceneProxy);
			SkyLightSceneProxy* lightSceneProxy = mSceneProxy;

			ENQUEUE_RENDER_COMMAND(DestroySkyLightCommand)([lightSceneProxy](RHICommandListImmediate& RHICmdList)
				{
					delete lightSceneProxy;
				}
			);
			mSceneProxy = nullptr;
		}
	}

	void SkyLightSceneProxy::initialize(float inBlendFraction, const SHVectorRGB3* inIrradianceEnvironmentMap, const SHVectorRGB3* blenddestinationIrradianceEnvironmentMap, const float* inAverageBrightness, const float* blendDestinationAverageBrightness)
	{
		mBlendFraction = Math::clamp(inBlendFraction, 0.0f, 1.0f);
		if (mBlendFraction > 0 && mBlendDestinationProcessedTexture != nullptr)
		{
			if (mBlendFraction < 1)
			{
				mIrradianceEnvironmentMap = (*inIrradianceEnvironmentMap) * (1 - mBlendFraction) + (*blenddestinationIrradianceEnvironmentMap) * mBlendFraction;
			}
			else
			{
				mIrradianceEnvironmentMap = *blenddestinationIrradianceEnvironmentMap;
				mAverageBrightness = *blendDestinationAverageBrightness;
			}
		}
		else
		{
			mIrradianceEnvironmentMap = *inIrradianceEnvironmentMap;
			mAverageBrightness = *inAverageBrightness;
			mBlendFraction = 0;
		}
	}

	SkyLightComponent::~SkyLightComponent()
	{
	}

	void SkyLightComponent::setCubemap(std::shared_ptr<RTextureCube> cube)
	{
		if (areDynamicDataChangeAllowed()
			&& mCubemap != cube)
		{
			mCubemap = cube;
			markRenderStateDirty();
			//setcaptureIs
		}
	}

	void SkyLightComponent::updateSkyCaptureContents(World* worldToUpdate)
	{
		if (worldToUpdate->mScene)
		{
			if (mSkyCapturesToUpdates.size() > 0)
			{
				ScopeLock lock(&skyCaptureToUpdateLock);
				updateSkyCaptureContentsArray(worldToUpdate, mSkyCapturesToUpdates, true);
			}
		}
	}

	void SkyLightComponent::updateSkyCaptureContentsArray(World* worldToUpdate, TArray<SkyLightComponent*>& componentArray, bool bBlendSources)
	{
		const bool bIsCompilingShaders = GShaderCompilingManager != nullptr && GShaderCompilingManager->isCompiling();
		for (int32 captureIndex = componentArray.size() - 1; captureIndex >= 0; captureIndex--)
		{
			SkyLightComponent* captureComponent = componentArray[captureIndex];
			AActor* owner = captureComponent->getOwner();
			if ((!owner || !owner->getLevel() || (worldToUpdate->containsActor(owner) && owner->getLevel()->bIsVisible)) && (!bIsCompilingShaders || captureComponent->mSourceType == SLS_SpecifiedCubmap))
			{
				if (captureComponent->mSourceType != SLS_SpecifiedCubmap || captureComponent->mCubemap)
				{
					if (bBlendSources)
					{
						BOOST_ASSERT(!captureComponent->mProcessedSkyTexture || captureComponent->mProcessedSkyTexture->getWidth() == captureComponent->mProcessedSkyTexture->getHeight());
						if (!captureComponent->mProcessedSkyTexture || captureComponent->mProcessedSkyTexture->getWidth() != captureComponent->mCubemapResolution)
						{
							captureComponent->mProcessedSkyTexture = new SkyTextureCubeResource();
							captureComponent->mProcessedSkyTexture->setupParameters(captureComponent->mCubemapResolution, Math::ceilLogTwo(captureComponent->mCubemapResolution) + 1, PF_FloatRGBA);
							beginInitResource(captureComponent->mProcessedSkyTexture);
							captureComponent->markRenderStateDirty();

						}
						worldToUpdate->mScene->updateSkyCaptureContents(captureComponent, false, captureComponent->mCubemap, captureComponent->mProcessedSkyTexture, captureComponent->mAverageBrightness, captureComponent->mIrradianceEnvironmentMap);
					}
					else
					{
						BOOST_ASSERT(!captureComponent->mBlendDestinationProcessedSkyTexture || captureComponent->mBlendDestinationProcessedSkyTexture->getWidth() == captureComponent->mBlendDestinationProcessedSkyTexture->getHeight());
						if (!captureComponent->mBlendDestinationProcessedSkyTexture || captureComponent->mBlendDestinationProcessedSkyTexture->getWidth() != captureComponent->mCubemapResolution)
						{
							captureComponent->mBlendDestinationProcessedSkyTexture = new SkyTextureCubeResource();
							captureComponent->mBlendDestinationProcessedSkyTexture->setupParameters(captureComponent->mCubemapResolution, Math::ceilLogTwo(captureComponent->mCubemapResolution) + 1, PF_FloatRGBA);
							beginInitResource(captureComponent->mBlendDestinationProcessedSkyTexture);
							captureComponent->markRenderStateDirty();
						}
						worldToUpdate->mScene->updateSkyCaptureContents(captureComponent, false, captureComponent->mBlendDestinationCubemap, captureComponent->mBlendDestinationProcessedSkyTexture, captureComponent->mBlendDestinationAverageBrightness, captureComponent->mBlendDestinationIrradianceEnvironmentMap);
					}
					captureComponent->mIrradianceMapFence.beginFence();
					captureComponent->bHasEverCapture = true;
					captureComponent->markRenderStateDirty();
				}
				componentArray.removeAt(captureIndex);
			}
		}
	}

	DECLARE_SIMPLER_REFLECTION(SkyLightComponent);
}