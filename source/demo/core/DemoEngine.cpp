#include "DemoEngine.h"
#include "Classes/Engine/GameEngine.h"
#include "Widgets/SViewport.h"
#include "Slate/SceneViewport.h"
#include "Widgets/SOverlay.h"
#include "AirEngine.h"
#include "Misc/App.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProperties.h"
#include "Framework/Application/SlateApplication.h"
#include "RenderingThread.h"
#include "EngineModule.h"
#include "RendererModule.h"
#include "RenderCore.h"
#include "core/DemoViewportClient.h"
#include "Classes/Engine/GameInstance.h"
#include "ApplicationManager.h"
#include "SimpleReflection.h"
#include "Classes/Components/SkyLightComponent.h"
namespace Air
{
	DemoEngine::DemoEngine(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{

	}

	void DemoEngine::initDemoApplication()
	{
		mApplication = ApplicationManager::initApplication();
	}
	
	void DemoEngine::intoDemo(URL url)
	{
		browse(getDemoWorldContext(), url);
	}

	void DemoEngine::init(class EngineLoop* inEngineLoop)
	{
		initDemoApplication();

		Engine::init(inEngineLoop);

		mLocalPlayerClass = LocalPlayer::StaticClass();

		RClass* gameInstanceClass = nullptr;
		if (gameInstanceClass == nullptr)
		{
			gameInstanceClass = GameInstance::StaticClass();
		}
		mGameInstance = newObject<GameInstance>(this, gameInstanceClass);
		mGameInstance->initializeStandalone(EWorldType::Demo);

		GWorld = mGameInstance->getWorldContext()->getWorld();

		mViewportClient = nullptr;
		if (GIsClient)
		{
			mViewportClient = newObject<DemoViewportClient>(this);
			mViewportClient->init(*mGameInstance->getWorldContext(), mGameInstance.get());
			mGameInstance->getWorldContext()->mGameViewport = mViewportClient;
		}
		if (mViewportClient)
		{
			bool bWindowAlreadyExists = !mViewportWindow.expired();
			if (!bWindowAlreadyExists)
			{
				mViewportWindow = GameEngine::createGameWindow();
			}
			createViewport();
			if (!bWindowAlreadyExists)
			{
				switchGameWindowToUseGameViewport();
			}

		}



		wstring error;
		std::shared_ptr<LocalPlayer>& localPlayer = mViewportClient->setupInitialLocalPlayer(error);


		WorldContext* worldContext = mGameInstance->getWorldContext();

		if (!worldContext->getWorld()->bIsWorldInitialized)
		{
			worldContext->getWorld()->initWorld();
		}

		worldContext->getWorld()->setGameMode(URL());
		
		worldContext->getWorld()->initializeActorsForPlay(URL());

		if (worldContext->mOwningGameInstance != nullptr)
		{
			for (auto it = worldContext->mOwningGameInstance->getLocalPlayerIterator(); it; it++)
			{
				wstring error2;
				const std::shared_ptr<LocalPlayer>& lp = *it;
				wstring url = TEXT("");
				bool b = lp->spawnPlayActor(url, error2, worldContext->getWorld());
				if (!b)
				{

				}
			}
		}

		if (mApplication != nullptr)
		{
			mApplication->init(this);
		}
		bIsInitiailized = true;
	}




	void DemoEngine::start()
	{
		WorldContext& worldContext = getDemoWorldContext();

		

	
		worldContext.getWorld()->beginPlay();
		redrawViewports(false);


		if (mApplication != nullptr)
		{
			mApplication->start();
		}
	}

	void DemoEngine::tick(float deltaSeconds, bool bIdleMode)
	{

		mApplication->update(deltaSeconds, bIdleMode);

		if (App::canEverRender())
		{
			cleanupGameViewport();
		}
		if (GIsClient && (mViewportClient == nullptr) && App::canEverRender())
		{
			PlatformMisc::requestExit(0);
		}
		{
			staticTick(deltaSeconds);
		}

		wstring originalGWorldContext = Name_None;

		for (int32 i = 0; i < mWorldList.size(); ++i)
		{
			if (mWorldList[i].getWorld() == GWorld)
			{
				originalGWorldContext = mWorldList[i].mContextHandle;
				break;
			}
		}

		for (int32 worldIdx = 0; worldIdx < mWorldList.size(); ++worldIdx)
		{
			WorldContext& context = mWorldList[worldIdx];
			if (context.getWorld() == nullptr || !context.getWorld()->shouldTick())
			{
				continue;
			}

			GWorld = context.getWorld();

			if (!isRunningDedicatedServer())
			{
				SkyLightComponent::updateSkyCaptureContents(context.getWorld());
			}

			if (!bIdleMode)
			{
				context.getWorld()->tick(LEVELTICK_ALL, deltaSeconds);
			}


		}





		if (mViewportClient != nullptr && !bIdleMode)
		{
			//mGameViewport->
		}

		if (PlatformProperties::supportsWindowedMode())
		{
			static bool bFirstTime = true;
			if (bFirstTime)
			{
				bFirstTime = false;
				if (!mViewportWindow.expired())
				{
					mViewportWindow.lock()->showWindow();
					//SlateApplication::get().regist()
				}
			}
		}
		if (!bIdleMode)
		{
			redrawViewports();
		}
		{
			bool bPauseRenderingRealtimeClock = GPauseRenderingRealtimeClock;
			ENQUEUE_RENDER_COMMAND(
				TickRenderingTimer)([bPauseRenderingRealtimeClock, deltaSeconds](RHICommandListImmediate& cmd)
				{
					if (!bPauseRenderingRealtimeClock)
					{
						GRenderingRealtimeClock.tick(deltaSeconds);
					}
					getRendererModule().tickRenderTargetPool();
				}
			);
		}

	}

	void DemoEngine::redrawViewports(bool bShouldPresent /* = true */)
	{
		if (nullptr != mViewportClient && mViewportClient->mViewport != nullptr)
		{
			mViewportClient->mViewport->draw(bShouldPresent);
		}
	}

	WorldContext& DemoEngine::getDemoWorldContext(bool bEnsureIsGWorld /* = false */)
	{
		for (int32 i = 0; i < mWorldList.size(); ++i)
		{
			if (mWorldList[i].mWorldType == EWorldType::Demo)
			{
				BOOST_ASSERT(!bEnsureIsGWorld || mWorldList[i].getWorld() == GWorld);
				return mWorldList[i];
			}
		}
		BOOST_ASSERT(false);
		return createNewWorldContext(EWorldType::Demo);
	}

	void DemoEngine::createGameViewportWidget()
	{
		bool bRenderDirectlyToWindow = true;
		const bool bStereoAllowd = bRenderDirectlyToWindow;
		std::shared_ptr<SOverlay> viewportOverlayWidgetRef = MakeSharedPtr<SOverlay>();

		

		std::shared_ptr<SViewport> gameViewportWidgetRef = MakeSharedPtr<SViewport>();

		mViewportWidget = gameViewportWidgetRef;
	}

	void DemoEngine::createViewport()
	{
		if (!mViewportWidget)
		{
			createGameViewportWidget();
		}
		std::shared_ptr<SViewport> gameViewportWidgetRef = mViewportWidget;
		auto window = mViewportWindow.lock();

		mSceneViewport = MakeSharedPtr<SceneViewport>(mViewportClient.get(), gameViewportWidgetRef);

		mViewportClient->mViewport = mSceneViewport.get();

		gameViewportWidgetRef->setViewportInterface(mSceneViewport);
	}

	void DemoEngine::switchGameWindowToUseGameViewport()
	{
		if (!mViewportWindow.expired() && mViewportWindow.lock()->getContent() != mViewportWidget)
		{
			if (!mViewportWidget)
			{
				createViewport();
			}
			std::shared_ptr<SViewport> gameViewportWidget = mViewportWidget;
			std::shared_ptr<SWindow> gameViewportWindow = mViewportWindow.lock();

			gameViewportWindow->setContent(gameViewportWidget);

			if (mSceneViewport)
			{
				mSceneViewport->resizeFrame((uint32)GSystemResolution.mResX, (uint32)GSystemResolution.mResY, GSystemResolution.mWindowType);
			}
			SlateApplication::get().registerGameViewport(gameViewportWidget);
		}
	}

	DECLARE_SIMPLER_REFLECTION(DemoEngine);
}