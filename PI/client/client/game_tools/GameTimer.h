#ifndef GAMETIMER_H
#define GAMETIMER_H

#include <Windows.h>
#include "pi_lib.h"
class GameTimer
{
public:
	GameTimer();

	float TotalTime()const;  // 单位为秒
	float DeltaTime()const; // 单位为秒
	//获取微秒
	int64 GetMachineTime() const;

	void Reset(); // 消息循环前调用
	void Start(); // 取消暂停时调用
	void Stop();  // 暂停时调用
	void Tick();  // 每帧调用

private:
	double mSecondsPerCount;
	double mDeltaTime;

	int64 mBaseTime;
	int64 mPausedTime;
	int64 mStopTime;
	int64 mPrevTime;
	int64 mCurrTime;

	bool mStopped;
};

#endif
