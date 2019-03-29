#pragma once
#include "CoreType.h"
#include "Internaltionalization/Text.h"
namespace Air
{
	enum class ESlowTaskVisibility
	{
		Default,
		ForceVisible,
		Invisible,
	};

	struct CORE_API SlowTask
	{
	public:
		SlowTask(float inAmountOfWork, const Text& inDefaultMessage, bool bInEnabled);

		Text mDefaultMessage;
		Text mFrameMessage;
		float mTotalAmountOfWork;

		float mCompletedWork;
		float mCurrentFrameScope;

		bool bEnabled;

		ESlowTaskVisibility mVisibility;

		double mStartTime;

		void enterProgressFrame(float expectedWorkThisFrame = 1.0f, Text text = Text());
	};


}