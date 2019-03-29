#include "DeferredShadingRenderer.h"
#include "ScenePrivate.h"
namespace Air
{
	bool DeferredShadingSceneRenderer::checkForLightFunction(const LightSceneInfo* lightSceneInfo) const
	{
		if (lightSceneInfo->mProxy->getLightFunctionMaterial() && lightSceneInfo->mProxy->getLightFunctionMaterial()->getMaterial(mScene->getFeatureLevel())->isLightFunction())
		{

		}
		return false;
	}
}