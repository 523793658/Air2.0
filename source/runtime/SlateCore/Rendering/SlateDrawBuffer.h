#pragma once
#include "SlateCore.h"
#include "Rendering/DrawElements.h"

namespace Air
{
	class SWindow;
	class SLATE_CORE_API SlateDrawBuffer
	{
	private: 
		static int globalIndex;
	public:

		int mBufferIndex;

		explicit SlateDrawBuffer()
			:mLocked(0)
			, mBufferIndex(globalIndex++)
		{}

		void clearBuffer();

		SlateWindowElementList& addWindowElementList(std::shared_ptr<SWindow> forWindow);

		TArray<std::shared_ptr<SlateWindowElementList>>& getWindowElementLists()
		{
			return mWindowElementLists;
		}

		bool lock();

		void unlock();
	protected:
		TArray < std::shared_ptr<SlateWindowElementList>> mWindowElementLists;

		TArray<std::shared_ptr<SlateWindowElementList>> mWindowElementListPool;

		volatile int32 mLocked;
	public:
		float2 mViewOffset;
	};
}