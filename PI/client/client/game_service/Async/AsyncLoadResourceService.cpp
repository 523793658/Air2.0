#include <Windows.h>
#include "pi_lib.h"
#include "AsyncLoadResourceService.h"
#include "game_service/PiScriptingEngine/tasks/LoadTask.h"

using pi::Task;
// ���캯��
AsyncLoadResourceService::AsyncLoadResourceService()
{
	mbThreadRunning = false;
}

// ��������
AsyncLoadResourceService::~AsyncLoadResourceService()
{
}

// Ԥ��ʼ��������ͬ���ġ����໥�����ĳ�ʼ�����Լ�ע��������ϵ
SyncResult AsyncLoadResourceService::PreInitialize()
{
	return SyncResult::SyncResult_Success;
}

// ��ʼ�����������Pending�������һ֡�������ã�Complete��ʾ����
AsyncResult AsyncLoadResourceService::Initialize()
{
	// �����߳�
	mbThreadRunning = true;
	mThread = std::thread(&AsyncLoadResourceService::ThreadWorkFunction, this);

	return AsyncResult_Complete;
}

// �������ͷ���Դ���������Pending�������һ֡�������ã�Complete��ʾ����
AsyncResult AsyncLoadResourceService::Finalize()
{
	mbThreadRunning = false;

	// �ȴ��߳̽���
	// todo �����߳�
	mThread.join();

	return AsyncResult_Complete;
}

// ֡���£��������Pending���������յ����»ص�
// �������Complete�����Ժ��ٸ������Service���������Failure�����������
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

// ��ȡ��ʾ���ƣ����������ⲿ��ȡ��ʾ�����ԡ�Log��
std::wstring AsyncLoadResourceService::GetDisplayName()
{
	return L"AsyncLoadResource";
}

// �̵߳Ĺ��������������̵߳��ã�
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

// ���һ���첽��������
void AsyncLoadResourceService::AddTask(Task *vrTaskData)
{
	mLoadTaskQueue.push(vrTaskData);
}