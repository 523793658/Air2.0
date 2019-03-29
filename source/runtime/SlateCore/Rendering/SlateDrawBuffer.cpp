#include "Rendering/SlateDrawBuffer.h"
namespace Air
{
	void SlateDrawBuffer::clearBuffer()
	{
		for (int32 windowIndex = 0; windowIndex < mWindowElementListPool.size(); ++windowIndex)
		{
			if (!mWindowElementListPool[windowIndex]->getWindow())
			{
				mWindowElementListPool.removeAt(windowIndex);
				--windowIndex;
			}
		}
		for (auto& existingList : mWindowElementLists)
		{
			if (existingList->getWindow())
			{
				mWindowElementListPool.push_back(existingList);
			}
		}
		mWindowElementLists.clear();
	}

	bool SlateDrawBuffer::lock()
	{
		return PlatformAtomics::interlockedCompareExchange(&mLocked, 1, 0) == 0;
	}

	void SlateDrawBuffer::unlock()
	{
		PlatformAtomics::interlockedExchange(&mLocked, 0);
	}


	SlateWindowElementList& SlateDrawBuffer::addWindowElementList(std::shared_ptr<SWindow> forWindow)
	{
		std::shared_ptr<SlateWindowElementList> windowElements;
		for (int32 windowIndex = 0; windowIndex < mWindowElementListPool.size(); ++windowIndex)
		{
			windowElements = mWindowElementListPool[windowIndex];
			if (windowElements->getWindow() == forWindow)
			{
				mWindowElementLists.push_back(windowElements);
				mWindowElementListPool.removeAt(windowIndex);
				windowElements->resetBuffers();
				return *windowElements;
			}
		}
		windowElements = MakeSharedPtr<SlateWindowElementList>(forWindow);
		mWindowElementLists.push_back(windowElements);
		return *windowElements;
	}

	int SlateDrawBuffer::globalIndex = 0;
}