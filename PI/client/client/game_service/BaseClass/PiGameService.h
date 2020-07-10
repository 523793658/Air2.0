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

// Game Service�Ļ���
// ����t3d����Ϸ�߼����ű�ϵͳ��UI������ʱ��ѭ���е��κι��ܲ��֣���������Ϊһ��Game Service
// ÿ�������GameService��������һ�����࣬Ȼ���Լ�ʵ���Լ���ʱ���������Ϳ����ˡ�
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

	// ״̬����Managerʹ�ú͹���Service���಻Ҫ��
	ServiceState mnServiceState;

	// ��Service�������ı��Service������
	std::vector<std::wstring> mDependencies;

	// �÷��񱻶��ٸ���������������
	int mDependentCount;

public:

	// ���캯��
	PiGameService(void);

	// ��������
	virtual ~PiGameService(void);

	// ��ȡ״̬
	ServiceState GetState();

	// ����״̬
	void SetState(ServiceState vnState);

	// ��ȡ������Service
	std::vector<std::wstring> & GetDependencies();

	// ������������
	void IncreaseDependentCount();

	// ������������
	void DecreaseDependentCount();

	// ��ȡ��������
	const int GetDependentCount();

	// ������������
	void SetDependentCount(const int vnCount);

	// Ԥ��ʼ��������ͬ���ġ����໥�����ĳ�ʼ�����Լ�ע��������ϵ
	virtual SyncResult PreInitialize() = 0;

	// ��ʼ�����������Pending�������һ֡�������ã�Complete��ʾ����
	virtual AsyncResult Initialize() = 0;

	// �������ͷ���Դ���������Pending�������һ֡�������ã�Complete��ʾ����
	virtual AsyncResult Finalize() = 0;

	// ֡���£��������Pending���������յ����»ص�
	// �������Complete�����Ժ��ٸ������Service���������Failure�����������
	virtual AsyncResult Update(const float vfElaspedTime) = 0;

	// ��ȡ��ʾ���ƣ����������ⲿ��ȡ��ʾ�����ԡ�Log��
	virtual std::wstring GetDisplayName() = 0;
};

#endif