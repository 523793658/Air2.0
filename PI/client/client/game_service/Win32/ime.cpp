#include "ime.h"
#include <imm.h>

#define SM_IMMENABLED           82

void CIME::Init(HWND hWnd)
{
	m_hWnd = hWnd;
	m_bAble = true;
	g_himc = NULL;
	if (GetSystemMetrics(SM_IMMENABLED))
	{
		g_himc = ImmGetContext(m_hWnd);
	}
}

void CIME::Enable()
{
	if (g_himc && !m_bAble)
	{
		ImmAssociateContext(m_hWnd, g_himc);
		m_bAble = true;
	}
}

void CIME::Disable()
{
	if (g_himc && m_bAble)
	{
		ImmAssociateContext(m_hWnd, NULL);
		m_bAble = false;
	}

}