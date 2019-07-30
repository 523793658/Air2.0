#include "Application.h"
#include <iostream>
#include "core/DemoEngine.h"
#include "SimpleReflection.h"
namespace Air
{
	Application::Application(const Air::ObjectInitializer& objectInitalizer)
		:ParentType(objectInitalizer)
	{

	}

	void Application::start()
	{
		mEngine->intoDemo(mCurrentURL);
	}
	

	DECLARE_SIMPLER_REFLECTION(Application);
}