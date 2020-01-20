#include "DeferredShadingRenderer.h"
#include "ScenePrivate.h"
namespace Air
{
	int32 GUseClusteredDeferredShading = 0;
	bool DeferredShadingSceneRenderer::shouldUseClusteredDeferredShading() const
	{
		return GUseClusteredDeferredShading != 0 && mScene->getFeatureLevel() >= ERHIFeatureLevel::SM5;
	}
}