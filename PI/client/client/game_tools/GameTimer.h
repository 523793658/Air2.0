#ifndef GAMETIMER_H
#define GAMETIMER_H

#include <Windows.h>
#include "pi_lib.h"
class GameTimer
{
public:
	GameTimer();

	float TotalTime()const;  // ��λΪ��
	float DeltaTime()const; // ��λΪ��
	//��ȡ΢��
	int64 GetMachineTime() const;

	void Reset(); // ��Ϣѭ��ǰ����
	void Start(); // ȡ����ͣʱ����
	void Stop();  // ��ͣʱ����
	void Tick();  // ÿ֡����

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
