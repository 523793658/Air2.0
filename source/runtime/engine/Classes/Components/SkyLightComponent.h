#pragma once
#include "Classes/Components/LightComponent.h"
#include "RenderResource.h"
#include "RenderingThread.h"
#include "Math/SHMath.h"
#include "TextureCube.h"
#include "RenderCommandFence.h"
namespace Air
{


	class SkyTextureCubeResource : public Texture, private DeferredCleanupInterface
	{
	public:
		SkyTextureCubeResource()
			:mSize(0)
			,mNumMips(0)
			,mFormat(PF_Unknown)
			,mTextureCubeRHI(nullptr)
			,mNumRefs(0)
		{

		}

		virtual ~SkyTextureCubeResource() { BOOST_ASSERT(mNumRefs == 0); }

		void setupParameters(int32 inSize, int32 inNumMips, EPixelFormat inFormat)
		{
			mSize = inSize;
			mNumMips = inNumMips;
			mFormat = inFormat;
		}

		virtual void initRHI() override;

		virtual void releaseRHI() override
		{
			mTextureCubeRHI.safeRelease();
			Texture::releaseRHI();
		}

		virtual uint32 getWidth() const override
		{
			return mSize;
		}

		virtual uint32 getHeight() const override
		{
			return mSize;
		}

		void AddRef()
		{
			BOOST_ASSERT(isInGameThread());
			mNumRefs++;
		}

		void Release();

		virtual void finishCleanup() override
		{
			delete this;
		}

	private:
		int32 mSize;
		int32 mNumMips;
		EPixelFormat mFormat;
		TextureCubeRHIRef mTextureCubeRHI;
		int32 mNumRefs;
	};

	enum ESkyLightSourceType
	{
		SLS_CapturedScene,
		SLS_SpecifiedCubmap,
		SLS_MAX,
	};

	class ENGINE_API SkyLightComponent : public LightComponentBase
	{
		GENERATED_RCLASS_BODY(SkyLightComponent, LightComponentBase)
	public:
		class SkyLightSceneProxy* createSceneProxy() const;

		virtual void postInitProperties() override;

		void setCubemap(std::shared_ptr<RTextureCube> cube);

		static void updateSkyCaptureContents(World* worldToUpdate);

		static void updateSkyCaptureContentsArray(World* worldToUpdate, TArray<SkyLightComponent*>& componentArray, bool bBlendSources);

	protected:
		virtual void createRenderState_Concurrent() override;

		virtual void destroyRenderState_Concurrent() override;
	public:
		TEnumAsByte<enum ESkyLightSourceType> mSourceType;

		std::shared_ptr<class RTextureCube> mCubemap;

		float mSourceCubemapAngle;

		int32 mCubemapResolution;

		float mSkyDistanceThreshold;

		bool bLowerHemisphereIsBlack{ false };

		LinearColor lowerHemisphereColor;

	protected:
		TRefCountPtr<SkyTextureCubeResource> mProcessedSkyTexture;

		TRefCountPtr<SkyTextureCubeResource> mBlendDestinationProcessedSkyTexture;

		std::shared_ptr<RTextureCube> mBlendDestinationCubemap;

		RenderCommandFence mIrradianceMapFence;

		SHVectorRGB3 mIrradianceEnvironmentMap;

		SHVectorRGB3 mBlendDestinationIrradianceEnvironmentMap;

		static TArray<SkyLightComponent*> mSkyCapturesToUpdates;
		static CriticalSection skyCaptureToUpdateLock;

		friend class SkyLightSceneProxy;

		class SkyLightSceneProxy* mSceneProxy;

		float mAverageBrightness;

		float mBlendFraction;

		bool bHasEverCapture{ false };

		float mBlendDestinationAverageBrightness;

	private:
		float mOcclusionMaxDistance;

		float mContrast;

		float mMinOcclusion;

		Color mOcclusionTint;
	};

}