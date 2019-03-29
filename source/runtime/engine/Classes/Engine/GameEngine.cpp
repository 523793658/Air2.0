#include "GameEngine.h"
#include "AirEngine.h"
#include "Framework/Application/SlateApplication.h"
#include "Classes/Engine/GameViewportClient.h"
#include "Classes/Engine/GameInstance.h"
#include "Classes/Engine/WorldContext.h"
#include "HAL/AirMemory.h"
#include "Classes/GameMapsSetting.h"
#include "ObjectGlobals.h"
#include "Widgets/SOverlay.h"
#include "Slate/SGameLayerManager.h"
#include "Slate/SceneViewport.h"
#include "Misc/App.h"
#include "HAL/PlatformProperties.h"
#include "SimpleReflection.h"

namespace Air
{



	EWindowMode::Type getWindowModeType(EWindowMode::Type windowMode)
	{
		return PlatformProperties::supportsWindowedMode() ? windowMode : EWindowMode::Fullscreen;
	}
	GameEngine::GameEngine(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{}

	void GameEngine::init(class EngineLoop* inEngineLoop)
	{
		Engine::init(inEngineLoop);

		{
			mGameInstance = new GameInstance(this);
			mGameInstance->initializeStandalone();
		}

		GameViewportClient* viewportClient = nullptr;
		if (GIsClient)
		{
			viewportClient = new GameViewportClient();
			viewportClient->init(*mGameInstance->getWorldContext(), mGameInstance);
			mGameViewport = viewportClient;
			mGameInstance->getWorldContext()->mGameViewport = viewportClient;
		}

		if (viewportClient)
		{
			bool bWindowAlreadyExits = !!mGameViewportWindow.lock();
			if (!bWindowAlreadyExits)
			{
				mGameViewportWindow = createGameWindow();
			}
			createGameViewport(viewportClient);
			if (!bWindowAlreadyExits)
			{
				switchGameWindowToUseGameViewport();
			}

			wstring error;
			if (viewportClient->setInitialLocalPlayer(error) == nullptr)
			{
				BOOST_ASSERT(false, error);
			}
		}
	}

	void GameEngine::switchGameWindowToUseGameViewport()
	{
		if (mSceneViewport)
		{
			mSceneViewport->resizeFrame((uint32)GSystemResolution.mResX, (uint32)GSystemResolution.mResY, GSystemResolution.mWindowType);
		}
	}

	void GameEngine::createGameViewportWidget(GameViewportClient* gameViewportClient)
	{
		bool bRenderDirectlyToWindow = !mStartupMovieCaptureHandle.isValid();
		const bool bStereoAllowd = bRenderDirectlyToWindow;
		std::shared_ptr<SOverlay> viewportOverlayWidgetRef = MakeSharedPtr<SOverlay>();

		std::shared_ptr<SGameLayerManager> gameLayerManagerRef = MakeSharedPtr<SGameLayerManager>();

		std::shared_ptr<SViewport> gameViewportWidgetRef = MakeSharedPtr<SViewport>();

		mGameViewportWidget = gameViewportWidgetRef;
		gameViewportClient->setViewportOverlayWidget(mGameViewportWindow.lock(), viewportOverlayWidgetRef);
		gameViewportClient->setGameLayerManager(gameLayerManagerRef);
	}

	void GameEngine::createGameViewport(GameViewportClient* gameViewportClient)
	{
		if (!mGameViewportWidget)
		{
			createGameViewportWidget(gameViewportClient);
		}
		std::shared_ptr<SViewport> gameViewportWidgetRef = mGameViewportWidget;
		auto window = mGameViewportWindow.lock();

		mSceneViewport = MakeSharedPtr<SceneViewport>(gameViewportClient, gameViewportWidgetRef);

		gameViewportClient->mViewport = mSceneViewport.get();

		gameViewportWidgetRef->setViewportInterface(mSceneViewport);

		ViewportFrame* viewportFrame = mSceneViewport.get();

		mGameViewport->setViewportFrame(viewportFrame);
	}

	std::shared_ptr<SWindow> GameEngine::createGameWindow()
	{
		int32 resX = GSystemResolution.mResX;
		int32 resY = GSystemResolution.mResY;

		EWindowMode::Type windowMode = GSystemResolution.mWindowType;

		conditionallyOverrideSetting(resX, resY, windowMode);

		SWindowAttribute attr;
		Memory::memzero(attr);
		attr.mType = EWindowType::GameWindow;
		attr.mSize = int2(1024, 768);
		attr.mScreenPosition = int2(200, 200);
		attr.mFocusWhenFirstShown = true;
		attr.mTitle = L"test";



		std::shared_ptr<SWindow> window = MakeSharedPtr<SWindow>(attr);

		const bool bShowImmediately = false;

		SlateApplication::get().addWindow(window, bShowImmediately);

		if (windowMode == EWindowMode::Fullscreen)
		{
			window->setWindowMode(EWindowMode::WindowedFullscreen);
		}
		else
		{
			window->setWindowMode(windowMode);
		}
		window->showWindow();

		SlateApplication::get().tick();
		return window;
	}



	void GameEngine::conditionallyOverrideSetting(int32& resolutionX, int32 &resolutionY, EWindowMode::Type& WindowMode)
	{
		
	}



	void GameEngine::tick(float deltaSeconds, bool bIdleMode)
	{
		if (App::canEverRender())
		{
			cleanupGameViewport();
		}

		if (GIsClient)
		{

		}

		//更新子系统
		{

		}

		if (!bIdleMode)
		{
			redrawViewports();
		}
	}

	void GameEngine::redrawViewports(bool bShouldPresent /* = true */)
	{
		if (mGameViewport != nullptr)
		{
			mGameViewport->layoutPlayers();
			if (mGameViewport->mViewport != nullptr)
			{
				mGameViewport->mViewport->draw(bShouldPresent);
			}
		}
	}

	void GameEngine::start()
	{
		mGameInstance->startGameInstance();
	}

	DECLARE_SIMPLER_REFLECTION(GameEngine);
}