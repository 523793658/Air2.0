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
		int mnLoop; // 0表示Timer只做一次，1表示一直循环生效
		CallBackType mCallbackFunction;
		bool mbNeedToDelete;
	};

	int mnTimerCount;

	std::vector<TimerInfo> mTimerArray;

	// 为了方便，保存V8Service的指针
	V8Service *mpV8Service;

public:
	
	// 构造函数
	TimerService();

	// 析构函数
	virtual ~TimerService();

	// 预初始化，用于同步的、无相互依赖的初始化，以及注册依赖关系
	virtual SyncResult PreInitialize();

	// 初始化，如果返回Pending，则会下一帧继续调用，Complete表示结束
	virtual AsyncResult Initialize();

	// 结束并释放资源，如果返回Pending，则会下一帧继续调用，Complete表示结束
	virtual AsyncResult Finalize();

	// 帧更新，如果返回Pending，则会继续收到更新回调
	// 如果返回Complete，则以后不再更新这个Service，如果返回Failure，则结束程序
	virtual AsyncResult Update(const float vfElaspedTime);

	// 获取显示名称，可以用来外部获取标示、调试、Log用
	virtual std::wstring GetDisplayName();

	// 添加一个Timer
	int AddTimer(const double vdbTimeSpand, v8::Isolate *vpIsolate, v8::Local<v8::Function> &vCallbackFunction, int vnLoop);

	// 删除一个Timer
	void KillTimer(const int vnTimerID);

	// ============================== 以下两个函数暴露给js脚本使用 ==============================

	// 添加一个Timer
	// 脚本原型：int AddTimer(double timespan, function callback, int vnLoop);
	// 参数说明：timespan：时间间隔
	// 参数说明：callback：脚本回调函数对象
	// 参数说明：vnLoop：Timer是否一直生效，0表示只执行一次，然后就不再调用，1表示一直都生效
	static void AddTimer(const v8::FunctionCallbackInfo<v8::Value> &vrArgs);

	// 删除一个Timer
	// 脚本原型：void KillTimer(int vnTimerID);
	static void KillTimer(const v8::FunctionCallbackInfo<v8::Value> &vrArgs);
};

#endif