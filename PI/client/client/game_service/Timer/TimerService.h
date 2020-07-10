#ifndef _TimerService_H_
#define _TimerService_H_

#include "game_service/BaseClass/PiGameService.h"
class TimerService : public PiGameService
{
private:

	using CallBackType = v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>>;

	struct TimerInfo
	{
		int mnTimerID;
		float mfTimeSpan;
		float mfTimer;
		int mnLoop; // 0��ʾTimerֻ��һ�Σ�1��ʾһֱѭ����Ч
		CallBackType mCallbackFunction;
		bool mbNeedToDelete;
	};

	int mnTimerCount;

	std::vector<TimerInfo> mTimerArray;

	// Ϊ�˷��㣬����V8Service��ָ��
	V8Service *mpV8Service;

public:
	
	// ���캯��
	TimerService();

	// ��������
	virtual ~TimerService();

	// Ԥ��ʼ��������ͬ���ġ����໥�����ĳ�ʼ�����Լ�ע��������ϵ
	virtual SyncResult PreInitialize();

	// ��ʼ�����������Pending�������һ֡�������ã�Complete��ʾ����
	virtual AsyncResult Initialize();

	// �������ͷ���Դ���������Pending�������һ֡�������ã�Complete��ʾ����
	virtual AsyncResult Finalize();

	// ֡���£��������Pending���������յ����»ص�
	// �������Complete�����Ժ��ٸ������Service���������Failure�����������
	virtual AsyncResult Update(const float vfElaspedTime);

	// ��ȡ��ʾ���ƣ����������ⲿ��ȡ��ʾ�����ԡ�Log��
	virtual std::wstring GetDisplayName();

	// ���һ��Timer
	int AddTimer(const double vdbTimeSpand, v8::Isolate *vpIsolate, v8::Local<v8::Function> &vCallbackFunction, int vnLoop);

	// ɾ��һ��Timer
	void KillTimer(const int vnTimerID);

	// ============================== ��������������¶��js�ű�ʹ�� ==============================

	// ���һ��Timer
	// �ű�ԭ�ͣ�int AddTimer(double timespan, function callback, int vnLoop);
	// ����˵����timespan��ʱ����
	// ����˵����callback���ű��ص���������
	// ����˵����vnLoop��Timer�Ƿ�һֱ��Ч��0��ʾִֻ��һ�Σ�Ȼ��Ͳ��ٵ��ã�1��ʾһֱ����Ч
	static void AddTimer(const v8::FunctionCallbackInfo<v8::Value> &vrArgs);

	// ɾ��һ��Timer
	// �ű�ԭ�ͣ�void KillTimer(int vnTimerID);
	static void KillTimer(const v8::FunctionCallbackInfo<v8::Value> &vrArgs);
};

#endif