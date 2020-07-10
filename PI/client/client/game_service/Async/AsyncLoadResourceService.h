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

	// �߳�
	std::thread mThread;

	// ��Ǽ����߳��Ƿ�������
	bool mbThreadRunning;

	// Ҫ������������
	mw_safe_queue<pi::Task*> mLoadTaskQueue;

	// ������ϵ��������
	mw_safe_queue<pi::Task*> mLoadCompletedQueue;

public:

	// ���캯��
	AsyncLoadResourceService();

	// ��������
	virtual ~AsyncLoadResourceService();

	// Ԥ��ʼ��������ͬ���ġ����໥�����ĳ�ʼ�����Լ�ע��������ϵ
	virtual SyncResult PreInitialize();

	// ��ʼ�����������Pending�������һ֡�������ã�Complete��ʾ����
	virtual AsyncResult Initialize();

	// �������ͷ���Դ���������Pending�������һ֡�������ã�Complete��ʾ����
	virtual AsyncResult Finalize();

	// ֡���£��������Pending���������յ����»ص�
	// �������Complete�����Ժ��ٸ������Service���������Failure�����������
	virtual AsyncResult Update(const float vfElaspedTime);

	// ��ȡ��ʾ���ƣ����������ⲿ��ȡ��ʾ�����ԡ�Log��
	virtual std::wstring GetDisplayName();

	// �����̵߳Ĺ��������������̵߳��ã�
	void ThreadWorkFunction();

	// ���һ���첽��������
	void AddTask(pi::Task *vrTaskData);

};

#endif