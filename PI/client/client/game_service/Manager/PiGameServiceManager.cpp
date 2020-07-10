#include "PiGameServiceManager.h"
#include "main/MWApplication.h"

// ���캯��
PiGameServiceManager::PiGameServiceManager(void)
{

}

// ��������
PiGameServiceManager::~PiGameServiceManager(void)
{

}

PiGameServiceManager * PiGameServiceManager::GetInstance()
{
	static PiGameServiceManager sPiGameServiceManager;
	return &sPiGameServiceManager;
}

// ע��һ����Ϸ����
AsyncResult PiGameServiceManager::RegisterGameService(PiGameService *vpGameService, int Priority)
{
	// �ȿ�����û��֮ǰע��ģ��������������������
	std::wstring tstrName = vpGameService->GetDisplayName();
	auto tpFindDependentCount = mDependentCountMap.find(tstrName);
	if (tpFindDependentCount != mDependentCountMap.end())
	{
		vpGameService->SetDependentCount(tpFindDependentCount->second);
	}

	// ��¼���������
	mServices.push_back(vpGameService);
	mServiceMap.insert(std::make_pair(tstrName, vpGameService));

	// ��һ֡����Ԥ��ʼ��
	vpGameService->SetState(PiGameService::SS_PreInitializing);

	return AsyncResult_Complete;
}

// ע��һ����Ϸ����
void PiGameServiceManager::UnregisterGameService(PiGameService *vpGameService)
{
	if (vpGameService == nullptr)
	{
		return;
	}

	// �������ͷ����̾Ϳ�����
	vpGameService->SetState(PiGameService::SS_Finalizing);
}

// ���һ��������ϵ
void PiGameServiceManager::AddDependency(PiGameService *vpSourceService, const wchar_t *vstrDependency)
{
	std::vector<std::wstring> &tDependencies = vpSourceService->GetDependencies();
	tDependencies.push_back(vstrDependency);

	// ������������Ƿ��Ѿ�ע�����
	auto tpFindService = mServiceMap.find(vstrDependency);
	if (tpFindService != mServiceMap.end())
	{
		tpFindService->second->IncreaseDependentCount();
	}
	else // ��û��ע�����
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

// ��ȡһ����Ϸ����
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

// �������������ܵ���

// Ŀǰû����
bool PiGameServiceManager::Initialize()
{
	return true;
}

// ����
void PiGameServiceManager::Finalize()
{
	// �Ȱ����еķ��񶼽������״̬
	size_t tnCount = mServices.size();
	for (size_t i = 0; i < tnCount; i++)
	{
		mServices[i]->SetState(PiGameService::SS_Finalizing);
	}

	// �������£�ֱ��ȫ�����������
	while (mServices.size() != 0)
	{
		Update(0.016f);
	}
}

// Service ֡����
void PiGameServiceManager::Update(const float vfElaspedTime)
{
	int tnCount = (int)mServices.size();
	for (int i = tnCount - 1; i >= 0; i--)
	{
		PiGameService *tpService = mServices[i];
		switch (tpService->GetState())
		{
			// ��Ҫ����Ԥ��ʼ����
			case PiGameService::SS_PreInitializing:
			{
				// ���Ԥ��ʼ��ʧ��
				if (tpService->PreInitialize() == SyncResult::SyncResult_Failure)
				{
					// @@�������

					// �˳�����
					MWApplication::GetInstance()->QuitApplication(1);
				}
				else
				{
					tpService->SetState(PiGameService::SS_Initializing);
				}

				break;
			}
			// ��Ҫ���г�ʼ��
			case PiGameService::SS_Initializing:
			{
				// ���ж��������ķ���ģ���Ƿ��Ѿ����ڲ����ѳ�ʼ��
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

				// ��������ķ���û��ȫ׼���ã��Ͳ������Լ��ĳ�ʼ��
				if (tbAllChecked == false)
				{
					break;
				}

				switch (tpService->Initialize())
				{
					case AsyncResult::AsyncResult_Failure:
					{
						// @@�������

						// �˳�����
						MWApplication::GetInstance()->QuitApplication(2);

						break;
					}
					case AsyncResult::AsyncResult_Complete: // �ɹ���ʼ����������һ��״̬
					{
						tpService->SetState(PiGameService::SS_Running);

						break;
					}
					case AsyncResult::AsyncResult_Pending: // ����Ҫ��һ����ʼ��
					{
						// �Ͳ��ı�״̬��
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
						// @@�������

						// �˳�����
						MWApplication::GetInstance()->QuitApplication(3);

						break;
					}
					case AsyncResult::AsyncResult_Complete: // ����Ҫ���и���
					{
						tpService->SetState(PiGameService::SS_WaitingToFinalize);

						break;
					}
				}

				break;
			}
			case PiGameService::SS_Finalizing:
			{
				// ���������񻹱����������������ţ��Ͳ����ͷ�
				if (tpService->GetDependentCount() != 0)
				{
					break;
				}
				
				switch (tpService->Finalize())
				{
					case AsyncResult::AsyncResult_Failure:
					{
						// @@�������

						// �˳�����
						MWApplication::GetInstance()->QuitApplication(4);

						break;
					}
					case AsyncResult::AsyncResult_Complete:
					{
						// �������ڽ���
						tpService->SetState(PiGameService::SS_Complete);

						// ��������ϵ
						std::vector<std::wstring> &tDependencies = tpService->GetDependencies();
						size_t tnCount = tDependencies.size();
						for (size_t j = 0; j < tnCount; j++)
						{
							auto tpFindService = mServiceMap.find(tDependencies[j]);
							if (tpFindService != mServiceMap.end())
							{
								// ��������������
								tpFindService->second->DecreaseDependentCount();
							}
							else
							{
								// @@ �����ܽ���ķ�֧����ʾ��������
							}
						}

						// �����ͷŰ�
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