#ifndef _IME_H_
#define _IME_H_

#include <Windows.h>

class CIME
{
public:
	//��ʼ��
	void Init(HWND hWnd);

	//��ȡ���뷨
	void Enable();

	//�������뷨
	void Disable();

private:
	//���ھ��
	HWND m_hWnd;

	//��������Ϣ
	HIMC g_himc;

	//���ñ��
	bool m_bAble;
};

#endif