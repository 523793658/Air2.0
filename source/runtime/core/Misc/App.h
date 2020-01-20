#pragma once
#include "CoreType.h"
#include "GenericPlatform/GenericPlatformMisc.h"
namespace Air
{
#define HAVE_RUNTIME_THREADING_SWITCHES		(PLATFORM_DESKTOP)


	class CORE_API App
	{
#if HAVE_RUNTIME_THREADING_SWITCHES
	public:
		static bool shouldUseThreadingForPerformance();
#else
		FORCEINLINE static bool shouldUseThreadingForPerformance()
		{
			return true;
		}
#endif

		FORCEINLINE static bool canEverRender()
		{
			return true;
		}

		static bool isInstalled()
		{
			return false;
		}

		static EBuildConfiguaration::Type getBuildConfiguration();
		
		FORCEINLINE static bool isGame()
		{
#if 1
			return true;
#endif
		}
		FORCEINLINE static double getCurrentTime()
		{
			return mCurrentTime;
		}
		
		FORCEINLINE static double getDeltaTime()
		{
			return mDeltaTime;
		}

		static void setDeltaTime(double seconds)
		{
			mDeltaTime = seconds;
		}

		static void updateLastTime()
		{
			mLastTime = mCurrentTime;
		}

		FORCEINLINE static double getFixedDeltaTime()
		{
			return mFixedDeltaTime;
		}

		FORCEINLINE static void setCurrentTime(double t)
		{
			mCurrentTime = t;
		}

		FORCEINLINE static double getIdleTime()
		{
			return mIdleTime;
		}
		static void setIdleTime(double seconds)
		{
			mIdleTime = seconds;
		}

		static void setGraphicsRHI(wstring RHIString);

		static wstring getGraphicsRHI();
	private:
		static double mCurrentTime;
		static double mDeltaTime;
		static double mLastTime;
		static double mFixedDeltaTime;
		static double mIdleTime;
		static wstring mGraphicsRHI;
	};
}