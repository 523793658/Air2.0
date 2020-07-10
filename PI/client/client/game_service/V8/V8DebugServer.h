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

	// 线程
	std::thread mThread;

	SettingsService* mpSettingService;

	v8::Isolate *mpIsolate;
	
	DebugPackage mPackage;

	bool mbConnect;

	bool mbDebugBlock;

	int mnSocket;

public:

	// @@目前只支持一个客户端连接吧
	int mnSocketID;

	// 网络线程是否需要运行
	bool mbNetWorkThreadRunning;

public:

	// 构造函数
	V8DebugServer();

	// 析构函数
	~V8DebugServer();

	// Singleton
	static V8DebugServer * GetInstance();

	v8::Isolate * GetIsolate();

	// 初始化
	bool Initialize(v8::Isolate *vpIsolate);

	// 更新
	void Update();

	// 结束
	void Finalize();

	// 以下是直接移植过来的代码，回头再整理规范性、合理性和移植性之类的东西
	bool GetRequest(int vnSocketID);

	// 以后这些都做统一的封包，该函数改成SendBuffer
	void SendStringMessage(int vnSocketID, const std::string& vstrContent);

	// 因为这里发送的数据都是字符串，所以直接使用string
	void SendBuffer(int vnSocketID, const std::string& str);

	// 调试线程
	void DebugThread();

	// v8的调试消息响应处理
	static void DebuggerAgentMessageHandler(const v8::Debug::Message &vrMessage);

	// v8调试事件回调函数
	static void DebugEventCallback(const v8::Debug::EventDetails &vrEvent_details);

	void RecvCallback(const char* data, int length);
};

#endif