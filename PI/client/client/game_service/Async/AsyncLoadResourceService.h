#ifndef _AsyncLoadResourceService_H_
#define _AsyncLoadResourceService_H_

#include <functional>
#include <unordered_map>
#include <string>
#include <thread>

#include "game_tools/GameTools.h"
#include "game_service/BaseClass/PiGameService.h"
#include "pi_safe_queue.h"
#include "game_service/BaseClass/Task.h"

class AsyncLoadResourceService : public PiGameService
{

private:

	// 线程
	std::thread mThread;

	// 标记加载线程是否在运行
	bool mbThreadRunning;

	// 要处理的任务队列
	mw_safe_queue<pi::Task*> mLoadTaskQueue;

	// 加载完毕的任务队列
	mw_safe_queue<pi::Task*> mLoadCompletedQueue;

public:

	// 构造函数
	AsyncLoadResourceService();

	// 析构函数
	virtual ~AsyncLoadResourceService();

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

	// 加载线程的工作函数（加载线程调用）
	void ThreadWorkFunction();

	// 添加一个异步加载任务
	void AddTask(pi::Task *vrTaskData);

};

#endif