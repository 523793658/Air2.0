#include "App.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformMisc.h"
namespace Air
{
	double App::mCurrentTime = 0.0;
	double App::mDeltaTime = 1 / 30.0;
	double App::mLastTime = 0.0;

	double App::mFixedDeltaTime = 1 / 30.0;
	double App::mIdleTime = 0.0;

	wstring App::mGraphicsRHI = wstring();

	bool App::shouldUseThreadingForPerformance()
	{
		static bool onlyOneThread = !PlatformProcess::supportsMultithreading() || PlatformMisc::numberOfCores() < 2;
		return !onlyOneThread;
	}
	
	EBuildConfiguaration::Type App::getBuildConfiguration()
	{
#if BUILD_DEBUG
		return EBuildConfiguaration::Debug;
#elif BUILD_DEVELOPMENT
#if IS_MONOLITHIC
		extern const bool GIsDebugGame;
		return GIsDebugGame ? EBuildConfiguaration::DebugGame : EBuildConfiguaration::DebugGame;
#else
		static const bool bUsingDebugGame = true;
		return bUsingDebugGame ? EBuildConfiguaration::DebugGame : EBuildConfiguaration::Development;
#endif

#elif	BUILD_SHIPPING
		return EBuildConfiguaration::Test;
#else
		return EBuildConfiguaration::Unknown;

#endif

	}

	void App::setGraphicsRHI(wstring RHIString)
	{
		mGraphicsRHI = RHIString;
	}

	wstring App::getGraphicsRHI()
	{
		return mGraphicsRHI;
	}

}