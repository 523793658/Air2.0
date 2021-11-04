#pragma once
#include "Containers/Array.h"
#include "UObject/Object.h"
#include "Classes/Engine/EngineBaseTypes.h"
#include "DemoViewportClient.h"
#include "DemoEngine.h"

namespace Air
{
	class Application;
	class DemoEngine;

	class Application : public Object
	{
		GENERATED_RCLASS_BODY(Application, Object)
	protected:
		ViewportCameraTransform* mCamera;
		DemoEngine* mEngine;
		World* mWorld;
		URL mCurrentURL;
	public:
		virtual ~Application() PURE_VIRTRUAL();
		
		virtual void init(DemoEngine* inEngine) 
		{
			mEngine = inEngine; 
			mWorld = mEngine->getViewportClient()->getWorld();
			mCamera = &mEngine->getViewportClient()->getViewTransform();
		}

		virtual void update(float deltaTime, bool bIdleMode) {}

		virtual void release() {}

		virtual string getTitle() PURE_VIRTRUAL(Application::getTitle, return "";);

		virtual void start();
	};

}