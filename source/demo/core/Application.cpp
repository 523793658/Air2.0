#include "Application.h"
#include <iostream>
#include "core/DemoEngine.h"
namespace Air
{
	void Application::start()
	{
		mEngine->intoDemo(mCurrentURL);
	}
	
}