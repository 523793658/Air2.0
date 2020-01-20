#include "RendererModule.h"
#include "Classes/Engine/World.h"
#include "scene.h"
#include "SceneView.h"
#include "SceneInterface.h"
#include "CanvasType.h"
#include "SceneViewExtension.h"
#include "sceneRendering.h"
#include "RenderingThread.h"
#include "RHIResource.h"
#include "ScenePrivate.h"
#include "Misc/App.h"
namespace Air
{
	std::shared_ptr<ISceneViewExtension> getRendererViewExtension()
	{
		class RenderViewExtension : public ISceneViewExtension
		{
		public:
			virtual void beginRenderViewFamily(SceneViewFamily& inViewFamily){}

			virtual void preRenderViewFamily_RenderThread(RHICommandListImmediate& RHICmdList, SceneViewFamily& inViewFamily) {}

			virtual void preRenderView_RenderThread(RHICommandListImmediate& RHICmdList, SceneView& inView) {};


		};

		std::shared_ptr<RenderViewExtension> ptr = MakeSharedPtr<RenderViewExtension>();
		return std::static_pointer_cast<ISceneViewExtension>(ptr);
	}

	static void ViewExtensionPreRender_RenderThread(RHICommandListImmediate& RHICmdList, SceneRenderer* sceneRenderer)
	{
		MemMark memStackMark(MemStack::get());
		for (int viewExt = 0; viewExt < sceneRenderer->mViewFamily.mViewExtensions.size(); viewExt++)
		{
			sceneRenderer->mViewFamily.mViewExtensions[viewExt]->preRenderViewFamily_RenderThread(RHICmdList, sceneRenderer->mViewFamily);

			for (int viewIndex = 0; viewIndex < sceneRenderer->mViewFamily.mViews.size(); viewIndex++)
			{
				sceneRenderer->mViewFamily.mViewExtensions[viewExt]->preRenderView_RenderThread(RHICmdList, sceneRenderer->mViews[viewIndex]);
			}
		}
	}


	static void RenderViewFamily_RenderThread(RHICommandListImmediate& RHICmdList, SceneRenderer* sceneRenderer)
	{
		MemMark memStackMark(MemStack::get());

		sceneRenderer->render(RHICmdList);
		SceneRenderer::waitForTasksClearSnapshotsAndDeleteSceneRenderer(RHICmdList, sceneRenderer);
	}

	void flushPendingDeleteRHIResource_RenderThread()
	{
		if (!GRHIThread)
		{
			RHIResource::flushPendingDeletes();
		}
	}

	RendererModule::RendererModule()
	{

	}

	void RendererModule::beginRenderingViewFamily(Canvas* canvas, SceneViewFamily* viewFamily)
	{
		World* world = nullptr;
		Scene* const scene = viewFamily->mScene->getRenderScene();
		if (scene)
		{
			world = scene->getWorld();
			if (world)
			{
				world->sendAllEndOfFrameUpdates();
			}
		}
		canvas->flush_GameThread();

		++GFrameNumber;

#if 1
		{
			extern std::shared_ptr<ISceneViewExtension> getRendererViewExtension();
			viewFamily->mViewExtensions.push_back(getRendererViewExtension());
		}
#endif

		for (int viewExt = 0; viewExt < viewFamily->mViewExtensions.size(); ++viewExt)
		{
			viewFamily->mViewExtensions[viewExt]->beginRenderViewFamily(*viewFamily);
		}

		if (scene)
		{


			SceneRenderer* sceneRenderer = SceneRenderer::createSceneRenderer(viewFamily);

			ENQUEUE_RENDER_COMMAND(
				ViewExtensionPreDrawCommand)([
				sceneRenderer](RHICommandListImmediate& RHICmdList)
				{
					ViewExtensionPreRender_RenderThread(RHICmdList, sceneRenderer);
				});

			ENQUEUE_RENDER_COMMAND(
				DrawSceneCommand)([sceneRenderer](RHICommandListImmediate& RHICmdList)
				{
					RenderViewFamily_RenderThread(RHICmdList, sceneRenderer);
					flushPendingDeleteRHIResource_RenderThread();
				});
		}

	}

	SceneViewStateInterface* RendererModule::allocateViewState()
	{
		return new SceneViewState();
	}

	SceneInterface* RendererModule::allocateScene(World* world, bool bInRequiresHitProxies, bool bCreateFXSystem, ERHIFeatureLevel::Type inFeatureLevel)
	{
		BOOST_ASSERT(isInGameThread());
		if (GIsClient && App::canEverRender() && !GUsingNullRHI)
		{
			Scene* newScene = new Scene(world, bInRequiresHitProxies, false, bCreateFXSystem, inFeatureLevel);
			mAllocatedScenes.emplace(newScene);
			return newScene;
		}
		else
		{
			return nullptr;
		}
	}
}