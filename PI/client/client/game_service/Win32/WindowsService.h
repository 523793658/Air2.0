#ifndef _WindowsService_H_
#define _WindowsService_H_

#include "game_service\BaseClass\PiGameService.h"
#include "game_tools\GameTimer.h"
#include "ime.h"

class MoveTitle
{
public:
	static MoveTitle& GetInstance()
	{
		static MoveTitle moveTitle;
		return moveTitle;
	}

	MoveTitle();
	~MoveTitle();

	void update();

	void startMove();

	void endMove();

private:
	bool mLButtonDownFlag;
	POINT mLastMousePos;
	POINT mCurMousePos;
	RECT mCurWindowRect;
};

class WindowsService : public PiGameService
{
private:

	// 程序的实例句柄
	HINSTANCE mhInstance;

	// 游戏的窗口句柄
	HWND mhWnd;

	// 为了准确表达，改这里的变量名，现在其实存的是分辨率尺寸
	// 而不是实际运行的窗口尺寸
	UINT mnResolutionWidth;
	UINT mnResolutionHeight;

	GameTimer* mnTimer;

	CIME imeInstance;

public:

	// 构造函数
	WindowsService();

	// 析构函数
	virtual ~WindowsService();

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

	// 获取游戏程序的窗口句柄
	HWND GetWindowHandle();

	// 创建游戏程序窗口
	bool CreateGameWindow(HINSTANCE vhInstance, const wchar_t *vstrTitle, const int vnWidth, const int vnHeight);

	// 进入消息循环
	void EnterMessageLoop();

	// 处理Windows消息
	static LRESULT CALLBACK WindowsMessageProcess(HWND vhWnd, UINT vnMessage, WPARAM wParam, LPARAM lParam);

	// 设置窗口的标题
	void SetWindowHeader(const wchar_t *vstrHeader);

	// 闪烁窗口
	bool FlashWindowProc(UINT nCount, DWORD time);

	//居中
	void CenterWindow();

	// 设置窗口的尺寸
	void SetWindowSize(UINT vnWidth, UINT vnHeight);

	// 获取窗口的尺寸
	void GetWindowSize(UINT *vpWidth, UINT *vpHeight);

	// 开关全屏
	void WindowsService::ToggleFullScreen(BOOL fullScreen);

	//最大化窗口
	void SetWindowsShow(UINT type);

	//是否启用输入法
	void enableImm(BOOL enableFlag);

	//显示/隐藏窗口
	void ShowMainWindow(BOOL show);

	//关闭窗口
	void destroy();

	// 强制关闭程序
	void Shutdown(int vnExitCode);

	int64 GetMachineTime();
};

#endif