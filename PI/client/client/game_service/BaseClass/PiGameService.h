#ifndef _PiGameService_H_
#define _PiGameService_H_

#include <string>
#include <vector>

enum SyncResult
{
	SyncResult_Success,
	SyncResult_Failure
};

enum AsyncResult
{
	AsyncResult_Complete,
	AsyncResult_Pending,
	AsyncResult_Failure
};

// Game Service的基类
// 比如t3d、游戏逻辑、脚本系统、UI，跑在时间循环中的任何功能部分，都可以视为一个Game Service
// 每个具体的GameService，就派生一个子类，然后自己实现自己的时间更新需求就可以了。
class PiGameService
{
public:

	enum ServiceState
	{
		SS_None, 
		SS_PreInitializing, 
		SS_Initializing, 
		SS_Running, 
		SS_WaitingToFinalize, 
		SS_Finalizing, 
		SS_Complete
	};

protected:

	// 状态，由Manager使用和管理，Service子类不要动
	ServiceState mnServiceState;

	// 该Service所依赖的别的Service的名称
	std::vector<std::wstring> mDependencies;

	// 该服务被多少个其它服务依赖着
	int mDependentCount;

public:

	// 构造函数
	PiGameService(void);

	// 析构函数
	virtual ~PiGameService(void);

	// 获取状态
	ServiceState GetState();

	// 设置状态
	void SetState(ServiceState vnState);

	// 获取依赖的Service
	std::vector<std::wstring> & GetDependencies();

	// 增加依赖计数
	void IncreaseDependentCount();

	// 减少依赖计数
	void DecreaseDependentCount();

	// 获取依赖计数
	const int GetDependentCount();

	// 设置依赖计数
	void SetDependentCount(const int vnCount);

	// 预初始化，用于同步的、无相互依赖的初始化，以及注册依赖关系
	virtual SyncResult PreInitialize() = 0;

	// 初始化，如果返回Pending，则会下一帧继续调用，Complete表示结束
	virtual AsyncResult Initialize() = 0;

	// 结束并释放资源，如果返回Pending，则会下一帧继续调用，Complete表示结束
	virtual AsyncResult Finalize() = 0;

	// 帧更新，如果返回Pending，则会继续收到更新回调
	// 如果返回Complete，则以后不再更新这个Service，如果返回Failure，则结束程序
	virtual AsyncResult Update(const float vfElaspedTime) = 0;

	// 获取显示名称，可以用来外部获取标示、调试、Log用
	virtual std::wstring GetDisplayName() = 0;
};

#endif