#include <Windows.h>
#include "pi_lib.h"
#include "AsyncLoadResourceService.h"
#include "game_service/PiScriptingEngine/tasks/LoadTask.h"

using pi::Task;
// 构造函数
AsyncLoadResourceService::AsyncLoadResourceService()
{
	mbThreadRunning = false;
}

// 析构函数
AsyncLoadResourceService::~AsyncLoadResourceService()
{
}

// 预初始化，用于同步的、无相互依赖的初始化，以及注册依赖关系
SyncResult AsyncLoadResourceService::PreInitialize()
{
	return SyncResult::SyncResult_Success;
}

// 初始化，如果返回Pending，则会下一帧继续调用，Complete表示结束
AsyncResult AsyncLoadResourceService::Initialize()
{
	// 启动线程
	mbThreadRunning = true;
	mThread = std::thread(&AsyncLoadResourceService::ThreadWorkFunction, this);

	return AsyncResult_Complete;
}

// 结束并释放资源，如果返回Pending，则会下一帧继续调用，Complete表示结束
AsyncResult AsyncLoadResourceService::Finalize()
{
	mbThreadRunning = false;

	// 等待线程结束
	// todo 销毁线程
	mThread.join();

	return AsyncResult_Complete;
}

// 帧更新，如果返回Pending，则会继续收到更新回调
// 如果返回Complete，则以后不再更新这个Service，如果返回Failure，则结束程序
AsyncResult AsyncLoadResourceService::Update(const float vfElaspedTime)
{
	Task* tTask;
	while (mLoadCompletedQueue.pop(tTask))
	{
		tTask->Callback();
		delete tTask;
	}

	return AsyncResult_Pending;
}

// 获取显示名称，可以用来外部获取标示、调试、Log用
std::wstring AsyncLoadResourceService::GetDisplayName()
{
	return L"AsyncLoadResource";
}

// 线程的工作函数（加载线程调用）
void AsyncLoadResourceService::ThreadWorkFunction()
{
	while (mbThreadRunning == true)
	{
		Task* tTask;
		while (mLoadTaskQueue.pop(tTask))
		{
			if (tTask->Run()) {
				mLoadCompletedQueue.push(tTask);
			}
			else {

			}
		}
		Sleep(1);
	}
}

// 添加一个异步加载任务
void AsyncLoadResourceService::AddTask(Task *vrTaskData)
{
	mLoadTaskQueue.push(vrTaskData);
}