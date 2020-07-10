#include "game_service/Manager/PiGameServiceManager.h"
#include "game_service/V8/V8Service.h"
#include "TimerService.h"
#include "game_service/PiScriptingEngine/backtrace.h"

using namespace v8;

// 构造函数
TimerService::TimerService()
{

}

// 析构函数
TimerService::~TimerService()
{

}

// 预初始化，用于同步的、无相互依赖的初始化，以及注册依赖关系
SyncResult TimerService::PreInitialize()
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	tpManager->AddDependency(this, L"V8");

	return SyncResult_Success;
}

// 初始化，如果返回Pending，则会下一帧继续调用，Complete表示结束
AsyncResult TimerService::Initialize()
{
	mnTimerCount = 0;

	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	mpV8Service = (V8Service *)tpManager->GetServiceByName(L"V8");

	// 注册两个给js使用的函数
	Isolate *tpIsolate = mpV8Service->GetIsolate();
	HandleScope tHandleScope(tpIsolate);

	Local<Context> tContext = mpV8Service->GetContext();
	Context::Scope tContextScope(tContext);

	Local<FunctionTemplate> tClassTemplate = FunctionTemplate::New(tpIsolate);
	tClassTemplate->SetClassName(v8::String::NewFromUtf8(tpIsolate, "Timer", v8::NewStringType::kNormal).ToLocalChecked());
	tClassTemplate->Set(tpIsolate, "AddTimer", FunctionTemplate::New(tpIsolate, TimerService::AddTimer));
	tClassTemplate->Set(tpIsolate, "KillTimer", FunctionTemplate::New(tpIsolate, TimerService::KillTimer));

	mpV8Service->AddFunctionBinding("Timer", tClassTemplate);

	return AsyncResult_Complete;
}

// 结束并释放资源，如果返回Pending，则会下一帧继续调用，Complete表示结束
AsyncResult TimerService::Finalize()
{
	mpV8Service = nullptr;

	return AsyncResult_Complete;
}

// 帧更新，如果返回Pending，则会继续收到更新回调
// 如果返回Complete，则以后不再更新这个Service，如果返回Failure，则结束程序
AsyncResult TimerService::Update(const float vfElaspedTime)
{
	int tnCount = (int)mTimerArray.size();
	for (int i = tnCount - 1; i >= 0; i--)
	{
		if (mTimerArray[i].mbNeedToDelete == true)
		{
			mTimerArray[i].mCallbackFunction.Reset();
			mTimerArray.erase(mTimerArray.begin() + i);
		}
	}

	tnCount = (int)mTimerArray.size();
	for (int i = 0; i < tnCount; i++)
	{
		mTimerArray[i].mfTimer -= vfElaspedTime;
		if (mTimerArray[i].mfTimer < 0.0f && mTimerArray[i].mbNeedToDelete == false)
		{
			// 调用脚本回调
			Isolate *tpIsolate = mpV8Service->GetIsolate();
			HandleScope tHandleScope(tpIsolate);
			Local<Context> tContext = mpV8Service->GetContext();
			Context::Scope tContextScope(tContext);

			Local<Function> tCallbackFunction = Local<Function>::New(tpIsolate, mTimerArray[i].mCallbackFunction);

			v8::TryCatch try_catch(tpIsolate);

			Local<Value> tResult;
			if (!tCallbackFunction->Call(tContext, tContext->Global(), 0, 0).ToLocal(&tResult))
			{
				// @@输出错误
				Backtrace::GetInstance().ReportException(tpIsolate, &try_catch);
			}

			// 如果要求只执行一次
			if (mTimerArray[i].mnLoop == 0)
			{
				mTimerArray[i].mbNeedToDelete = true;
			}
			else
			{
				// 重新计时
				mTimerArray[i].mfTimer = mTimerArray[i].mfTimeSpan;
			}
		}
	}

	return AsyncResult_Pending;
}

// 获取显示名称，可以用来外部获取标示、调试、Log用
std::wstring TimerService::GetDisplayName()
{
	return L"Timer";
}

// 添加一个Timer
int TimerService::AddTimer(const double vdbTimeSpand, Isolate *vpIsolate, Local<Function> &vCallbackFunction, int vnLoop)
{
	TimerInfo tNewTimer;
	tNewTimer.mfTimeSpan = (float)vdbTimeSpand;
	tNewTimer.mfTimer = tNewTimer.mfTimeSpan;
	tNewTimer.mnTimerID = mnTimerCount++;
	tNewTimer.mCallbackFunction = Persistent<Function, CopyablePersistentTraits<Function> >(vpIsolate, vCallbackFunction);
	tNewTimer.mnLoop = vnLoop;
	tNewTimer.mbNeedToDelete = false;

	mTimerArray.push_back(tNewTimer);

	return tNewTimer.mnTimerID;
}

// 删除一个Timer
void TimerService::KillTimer(const int vnTimerID)
{
	size_t tnCount = mTimerArray.size();
	for (size_t i = 0; i < tnCount; i++)
	{
		if (mTimerArray[i].mnTimerID == vnTimerID)
		{
			mTimerArray[i].mbNeedToDelete = true;

			return;
		}
	}
}

// 添加一个Timer
// 脚本原型：int AddTimer(double timespan, function callback, int vnLoop);
// 参数说明：timespan：时间间隔
// 参数说明：callback：脚本回调函数对象
// 参数说明：vnLoop：Timer是否一直生效，0表示只执行一次，然后就不再调用，1表示一直都生效
void TimerService::AddTimer(const FunctionCallbackInfo<Value> &vrArgs)
{
	v8::HandleScope handle_scope(vrArgs.GetIsolate());

	if (vrArgs.Length() != 3)
	{
		// @@ 输出错误
		vrArgs.GetReturnValue().SetUndefined();

		return;
	}

	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	TimerService *tpTimeService = (TimerService *)tpManager->GetServiceByName(L"Timer");

	Local<Function> tCallbackFunction = Local<Function>::Cast(vrArgs[1]);

	int tnTimerID = tpTimeService->AddTimer(vrArgs[0]->NumberValue(), vrArgs.GetIsolate(), tCallbackFunction, vrArgs[2]->Int32Value());

	vrArgs.GetReturnValue().Set(v8::Int32::New(vrArgs.GetIsolate(), tnTimerID));
}

// 删除一个Timer
// 脚本原型：void KillTimer(int vnTimerID);
void TimerService::KillTimer(const FunctionCallbackInfo<Value> &vrArgs)
{
	v8::HandleScope handle_scope(vrArgs.GetIsolate());

	vrArgs.GetReturnValue().SetUndefined();

	if (vrArgs.Length() != 1)
	{
		// @@ 输出错误
		return;
	}

	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	TimerService *tpTimeService = (TimerService *)tpManager->GetServiceByName(L"Timer");

	tpTimeService->KillTimer(vrArgs[0]->Int32Value());
}