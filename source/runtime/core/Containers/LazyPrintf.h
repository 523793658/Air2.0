#pragma once
#include "CoreType.h"
#include "Containers/String.h"
namespace Air
{
	class LazyPrintf
	{
	public:
		LazyPrintf(const TCHAR* inputWithPercentS)
			:mCurrentInputPos(inputWithPercentS)
		{
			mCurrentState.reserve(50 * 1024);
		}

		wstring getResultString()
		{
			BOOST_ASSERT(!processUntilPercentS());
			mCurrentState += mCurrentInputPos;
			return mCurrentState;
		}

		void pushParam(const TCHAR* data)
		{
			if (processUntilPercentS())
			{
				mCurrentState += data;
			}
			else
			{
				BOOST_ASSERT(false);
			}
		}


	private:

		bool processUntilPercentS()
		{
			const TCHAR* found = CString::strstr(mCurrentInputPos, TEXT("%s"));
			if (found == 0)
			{
				return false;
			}
			while (mCurrentInputPos < found)
			{
				mCurrentState += *mCurrentInputPos++;
			}
			mCurrentInputPos += 2;
			return true;
		}



		const TCHAR* mCurrentInputPos;
		wstring mCurrentState;
	};
}