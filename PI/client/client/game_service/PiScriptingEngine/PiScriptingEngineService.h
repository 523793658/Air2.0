#ifndef _PiScriptingEngineService_H_
#define _PiScriptingEngineService_H_

#include "game_service/BaseClass/PiGameService.h"

class PiBindings;
class V8Service;
class PiScriptingEngineService : public PiGameService
{
private:

	PiBindings *bindings;
	V8Service *mpV8Service;

public:

	// ���캯��
	PiScriptingEngineService();

	// ��������
	virtual ~PiScriptingEngineService();

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

	// ִ��js�ļ�, ��¶���ײ�ʹ����������js
	void executeJS(const char* path, const char* data);

	// ִ����JS
	void startMainJS();
};


#endif