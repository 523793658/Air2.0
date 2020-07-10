#include "PiGameServiceManager.h"
#include "main/MWApplication.h"

// 构造函数
PiGameServiceManager::PiGameServiceManager(void)
{

}

// 析构函数
PiGameServiceManager::~PiGameServiceManager(void)
{

}

PiGameServiceManager * PiGameServiceManager::GetInstance()
{
	static PiGameServiceManager sPiGameServiceManager;
	return &sPiGameServiceManager;
}

// 注册一个游戏服务
AsyncResult PiGameServiceManager::RegisterGameService(PiGameService *vpGameService, int Priority)
{
	// 先看看有没有之前注册的，依赖本服务的其它服务
	std::wstring tstrName = vpGameService->GetDisplayName();
	auto tpFindDependentCount = mDependentCountMap.find(tstrName);
	if (tpFindDependentCount != mDependentCountMap.end())
	{
		vpGameService->SetDependentCount(tpFindDependentCount->second);
	}

	// 记录进服务队列
	mServices.push_back(vpGameService);
	mServiceMap.insert(std::make_pair(tstrName, vpGameService));

	// 下一帧进行预初始化
	vpGameService->SetState(PiGameService::SS_PreInitializing);

	return AsyncResult_Complete;
}

// 注销一个游戏服务
void PiGameServiceManager::UnregisterGameService(PiGameService *vpGameService)
{
	if (vpGameService == nullptr)
	{
		return;
	}

	// 让它走释放流程就可以了
	vpGameService->SetState(PiGameService::SS_Finalizing);
}

// 添加一个依赖关系
void PiGameServiceManager::AddDependency(PiGameService *vpSourceService, const wchar_t *vstrDependency)
{
	std::vector<std::wstring> &tDependencies = vpSourceService->GetDependencies();
	tDependencies.push_back(vstrDependency);

	// 找找这个服务是否已经注册进来
	auto tpFindService = mServiceMap.find(vstrDependency);
	if (tpFindService != mServiceMap.end())
	{
		tpFindService->second->IncreaseDependentCount();
	}
	else // 还没有注册进来
	{
		auto tpFindDependentCount = mDependentCountMap.find(vstrDependency);
		if (tpFindDependentCount != mDependentCountMap.end())
		{
			tpFindDependentCount->second++;
		}
		else
		{
			mDependentCountMap.insert(std::make_pair(vstrDependency, 1));
		}
	}
}

// 获取一个游戏服务
PiGameService * PiGameServiceManager::GetServiceByName(wchar_t *vstrName)
{
	auto tpFindService = mServiceMap.find(vstrName);
	if (tpFindService != mServiceMap.end())
	{
		return tpFindService->second;
	}
	else
	{
		return nullptr;
	}
}

// 以下由主程序框架调用

// 目前没有用
bool PiGameServiceManager::Initialize()
{
	return true;
}

// 结束
void PiGameServiceManager::Finalize()
{
	// 先把所有的服务都进入结束状态
	size_t tnCount = mServices.size();
	for (size_t i = 0; i < tnCount; i++)
	{
		mServices[i]->SetState(PiGameService::SS_Finalizing);
	}

	// 继续更新，直到全部服务处理完毕
	while (mServices.size() != 0)
	{
		Update(0.016f);
	}
}

// Service 帧更新
void PiGameServiceManager::Update(const float vfElaspedTime)
{
	int tnCount = (int)mServices.size();
	for (int i = tnCount - 1; i >= 0; i--)
	{
		PiGameService *tpService = mServices[i];
		switch (tpService->GetState())
		{
			// 需要进行预初始化了
			case PiGameService::SS_PreInitializing:
			{
				// 如果预初始化失败
				if (tpService->PreInitialize() == SyncResult::SyncResult_Failure)
				{
					// @@输出错误

					// 退出程序
					MWApplication::GetInstance()->QuitApplication(1);
				}
				else
				{
					tpService->SetState(PiGameService::SS_Initializing);
				}

				break;
			}
			// 需要进行初始化
			case PiGameService::SS_Initializing:
			{
				// 先判断其依赖的服务模块是否已经存在并都已初始化
				bool tbAllChecked = true;
				std::vector<std::wstring> &tDependencies = tpService->GetDependencies();
				size_t tnCount = tDependencies.size();
				for (size_t j = 0; j < tnCount; j++)
				{
					auto tpFindService = mServiceMap.find(tDependencies[j]);
					if (tpFindService != mServiceMap.end())
					{
						switch (tpFindService->second->GetState())
						{
							case PiGameService::SS_Running:
							case PiGameService::SS_WaitingToFinalize:
							{
								break;
							}
							default:
							{
								tbAllChecked = false;
							}
						}
					}
					else
					{
						tbAllChecked = false;
					}
				}

				// 如果依赖的服务并没有全准备好，就不能走自己的初始化
				if (tbAllChecked == false)
				{
					break;
				}

				switch (tpService->Initialize())
				{
					case AsyncResult::AsyncResult_Failure:
					{
						// @@输出错误

						// 退出程序
						MWApplication::GetInstance()->QuitApplication(2);

						break;
					}
					case AsyncResult::AsyncResult_Complete: // 成功初始化，进入下一个状态
					{
						tpService->SetState(PiGameService::SS_Running);

						break;
					}
					case AsyncResult::AsyncResult_Pending: // 还需要进一步初始化
					{
						// 就不改变状态了
						break;
					}
				}

				break;
			}
			case PiGameService::SS_Running:
			{
				switch (tpService->Update(vfElaspedTime))
				{
					case AsyncResult::AsyncResult_Failure:
					{
						// @@输出错误

						// 退出程序
						MWApplication::GetInstance()->QuitApplication(3);

						break;
					}
					case AsyncResult::AsyncResult_Complete: // 不需要进行更新
					{
						tpService->SetState(PiGameService::SS_WaitingToFinalize);

						break;
					}
				}

				break;
			}
			case PiGameService::SS_Finalizing:
			{
				// 如果这个服务还被其它服务所依赖着，就不能释放
				if (tpService->GetDependentCount() != 0)
				{
					break;
				}
				
				switch (tpService->Finalize())
				{
					case AsyncResult::AsyncResult_Failure:
					{
						// @@输出错误

						// 退出程序
						MWApplication::GetInstance()->QuitApplication(4);

						break;
					}
					case AsyncResult::AsyncResult_Complete:
					{
						// 生命周期结束
						tpService->SetState(PiGameService::SS_Complete);

						// 找依赖关系
						std::vector<std::wstring> &tDependencies = tpService->GetDependencies();
						size_t tnCount = tDependencies.size();
						for (size_t j = 0; j < tnCount; j++)
						{
							auto tpFindService = mServiceMap.find(tDependencies[j]);
							if (tpFindService != mServiceMap.end())
							{
								// 减少其依赖计数
								tpFindService->second->DecreaseDependentCount();
							}
							else
							{
								// @@ 不可能进入的分支，提示错误或断言
							}
						}

						// 彻底释放吧
						mServiceMap.erase(mServiceMap.find(tpService->GetDisplayName()));
						delete tpService;
						mServices.erase(mServices.begin() + i);

						break;
					}
				}

				break;
			}
		}
	}
}