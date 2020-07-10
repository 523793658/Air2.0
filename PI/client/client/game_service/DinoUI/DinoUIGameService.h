#ifndef _DinoUIGameService_H_
#define _DinoUIGameService_H_
#include "game_service/BaseClass/PiGameService.h"

class DinoUIGameService : public PiGameService
{
private:

	DinoUIEngine *mpUIEngine;

	// 保存一下Windows的窗口句柄
	HWND mhWnd;

	TCHAR *mpClipboardBuffer;

public:

	bool mbInitialized;

	// 构造函数
	DinoUIGameService();

	// 析构函数
	virtual ~DinoUIGameService();

	// 预初始化，用于同步的、无相互依赖的初始化，以及注册依赖关系
	virtual SyncResult PreInitialize();

	// 初始化，如果返回Pending，则会下一帧继续调用，Complete表示结束
	virtual AsyncResult Initialize();

	// 结束并释放资源，如果返回Pending，则会下一帧继续调用，Complete表示结束
	virtual AsyncResult Finalize();

	// 帧更新，如果返回Pending，则会继续收到更新回调
	// 如果返回Complete，则以后不再更新这个Service，如果返回Failure，则结束程序
	virtual AsyncResult Update(const float vfElaspedTime);

	// 获取显示名称，可以用来外部获取标示、调试、Log用
	virtual std::wstring GetDisplayName();

	// 为了等待t3d渲染器完全初始化完成，真正的UI初始化搬到这里来做
	static void DoRealInitialize(const v8::FunctionCallbackInfo<v8::Value> &vrArgs);

	// 对外部输出UI内部的输出信息
	void OutputUIInformation(DinoUICStr vstrInformation);

	// 复制代理
	void CopyToClipboard(DinoUICStr vstrContent);

	// 粘贴代理
	DinoUICStr GetClipBoardContent(void);
};

#endif