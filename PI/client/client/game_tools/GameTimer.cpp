#include "GameTimer.h"
GameTimer::GameTimer() :mSecondsPerCount(0.0), mDeltaTime(-1.0), mBaseTime(0),
mPausedTime(0), mPrevTime(0), mCurrTime(0), mStopped(FALSE)
{
	int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	mSecondsPerCount = 1.0 / (double)countsPerSec;
}

void GameTimer::Tick()
{
	if (mStopped)
	{
		mDeltaTime = 0.0;
		return;
	}
	int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	mCurrTime = currTime;
	mDeltaTime = (mCurrTime - mPrevTime)*mSecondsPerCount;

	mPrevTime = mCurrTime;
	if (mDeltaTime < 0.0)
	{
		mDeltaTime = 0.0;
	}
}

float GameTimer::DeltaTime() const
{
	return (float)mDeltaTime;
}

void GameTimer::Reset()
{
	int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	mBaseTime = currTime;
	mPrevTime = currTime;
	mStopTime = 0;
	mStopped = false;
}

void GameTimer::Stop()
{
	if (!mStopped)
	{
		int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
		mStopTime = currTime;
		mStopped = TRUE;
	}
}

void GameTimer::Start()
{
	int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);
	if (mStopped)
	{
		mPausedTime += (startTime - mStopTime);
		mPrevTime = startTime;
		mStopTime = 0;
		mStopped = FALSE;
	}
}

float GameTimer::TotalTime()const
{
	if (mStopped)
	{
		return (float)((mStopTime - mBaseTime - mPausedTime)*mSecondsPerCount);
	}
	else
	{
		return (float)((mCurrTime - mBaseTime - mPausedTime)*mSecondsPerCount);
	}
}
//获取程序启动的微秒数
int64 GameTimer::GetMachineTime() const
{
	LARGE_INTEGER currTime;
	QueryPerformanceCounter(&currTime);

	LARGE_INTEGER countsPerSec;
	QueryPerformanceFrequency(&countsPerSec);

	return ((double)currTime.QuadPart) / countsPerSec.QuadPart * 1000000;
}
