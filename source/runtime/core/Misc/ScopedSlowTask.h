#pragma once
#include "CoreType.h"
#include "Misc/SlowTask.h"
#include "Internaltionalization/Text.h"
namespace Air
{
	struct ScopedSlowTask : SlowTask
	{
	public:
		FORCEINLINE ScopedSlowTask(float inAmountOfWork, const Text& inDefaultMessage = Text(), bool bInEnabled = true)
			:SlowTask(inAmountOfWork, inDefaultMessage, bInEnabled)
		{

		}
	};
}