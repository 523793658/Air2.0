#include "game_service/Manager/PiGameServiceManager.h"
#include "game_service/V8/V8Service.h"
#include "TimerService.h"
#include "game_service/PiScriptingEngine/backtrace.h"

using namespace v8;

// ���캯��
TimerService::TimerService()
{

}

// ��������
TimerService::~TimerService()
{

}

// Ԥ��ʼ��������ͬ���ġ����໥�����ĳ�ʼ�����Լ�ע��������ϵ
SyncResult TimerService::PreInitialize()
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	tpManager->AddDependency(this, L"V8");

	return SyncResult_Success;
}

// ��ʼ�����������Pending�������һ֡�������ã�Complete��ʾ����
AsyncResult TimerService::Initialize()
{
	mnTimerCount = 0;

	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	mpV8Service = (V8Service *)tpManager->GetServiceByName(L"V8");

	// ע��������jsʹ�õĺ���
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

// �������ͷ���Դ���������Pending�������һ֡�������ã�Complete��ʾ����
AsyncResult TimerService::Finalize()
{
	mpV8Service = nullptr;

	return AsyncResult_Complete;
}

// ֡���£��������Pending���������յ����»ص�
// �������Complete�����Ժ��ٸ������Service���������Failure�����������
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
			// ���ýű��ص�
			Isolate *tpIsolate = mpV8Service->GetIsolate();
			HandleScope tHandleScope(tpIsolate);
			Local<Context> tContext = mpV8Service->GetContext();
			Context::Scope tContextScope(tContext);

			Local<Function> tCallbackFunction = Local<Function>::New(tpIsolate, mTimerArray[i].mCallbackFunction);

			v8::TryCatch try_catch(tpIsolate);

			Local<Value> tResult;
			if (!tCallbackFunction->Call(tContext, tContext->Global(), 0, 0).ToLocal(&tResult))
			{
				// @@�������
				Backtrace::GetInstance().ReportException(tpIsolate, &try_catch);
			}

			// ���Ҫ��ִֻ��һ��
			if (mTimerArray[i].mnLoop == 0)
			{
				mTimerArray[i].mbNeedToDelete = true;
			}
			else
			{
				// ���¼�ʱ
				mTimerArray[i].mfTimer = mTimerArray[i].mfTimeSpan;
			}
		}
	}

	return AsyncResult_Pending;
}

// ��ȡ��ʾ���ƣ����������ⲿ��ȡ��ʾ�����ԡ�Log��
std::wstring TimerService::GetDisplayName()
{
	return L"Timer";
}

// ���һ��Timer
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

// ɾ��һ��Timer
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

// ���һ��Timer
// �ű�ԭ�ͣ�int AddTimer(double timespan, function callback, int vnLoop);
// ����˵����timespan��ʱ����
// ����˵����callback���ű��ص���������
// ����˵����vnLoop��Timer�Ƿ�һֱ��Ч��0��ʾִֻ��һ�Σ�Ȼ��Ͳ��ٵ��ã�1��ʾһֱ����Ч
void TimerService::AddTimer(const FunctionCallbackInfo<Value> &vrArgs)
{
	v8::HandleScope handle_scope(vrArgs.GetIsolate());

	if (vrArgs.Length() != 3)
	{
		// @@ �������
		vrArgs.GetReturnValue().SetUndefined();

		return;
	}

	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	TimerService *tpTimeService = (TimerService *)tpManager->GetServiceByName(L"Timer");

	Local<Function> tCallbackFunction = Local<Function>::Cast(vrArgs[1]);

	int tnTimerID = tpTimeService->AddTimer(vrArgs[0]->NumberValue(), vrArgs.GetIsolate(), tCallbackFunction, vrArgs[2]->Int32Value());

	vrArgs.GetReturnValue().Set(v8::Int32::New(vrArgs.GetIsolate(), tnTimerID));
}

// ɾ��һ��Timer
// �ű�ԭ�ͣ�void KillTimer(int vnTimerID);
void TimerService::KillTimer(const FunctionCallbackInfo<Value> &vrArgs)
{
	v8::HandleScope handle_scope(vrArgs.GetIsolate());

	vrArgs.GetReturnValue().SetUndefined();

	if (vrArgs.Length() != 1)
	{
		// @@ �������
		return;
	}

	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	TimerService *tpTimeService = (TimerService *)tpManager->GetServiceByName(L"Timer");

	tpTimeService->KillTimer(vrArgs[0]->Int32Value());
}