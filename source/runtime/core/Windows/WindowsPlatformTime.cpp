#include "WindowsPlatformTime.h"
#include "boost/boost/assert.hpp"
#include "Containers/Ticker.h"
#include "Windows/WindowsHWrapper.h"
namespace Air
{
	double WindowsPlatformTime::initTiming()
	{
		Windows::LARGE_INTEGER frequency;
		BOOST_VERIFY(Windows::QueryPerformanceFrequency(&frequency));
		mSecondsPerCycle = 1.0 / frequency.QuadPart;
		mSecondsPerCycle64 = 1.0 / frequency.QuadPart;
		static const float PollingInterval = 1.0f / 4.0f;
		//Ticker::getCoreTicker().addTicker();
		return PlatformTime::seconds();
	}

	void WindowsPlatformTime::systemTime(int32& year, int32& month, int32& dayOfWeek, int32& day, int32& hour, int32& min, int32 & sec, int32& msec)
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		year = st.wYear;
		month = st.wMonth;
		dayOfWeek = st.wDayOfWeek;
		day = st.wDay;
		hour = st.wHour;
		min = st.wMinute;
		sec = st.wSecond;
		msec = st.wMilliseconds;
	}
}