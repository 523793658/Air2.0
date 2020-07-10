#include "PiGameService.h"

// 构造函数
PiGameService::PiGameService(void)
{
	mDependentCount = 0;
}

// 析构函数
PiGameService::~PiGameService(void)
{

}

// 设置状态
void PiGameService::SetState(ServiceState vnState)
{
	mnServiceState = vnState;
}

// 获取状态
PiGameService::ServiceState PiGameService::GetState()
{
	return mnServiceState;
}

// 获取依赖的Service
std::vector<std::wstring> & PiGameService::GetDependencies()
{
	return mDependencies;
}

// 增加依赖计数
void PiGameService::IncreaseDependentCount()
{
	mDependentCount++;
}

// 减少依赖计数
void PiGameService::DecreaseDependentCount()
{
	mDependentCount--;
}

// 获取依赖计数
const int PiGameService::GetDependentCount()
{
	return mDependentCount;
}

// 设置依赖计数
void PiGameService::SetDependentCount(const int vnCount)
{
	mDependentCount = vnCount;
}