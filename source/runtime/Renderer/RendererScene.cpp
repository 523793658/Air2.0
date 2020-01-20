#include "ScenePrivate.h"
#include "LightMapRendering.h"
#include "LightSceneInfo.h"
#include "Classes/Components/LightComponent.h"
namespace Air
{


	void Scene::addLight(LightComponent* light)
	{
		LightSceneProxy* proxy = light->createSceneProxy();
		if (proxy)
		{
			light->mSceneProxy = proxy;

			proxy->setTransform(light->mComponentToWorld.toMatrixNoScale(), light->getLightPosition());
			proxy->mLightSceneInfo = new LightSceneInfo(proxy, true);
			++mNumVisibleLights_GameThread;
			LightSceneInfo* lightSceneInfo = proxy->mLightSceneInfo;

			ENQUEUE_RENDER_COMMAND(
				AddLightCommand)([this, lightSceneInfo](RHICommandListImmediate& RHICmdList)
				{
					this->addLightSceneInfo_RenderThread(lightSceneInfo);
				}
			);
		}
	}

	void Scene::updateLightColorAndBrightness(LightComponent* light)
	{
		if (light->mSceneProxy)
		{
			struct UpdateLightColorParameters
			{
				LinearColor newColor;
				float newIndirectLightingScale;
			};

			UpdateLightColorParameters newParameters;
			newParameters.newColor = LinearColor(light->mLightColor) * light->computeLightBrightness();
			newParameters.newIndirectLightingScale = light->mIndirectLightingIntensity;
			LightSceneInfo* lightSceneInfo = light->mSceneProxy->getLightSceneInfo();

			ENQUEUE_RENDER_COMMAND(
				UpdateLightColorAndBrightness)([lightSceneInfo, this, newParameters](RHICommandListImmediate& RHICmdList)
				{
					if (lightSceneInfo && lightSceneInfo->bVisible)
					{
						lightSceneInfo->mProxy->setColor(newParameters.newColor);
						lightSceneInfo->mProxy->mIndirectLightingScale = newParameters.newIndirectLightingScale;

						if (lightSceneInfo->mId != INDEX_NONE)
						{
							this->mLights[lightSceneInfo->mId].mColor = newParameters.newColor;
						}
					}
				});
		}
	}

	void Scene::addLightSceneInfo_RenderThread(LightSceneInfo* lightSceneInfo)
	{
		BOOST_ASSERT(lightSceneInfo->bVisible);
		lightSceneInfo->mId = mLights.add(LightSceneInfoCompact(lightSceneInfo));
		const LightSceneInfoCompact& lightSceneInfoCompact = mLights[lightSceneInfo->mId];
		if (lightSceneInfo->mProxy->getLightType() == LightType_Directional && !lightSceneInfo->mProxy->hasStaticLighting())
		{
			if (!mSimpleDirectionalLight)
			{
				mSimpleDirectionalLight = lightSceneInfo;
			}
			if (getShadingPath() == EShadingPath::Mobile)
			{

			}
		}
		const bool bForwardShading = isForwardShadingEnabled(getFeatureLevelShaderPlatform(mFeatureLevel));
		if (bForwardShading && lightSceneInfo->mProxy->castsDynamicShadow())
		{

		}

		if (lightSceneInfo->mProxy->isUsedAsAtmosphereSunLight() && (!mSunLight || lightSceneInfo->mProxy->getColor().computeLuminance() > mSunLight->mProxy->getColor().computeLuminance()))
		{
			mSunLight = lightSceneInfo;
		}

		lightSceneInfo->addToScene();
	}
}