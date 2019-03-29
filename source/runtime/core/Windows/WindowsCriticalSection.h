#pragma once
#include "CoreType.h"
#include "MinimalWindowsApi.h"
namespace Air
{
	class WindowsCriticalSection
	{
		Windows::CRITICAL_SECTION mCriticalSection;
	public:
		FORCEINLINE WindowsCriticalSection()
		{
			CA_SUPPRESS(28125);
			Windows::InitializeCriticalSection(&mCriticalSection);
			Windows::SetCriticalSectionSpinCount(&mCriticalSection, 4000);
		}

		FORCEINLINE ~WindowsCriticalSection()
		{
			Windows::DeleteCriticalSection(&mCriticalSection);
		}

		FORCEINLINE void lock()
		{
			if (Windows::TryEnterCriticalSection(&mCriticalSection) == 0)
			{
				Windows::EnterCriticalSection(&mCriticalSection);
			}
		}

		FORCEINLINE bool tryLock()
		{
			if (Windows::TryEnterCriticalSection(&mCriticalSection))
			{
				return true;
			}
			return false;
		}

		FORCEINLINE void unlock()
		{
			Windows::LeaveCriticalSection(&mCriticalSection);
		}

	private:
		WindowsCriticalSection(const WindowsCriticalSection&);

		WindowsCriticalSection& operator = (const WindowsCriticalSection&);
	};

	typedef WindowsCriticalSection CriticalSection;

}