#include "ShadowRendering.h"

namespace Air
{



	TGlobalResource<StencilingGeometry::TStencilSphereVertexBuffer<18, 12, float4>> StencilingGeometry::GStencilSphereVertexBuffer;
	TGlobalResource<StencilingGeometry::TStencilSphereVertexBuffer<18, 12, float3>> StencilingGeometry::GStencilSphereVectorBuffer;

	TGlobalResource<StencilingGeometry::TStencilSphereIndexBuffer<18, 12>> StencilingGeometry::GStencilSphereIndexBuffer;
	TGlobalResource<StencilingGeometry::TStencilSphereVertexBuffer<4, 4, float4>> StencilingGeometry::GLowPolyStencilSphereVertexBuffer;
	TGlobalResource<StencilingGeometry::TStencilSphereIndexBuffer<4, 4>> StencilingGeometry::GLowPolyStencilSphereIndexBuffer;

	TGlobalResource<StencilingGeometry::StencilConeVertexBuffer> StencilingGeometry::GStencilConeVertexBuffer;
	TGlobalResource<StencilingGeometry::StencilConeIndexBuffer> StencilingGeometry::GStencilConeIndexBuffer;

	float getLightFadeFactor(const SceneView& view, const LightSceneProxy* proxy)
	{
		Sphere bounds = proxy->getBoundingSphere();
		const float distanceSquared = (bounds.mCenter - view.mViewMatrices.getViewOrigin()).sizeSquared();
		extern float GMinScreenRadiusForLights;
		float sizeFade = Math::square(Math::min(0.0002f, GMinScreenRadiusForLights / bounds.W) * 1.0f) * distanceSquared;
		sizeFade = Math::clamp(6.0f - 6.0f * sizeFade, 0.0f, 1.0f);
		float maxDist = proxy->getMaxDrawDistance();
		float range = proxy->getFadeRange();
		float distanceFade = maxDist ? (maxDist - Math::sqrt(distanceSquared)) / range : 1.0f;
		distanceFade = Math::clamp(distanceFade, 0.0f, 1.0f);
		return sizeFade * distanceFade;
	}

	bool SceneRenderer::checkForProjectedShadows(const LightSceneInfo* lightSceneInfo) const
	{
		return false;
	}

	void SceneRenderer::renderShadowDepthMapAtlases(RHICommandListImmediate& RHICmdList)
	{

	}

	void SceneRenderer::renderShadowDepthMaps(RHICommandListImmediate& RHICmdList)
	{
		BOOST_ASSERT(RHICmdList.isOutsideRenderPass());

		SceneRenderTargets& sceneContext = SceneRenderTargets::get(RHICmdList);

		SceneRenderer::renderShadowDepthMapAtlases(RHICmdList);

		BOOST_ASSERT(RHICmdList.isOutsideRenderPass());

		//for(int32 cubemapIndex = 0;)
	}

}