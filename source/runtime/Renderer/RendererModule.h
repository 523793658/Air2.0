#pragma once
#include "RendererMininal.h"
#include "RendererInterface.h"
#include "EngineModule.h"
#include <set>
namespace Air
{

	class RendererModule : public IRendererModule
	{
	public:
		RendererModule();

		virtual void beginRenderingViewFamily(Canvas* canvas, SceneViewFamily* viewFamily) override;

		virtual SceneViewStateInterface* allocateViewState() override;

		virtual SceneInterface* allocateScene(World* world, bool bInRequiresHitProxies, bool bCreateFXSystem, ERHIFeatureLevel::Type inFeatureLevel) override;

		virtual void tickRenderTargetPool() override;

		virtual const TSet<SceneInterface*>& getAllocatedScenes() override
		{
			return mAllocatedScenes;
		}
	private:
		TSet<SceneInterface*> mAllocatedScenes;
	};

	extern ICustomCulling* GCustomCullingImpl;
}