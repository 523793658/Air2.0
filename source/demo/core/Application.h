#pragma once
#include "Containers/Array.h"
#include "Object.h"
#include "Classes/Engine/EngineBaseTypes.h"
#include "DemoViewportClient.h"
#include "DemoEngine.h"

namespace Air
{
	class Application;
	class DemoEngine;

	class Application : public Object
	{
	protected:
		ViewportCameraTransform* mCamera;
		DemoEngine* mEngine;
		World* mWorld;
		URL mCurrentURL;
	public:
		virtual ~Application() = 0 {};
		
		virtual void init(DemoEngine* inEngine) 
		{
			mEngine = inEngine; 
			mWorld = mEngine->getViewportClient()->getWorld();
			mCamera = &mEngine->getViewportClient()->getViewTransform();
		}

		virtual void update(float deltaTime, bool bIdleMode) {}

		virtual void release() {}

		virtual string getTitle() = 0;

		virtual void start();
	};

}