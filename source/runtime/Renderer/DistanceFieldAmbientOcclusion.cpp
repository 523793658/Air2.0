#include "DeferredShadingRenderer.h"
namespace Air
{
	bool DeferredShadingSceneRenderer::shouldRenderDistanceFieldAO() const
	{
		return mViewFamily.mEngineShowFlags.DistanceFieldAO && !mViewFamily.mEngineShowFlags.VisualizeDistanceFieldAO &&
			!mViewFamily.mEngineShowFlags.VisualizeDistanceFieldGI
			&& !mViewFamily.mEngineShowFlags.VisualizeMeshDistanceFields;
	}
}