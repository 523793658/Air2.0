// 目前用的是WinSock写的网络通信，应该用跨平台的Sock库来做，否则只能跑在Windows下
#ifdef _V8_Platform_Windows_
	#include <Windows.h>
#endif

#include "V8DebugServer.h"
#include "game_service/Manager/PiGameServiceManager.h"
#include "game_service/Settings/SettingsService.h"
#include "V8Service.h"

#include "pi_lib.h"

using namespace v8;

const int kRecvBuffLen = 4096;


// 构造函数
V8DebugServer::V8DebugServer()
: mbConnect(false)
, mbDebugBlock(false)
{

}

// 析构函数
V8DebugServer::~V8DebugServer()
{

}

// Singleton
V8DebugServer * V8DebugServer::GetInstance()
{
	static V8DebugServer sInstance;
	return &sInstance;
}

// 初始化
bool V8DebugServer::Initialize(v8::Isolate *vpIsolate)
{
	// 初始化WinSocket
	WSADATA tWSAData = { 0 };
	if (WSAStartup(MAKEWORD(2, 2), &tWSAData) != 0)
	{
		return false;
	}

	mpIsolate = vpIsolate;

	mpSettingService = (SettingsService *)PiGameServiceManager::GetInstance()->GetServiceByName(L"Settings");
	mbDebugBlock = mpSettingService->getDebugSetting().enable;

	// 设置响应消息的处理函数

	v8::Debug::SetMessageHandler(vpIsolate, V8DebugServer::DebuggerAgentMessageHandler);
	v8::Debug::SetDebugEventListener(vpIsolate, V8DebugServer::DebugEventCallback);

	mbNetWorkThreadRunning = true;

	// 启动Socket工作线程
	
	mThread = std::thread(&V8DebugServer::DebugThread, this);

	mPackage.SetRecver(this);

	return true;
}

// 更新
void V8DebugServer::Update()
{
	// V8帧更新
	while (mbDebugBlock)
	{
		Sleep(10);
	}
	v8::Debug::ProcessDebugMessages(mpIsolate);

}

// 结束
void V8DebugServer::Finalize()
{
	mbNetWorkThreadRunning = false;

	closesocket(mnSocket);
	// 释放WinSocket
	WSACleanup();
	mThread.join();
}

// v8调式协议基于json，因此可以直接使用string获取
bool V8DebugServer::GetRequest(int vnSocketID)
 {
	int tnLength;
	char tmp[kRecvBuffLen + 1] = { 0 };
	tnLength = recv(vnSocketID, tmp, kRecvBuffLen, 0);

	if (tnLength > 0)
	{
		tmp[tnLength] = '\0';
		mPackage.Push(tmp, tnLength);
	}
	else
	{
// 		int error = WSAGetLastError();
// 		printf("error: %d", error);

		mbConnect = false;
		return false;
	}

	return true;
}

// 以后这些都做统一的封包，该函数改成SendBuffer
void V8DebugServer::SendStringMessage(int vnSocketID, const std::string& vstrContent)
{
	if (vnSocketID > 0)
	{
		// 先发送长度
		int tnLength = vstrContent.size();

		char tmp[1024] = { 0 };
		sprintf_s(tmp, "Content-Length: %d\r\n\r\n", tnLength);
		send(vnSocketID, tmp, strlen(tmp), 0);

		// 再发送内容
		send(vnSocketID, vstrContent.c_str(), tnLength, 0);

	}
}

void V8DebugServer::SendBuffer(int vnSocketID, const std::string& str)
{
	send(vnSocketID, str.c_str(), str.size(), 0);
} 

v8::Isolate * V8DebugServer::GetIsolate()
{
	return mpIsolate;
}

// Copies a C string to a 16-bit string.  Does not check for buffer overflow.
// Does not use the V8 engine to convert strings, so it can be used
// in any thread.  Returns the length of the string.
int AsciiToUtf16(const char* input_buffer, uint16_t* output_buffer) {
	int i;
	for (i = 0; input_buffer[i] != '\0'; ++i) {
		// ASCII does not use chars > 127, but be careful anyway.
		output_buffer[i] = static_cast<unsigned char>(input_buffer[i]);
	}
	output_buffer[i] = 0;
	return i;
}

// 调试线程
void V8DebugServer::DebugThread()
{
	int portno, clilen;
	struct sockaddr_in serv_addr, cli_addr;

	// Create the server socket
	mnSocket = socket(AF_INET, SOCK_STREAM, 0);

	// Listen to connections on port 42000
	::ZeroMemory((char *)&serv_addr, sizeof(serv_addr));
	portno = mpSettingService->getDebugSetting().port;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	int bindResult = bind(mnSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	
	if (bindResult != SOCKET_ERROR)
	{
		listen(mnSocket, 5);
		clilen = sizeof(cli_addr);

		while (mbNetWorkThreadRunning)
		{
			// Wait for debug client to connect.
			mnSocketID = accept(mnSocket, (struct sockaddr *) &cli_addr, &clilen);

			if (mnSocketID == SOCKET_ERROR)
			{
				break;
			}

			char tmpBuf[1024] = { 0 };
			mbConnect = true;
			SendBuffer(mnSocketID, "Type: connect\r\n");
			sprintf_s(tmpBuf, "V8-Version: %s\r\n", v8::V8::GetVersion());
			SendBuffer(mnSocketID, tmpBuf);
			SendBuffer(mnSocketID, "Protocol-Version: 1\r\n");
			sprintf_s(tmpBuf, "Embedding-Host: %s\r\n", "Pi");
			SendBuffer(mnSocketID, tmpBuf);
			SendBuffer(mnSocketID, "Content-Length: 0\r\n");
			SendBuffer(mnSocketID, "\r\n");

			while (mbConnect)
			{
				// 获取客户端发过来的一个请求命令
				bool result = GetRequest(mnSocketID);
				if (!result)
				{
					break;
				}
				else
				{
					mbDebugBlock = false;
				}

			}
		}
	}
}

// v8的调试消息响应处理
void V8DebugServer::DebuggerAgentMessageHandler(const v8::Debug::Message &vrMessage)
{
	HandleScope handlescope(vrMessage.GetIsolate());

	v8::String::Utf8Value tValue(vrMessage.GetJSON());

	// 把反馈发回给客户端
	V8DebugServer* tServer = V8DebugServer::GetInstance();

	tServer->SendStringMessage(tServer->mnSocketID, *tValue);
}

// v8调试事件回调函数
void V8DebugServer::DebugEventCallback(const v8::Debug::EventDetails &vrEvent_details)
{
	return;
}

void V8DebugServer::RecvCallback(const char* data, int length)
{
	// 向v8转达客户端的请求
	uint16_t* buffer = new uint16_t[length + 1];

	const char* disconnectRequestStr = "{\"type\":\"request\",\"command\":\"disconnect\"}";
	const char* result = strstr(data, disconnectRequestStr);
	if (result != NULL)
	{
		mbConnect = false;
	}

	//printf("%s\n", data);

	int dataLen = AsciiToUtf16(data, buffer);

	v8::Debug::SendCommand(mpIsolate, buffer, dataLen);

	delete [] buffer;
}
