#pragma once
#include "RenderCore.h"
#include "Async/TaskGraph.h"
namespace Air
{
	class RENDER_CORE_API RenderCommandFence
	{
	public:
		void beginFence();

		void wait(bool bProcessGameThreadTasks = false) const;

		bool isFenceComplete() const;
	private:
		mutable GraphEventRef mCompletionEvent;
	};
}