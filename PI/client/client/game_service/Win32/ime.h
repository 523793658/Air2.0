#ifndef _IME_H_
#define _IME_H_

#include <Windows.h>

class CIME
{
public:
	//初始化
	void Init(HWND hWnd);

	//获取输入法
	void Enable();

	//禁用输入法
	void Disable();

private:
	//窗口句柄
	HWND m_hWnd;

	//上下文信息
	HIMC g_himc;

	//启用标记
	bool m_bAble;
};

#endif