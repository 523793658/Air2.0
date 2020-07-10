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

	// �����ʵ�����
	HINSTANCE mhInstance;

	// ��Ϸ�Ĵ��ھ��
	HWND mhWnd;

	// Ϊ��׼ȷ��������ı�������������ʵ����Ƿֱ��ʳߴ�
	// ������ʵ�����еĴ��ڳߴ�
	UINT mnResolutionWidth;
	UINT mnResolutionHeight;

	GameTimer* mnTimer;

	CIME imeInstance;

public:

	// ���캯��
	WindowsService();

	// ��������
	virtual ~WindowsService();

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

	// ��ȡ��Ϸ����Ĵ��ھ��
	HWND GetWindowHandle();

	// ������Ϸ���򴰿�
	bool CreateGameWindow(HINSTANCE vhInstance, const wchar_t *vstrTitle, const int vnWidth, const int vnHeight);

	// ������Ϣѭ��
	void EnterMessageLoop();

	// ����Windows��Ϣ
	static LRESULT CALLBACK WindowsMessageProcess(HWND vhWnd, UINT vnMessage, WPARAM wParam, LPARAM lParam);

	// ���ô��ڵı���
	void SetWindowHeader(const wchar_t *vstrHeader);

	// ��˸����
	bool FlashWindowProc(UINT nCount, DWORD time);

	//����
	void CenterWindow();

	// ���ô��ڵĳߴ�
	void SetWindowSize(UINT vnWidth, UINT vnHeight);

	// ��ȡ���ڵĳߴ�
	void GetWindowSize(UINT *vpWidth, UINT *vpHeight);

	// ����ȫ��
	void WindowsService::ToggleFullScreen(BOOL fullScreen);

	//��󻯴���
	void SetWindowsShow(UINT type);

	//�Ƿ��������뷨
	void enableImm(BOOL enableFlag);

	//��ʾ/���ش���
	void ShowMainWindow(BOOL show);

	//�رմ���
	void destroy();

	// ǿ�ƹرճ���
	void Shutdown(int vnExitCode);

	int64 GetMachineTime();
};

#endif