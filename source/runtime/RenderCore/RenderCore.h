#pragma once

#include "CoreMinimal.h"
#ifdef RenderCore_EXPORTS
#define RENDER_CORE_API		DLLEXPORT
#else
#define RENDER_CORE_API		DLLIMPORT
#endif

namespace Air
{
	class Timer
	{
	public:
		Timer()
			:mCurrentDeltaTime(0.0f)
			,mCurrentTime(0.0f)
		{}

		float getCurrentTime() const
		{
			return mCurrentTime;
		}

		float getCurrentDeltaTime() const
		{
			return mCurrentDeltaTime;
		}

		void tick(float deltaTime)
		{
			mCurrentDeltaTime = deltaTime;
			mCurrentTime += deltaTime;
		}
	protected:
		float mCurrentDeltaTime;
		float mCurrentTime;
	};


	extern RENDER_CORE_API Timer GRenderingRealtimeClock;
	extern RENDER_CORE_API bool GPauseRenderingRealtimeClock;
}