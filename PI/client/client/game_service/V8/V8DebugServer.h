#ifndef _V8DebugServer_H_
#define _V8DebugServer_H_

#include "v8.h"
#include "libplatform/libplatform.h"
#include "v8-debug.h"

#include <thread>
#include "DebugPackage.h"

class SettingsService;
class V8DebugServer : public IRecver
{
private:

	// �߳�
	std::thread mThread;

	SettingsService* mpSettingService;

	v8::Isolate *mpIsolate;
	
	DebugPackage mPackage;

	bool mbConnect;

	bool mbDebugBlock;

	int mnSocket;

public:

	// @@Ŀǰֻ֧��һ���ͻ������Ӱ�
	int mnSocketID;

	// �����߳��Ƿ���Ҫ����
	bool mbNetWorkThreadRunning;

public:

	// ���캯��
	V8DebugServer();

	// ��������
	~V8DebugServer();

	// Singleton
	static V8DebugServer * GetInstance();

	v8::Isolate * GetIsolate();

	// ��ʼ��
	bool Initialize(v8::Isolate *vpIsolate);

	// ����
	void Update();

	// ����
	void Finalize();

	// ������ֱ����ֲ�����Ĵ��룬��ͷ������淶�ԡ������Ժ���ֲ��֮��Ķ���
	bool GetRequest(int vnSocketID);

	// �Ժ���Щ����ͳһ�ķ�����ú����ĳ�SendBuffer
	void SendStringMessage(int vnSocketID, const std::string& vstrContent);

	// ��Ϊ���﷢�͵����ݶ����ַ���������ֱ��ʹ��string
	void SendBuffer(int vnSocketID, const std::string& str);

	// �����߳�
	void DebugThread();

	// v8�ĵ�����Ϣ��Ӧ����
	static void DebuggerAgentMessageHandler(const v8::Debug::Message &vrMessage);

	// v8�����¼��ص�����
	static void DebugEventCallback(const v8::Debug::EventDetails &vrEvent_details);

	void RecvCallback(const char* data, int length);
};

#endif