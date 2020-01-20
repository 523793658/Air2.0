#include "RenderCommandFence.h"
#include "RenderingThread.h"
#include "Async/TaskGraphInterfaces.h"
#include "HAL/PlatformProcess.h"
#include "HAL/Event.h"
namespace Air
{

	void RenderCommandFence::beginFence()
	{
		if (!GIsThreadedRendering)
		{
			return;
		}
		else
		{
			static std::mutex NullGraphTaskMutex;
			mCompletionEvent = TGraphTask<NullGraphTask>::createTask(nullptr, ENamedThreads::GameThread).constructAndDispatchWhenReady(ENamedThreads::getRenderThread());
		}
	}

	void RenderCommandFence::wait(bool bProcessGameThreadTasks) const
	{
		if (!isFenceComplete())
		{
			gameThreadWaitForTask(mCompletionEvent);
		}
	}

	void checkRenderingThreadHealth()
	{
	}


	bool RenderCommandFence::isFenceComplete() const
	{
		if (!GIsThreadedRendering)
		{
			return true;
		}
		checkRenderingThreadHealth();
		if (!mCompletionEvent || mCompletionEvent->isComplete())
		{
			mCompletionEvent = nullptr;
			return true;
		}
		return false;
	}
}