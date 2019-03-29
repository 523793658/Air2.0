#include "LightSceneInfo.h"
#include "ScenePrivate.h"
#include "SceneManagement.h"
namespace Air
{
	LightSceneInfo::LightSceneInfo(LightSceneProxy* inProxy, bool inbVisible)
		:mProxy(inProxy)
		,mId(INDEX_NONE)
		,bVisible(inbVisible)
		,mScene(inProxy->getLightComponent()->getScene()->getRenderScene())
	{
		BOOST_ASSERT(bVisible);

		beginInitResource(this);

		for (uint32 lightTypeIndex = 0; lightTypeIndex < LightType_Max; ++lightTypeIndex)
		{
			for (uint32 a = 0; a < 2; ++a)
			{
				for (uint32 b = 0; b < 2; b++)
				{
					for (uint32 c = 0; c < 2; c++)
					{
						mTranslucentInjectCachedShaderMaps[lightTypeIndex][a][b][c] = nullptr;
					}
				}
			}
		}
	}

	LightSceneInfo::~LightSceneInfo()
	{
		releaseResource();
	}

	bool LightSceneInfo::shouldRenderLight(const ViewInfo& view) const
	{
		bool bLocalVisible = bVisible ? view.mVisibleLightInfos[mId].bInViewFrustum : true;
		return bLocalVisible && (!view.bStaticSceneOnly || mProxy->hasStaticShadowing()) && (mProxy->getLightingChannelMask() & getDefaultLightingChannelMask() || view.bUseLightingChannels);
	}

	void LightSceneInfo::addToScene()
	{
		const LightSceneInfoCompact& lightSceneInfoCompact = mScene->mLights[mId];
		if (mProxy->castsDynamicShadow() || mProxy->castsStaticShadow() || mProxy->hasStaticLighting())
		{
			mScene->mLightOctree.addElement(lightSceneInfoCompact);
			MemMark memStackMark(MemStack::get());
			for (ScenePrimitiveOctree::TConstElementBoxIterator<SceneRenderingAllocator> primitiveIt(mScene->mPrimitiveOctree, getBoundingBox());
				primitiveIt.hasPendingElements(); primitiveIt.advance())
			{
				createLightPrimitiveInteraction(lightSceneInfoCompact, primitiveIt.getCurrentElement());
			}
		}
	}

	void LightSceneInfo::createLightPrimitiveInteraction(const LightSceneInfoCompact& lightSceneInfoCompact, const PrimitiveSceneInfoCompact& primitiveSceneInfoCompact)
	{
	}

	void LightSceneInfoCompact::init(LightSceneInfo* inLightSceneInfo)
	{
		mLightSceneInfo = inLightSceneInfo;
		Sphere boundingSphere(inLightSceneInfo->mProxy->getOrigin(), inLightSceneInfo->mProxy->getRadius() > 0.0f ? inLightSceneInfo->mProxy->getRadius() : FLT_MAX);
		Memory::memcpy(&mBoundingSphereVector, &boundingSphere, sizeof(mBoundingSphereVector));
		mColor = inLightSceneInfo->mProxy->getColor();
		mLightType = inLightSceneInfo->mProxy->getLightType();
		bCastDynamicShadow = inLightSceneInfo->mProxy->castsDynamicShadow();
		bCastStaticShadow = inLightSceneInfo->mProxy->castsStaticShadow();
		bStaticLighting = inLightSceneInfo->mProxy->hasStaticLighting();
	}
}