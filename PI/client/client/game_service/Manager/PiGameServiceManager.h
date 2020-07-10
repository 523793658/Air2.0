#ifndef _PiGameServiceManager_H_
#define _PiGameServiceManager_H_

#include <unordered_map>
#include <string>
#include <vector>
#include "game_service/BaseClass/PiGameService.h"

// ����Game Service
class PiGameServiceManager
{
private:

	std::unordered_map<std::wstring, PiGameService *> mServiceMap;
	std::vector<PiGameService *> mServices;

	// ��ʱ��¼��������
	std::unordered_map<std::wstring, int> mDependentCountMap;

public:

	// ���캯��
	PiGameServiceManager(void);

	// ��������
	~PiGameServiceManager(void);

	static PiGameServiceManager * GetInstance();

	// ע��һ����Ϸ����
	AsyncResult RegisterGameService(PiGameService *vpGameService, int Priority = 1000);

	// ע��һ����Ϸ����
	void UnregisterGameService(PiGameService *vpGameService);

	// ��ȡһ����Ϸ����
	PiGameService * GetServiceByName(wchar_t *vstrName);

	// ���һ��������ϵ
	void AddDependency(PiGameService *vpSourceService, const wchar_t *vstrDependency);

	// �������������ܵ���

	// Ŀǰû����
	bool Initialize();

	// ����
	void Finalize();

	// Service ֡����
	virtual void Update(const float vfElaspedTime);
};

#endif