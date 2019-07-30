#pragma once
#include "EngineMininal.h"
#include "Engine.h"
#include "Widgets/SWindow.h"
#include "MovieSceneCaptureHandle.h"
namespace Air
{
	class GameInstance;

	class ENGINE_API GameEngine
		:public Engine
	{
		GENERATED_RCLASS_BODY(GameEngine, Engine)
	public:

		static std::shared_ptr<SWindow> createGameWindow();

		static void conditionallyOverrideSetting(int32& resolutionX, int32 &resolutionY, EWindowMode::Type& WindowMode);

		virtual void tick(float deltaSeconds, bool bIdleMode) override;
		
		virtual void redrawViewports(bool bShouldPresent = true) override;

		virtual void init(class EngineLoop* inEngineLoop) override;

		void createGameViewport(GameViewportClient* gameViewportClient);

		void createGameViewportWidget(GameViewportClient* gameViewportClient);

		void switchGameWindowToUseGameViewport();

		virtual void start() override;
	private:
		GameInstance* mGameInstance;

	protected:
		MovieSceneCaptureHandle mStartupMovieCaptureHandle;

	public:
		std::shared_ptr<class GameViewportClient> mGameViewport;


		std::weak_ptr<class SWindow> mGameViewportWindow;
		std::shared_ptr<class SViewport> mGameViewportWidget;
		std::shared_ptr<class SceneViewport> mSceneViewport;
	};
}