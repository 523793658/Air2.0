#include "HAL/PlatformTime.h"
#include "CoreGlobals.h"
#include "SlowTask.h"

namespace Air
{
	SlowTask::SlowTask(float inAmountOfWork, const Text& inDefaultMessage, bool bInEnabled)
		: mDefaultMessage(inDefaultMessage)
		, mFrameMessage()
		, mTotalAmountOfWork(inAmountOfWork)
		, mCompletedWork(0)
		, mCurrentFrameScope(0)
		, mVisibility(ESlowTaskVisibility::Default)
		, mStartTime(PlatformTime::seconds())
		, bEnabled(bInEnabled && isInGameThread())
	{
		if (mTotalAmountOfWork == 0.f)
		{
			mTotalAmountOfWork = mCurrentFrameScope = 1.0f;
		}
	}

	void SlowTask::enterProgressFrame(float expectedWorkThisFrame /* = 1.0f */, Text text /* = Text() */)
	{
		mFrameMessage = text;
		mCompletedWork += mCurrentFrameScope;
		const float workRemaining = mTotalAmountOfWork - mCompletedWork;
		mCurrentFrameScope = std::min<float>(workRemaining, expectedWorkThisFrame);

	}

}