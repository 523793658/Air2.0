#include "ScenePrivate.h"
#include "LightMapRendering.h"
#include "LightSceneInfo.h"
#include "Classes/Components/LightComponent.h"
namespace Air
{
	SIZE_T StaticMeshDrawListBase::mTotalBytesUsed = 0;


	template<>
	TStaticMeshDrawList<TBasePassDrawingPolicy<ConstantLightMapPolicy>>& Scene::getBasePassDrawList<ConstantLightMapPolicy>(EBasePassDrawListType drawType)
	{
		return mBasePassConstantLightMapPolicyDrawList[drawType];
	}

	void Scene::addLight(LightComponent* light)
	{
		LightSceneProxy* proxy = light->createSceneProxy();
		if (proxy)
		{
			light->mSceneProxy = proxy;

			proxy->setTransform(light->mComponentToWorld.toMatrixNoScale(), light->getLightPosition());
			proxy->mLightSceneInfo = new LightSceneInfo(proxy, true);
			++mNumVisibleLights_GameThread;
			ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
				AddLightCommand,
				Scene*, scene, this,
				LightSceneInfo*, lightSceneInfo, proxy->mLightSceneInfo,
				{
					scene->addLightSceneInfo_RenderThread(lightSceneInfo);
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
			ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(
				UpdateLightColorAndBrightness, 
				LightSceneInfo*, lightSceneInfo, light->mSceneProxy->getLightSceneInfo(),
				Scene*, scene, this,
				UpdateLightColorParameters, parameters, newParameters,
				{
					if (lightSceneInfo && lightSceneInfo->bVisible)
					{
						lightSceneInfo->mProxy->setColor(parameters.newColor);
						lightSceneInfo->mProxy->mIndirectLightingScale = parameters.newIndirectLightingScale;

						if (lightSceneInfo->mId != INDEX_NONE)
						{
							scene->mLights[lightSceneInfo->mId].mColor = parameters.newColor;
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
		const bool bForwardShading = isForwardShadingEnabled(mFeatureLevel);
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