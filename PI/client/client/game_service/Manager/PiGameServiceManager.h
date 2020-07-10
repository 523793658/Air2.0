#ifndef _PiGameServiceManager_H_
#define _PiGameServiceManager_H_

#include <unordered_map>
#include <string>
#include <vector>
#include "game_service/BaseClass/PiGameService.h"

// 管理Game Service
class PiGameServiceManager
{
private:

	std::unordered_map<std::wstring, PiGameService *> mServiceMap;
	std::vector<PiGameService *> mServices;

	// 临时记录依赖计数
	std::unordered_map<std::wstring, int> mDependentCountMap;

public:

	// 构造函数
	PiGameServiceManager(void);

	// 析构函数
	~PiGameServiceManager(void);

	static PiGameServiceManager * GetInstance();

	// 注册一个游戏服务
	AsyncResult RegisterGameService(PiGameService *vpGameService, int Priority = 1000);

	// 注销一个游戏服务
	void UnregisterGameService(PiGameService *vpGameService);

	// 获取一个游戏服务
	PiGameService * GetServiceByName(wchar_t *vstrName);

	// 添加一个依赖关系
	void AddDependency(PiGameService *vpSourceService, const wchar_t *vstrDependency);

	// 以下由主程序框架调用

	// 目前没有用
	bool Initialize();

	// 结束
	void Finalize();

	// Service 帧更新
	virtual void Update(const float vfElaspedTime);
};

#endif