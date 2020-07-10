#include "PiGameService.h"

// ���캯��
PiGameService::PiGameService(void)
{
	mDependentCount = 0;
}

// ��������
PiGameService::~PiGameService(void)
{

}

// ����״̬
void PiGameService::SetState(ServiceState vnState)
{
	mnServiceState = vnState;
}

// ��ȡ״̬
PiGameService::ServiceState PiGameService::GetState()
{
	return mnServiceState;
}

// ��ȡ������Service
std::vector<std::wstring> & PiGameService::GetDependencies()
{
	return mDependencies;
}

// ������������
void PiGameService::IncreaseDependentCount()
{
	mDependentCount++;
}

// ������������
void PiGameService::DecreaseDependentCount()
{
	mDependentCount--;
}

// ��ȡ��������
const int PiGameService::GetDependentCount()
{
	return mDependentCount;
}

// ������������
void PiGameService::SetDependentCount(const int vnCount)
{
	mDependentCount = vnCount;
}