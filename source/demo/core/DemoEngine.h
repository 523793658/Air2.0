#pragma once
#include "core/DemoConfig.h"
#include "Classes/Engine/Engine.h"
#include "DemoInput.h"
namespace Air
{
	class DemoViewportClient;
	class Application;
	class DEMO_API DemoEngine : public Engine
	{
		GENERATED_RCLASS_BODY(DemoEngine, Engine)
	public:
		virtual void init(class EngineLoop* inEngineLoop) override;
		virtual void start() override;
		virtual void tick(float deltaSeconds, bool bIdleMode) override;
		virtual void redrawViewports(bool bShouldPresent = true ) override;

		WorldContext& getDemoWorldContext(bool bEnsureIsGWorld = false);

		void createViewport();

		void switchGameWindowToUseGameViewport();
		void createGameViewportWidget();

		void intoDemo(URL url);

		DemoViewportClient* getViewportClient()
		{
			return mViewportClient;
		}
	private:
		void initDemoApplication();
		GameInstance* createGameInstance();
	private:
		std::weak_ptr<class SWindow> mViewportWindow;
		std::shared_ptr<class SViewport> mViewportWidget;
		std::shared_ptr<class SceneViewport> mSceneViewport;
		DemoViewportClient* mViewportClient{ nullptr };

		Application* mApplication;
		class PlayerInput* mPlayerInput;
		GameInstance* mGameInstance;
	};
}