#include <windows.h>

#include "application.h"
#include "inputmanager.h"

typedef struct
{
	HANDLE hwnd;
	uint umessage;
	uint wparam;
	uint lparam;
}WindowsEventData;

static int _get_modifier_key(void)
{
	return
		(((GetKeyState(VK_LSHIFT) < 0) ||
		(GetKeyState(VK_RSHIFT) < 0)) ? KEY_MODIFIER_SHIFT : 0) |
		(((GetKeyState(VK_LCONTROL) < 0) ||
		(GetKeyState(VK_RCONTROL) < 0)) ? KEY_MODIFIER_CTRL : 0) |
		(((GetKeyState(VK_LMENU) < 0) ||
		(GetKeyState(VK_RMENU) < 0)) ? KEY_MODIFIER_ALT : 0);
}

void PI_API pi_inputmanager_window_msg_callback(void *user_data, void *event_data)
{
	//uint size, i;
	sint modifier = 0;
	InputManager *mgr = (InputManager *)user_data;
	WindowsEventData *msg_event = (WindowsEventData *)event_data;

	PI_USE_PARAM(msg_event->hwnd);
	switch (msg_event->umessage)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		{
			KeyEvent event;
			modifier = _get_modifier_key();
			event.key = msg_event->wparam;
			event.modifier = modifier;
			pi_inputmanager_notify_event(mgr, PIET_KEYBOARD_KEY_PRESSED, &event);
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		{
			KeyEvent event;
			modifier = _get_modifier_key();
			event.key = msg_event->wparam;
			event.modifier = modifier;
			pi_inputmanager_notify_event(mgr, PIET_KEYBOARD_KEY_RELEASED, &event);
		}
		break;
		// 鼠标消息
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		{
			MouseEvent event;
			modifier = _get_modifier_key();
			event.button = msg_event->wparam;
			event.x = LOWORD(msg_event->lparam);
			event.y = HIWORD(msg_event->lparam);
			event.modifier = modifier;
			pi_inputmanager_notify_event(mgr, PIET_MOUSE_KEY_PRESSED, &event);
		}
		break;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		{
			MouseEvent event;
			modifier = _get_modifier_key();
			event.button = msg_event->wparam;
			event.x = LOWORD(msg_event->lparam);
			event.y = HIWORD(msg_event->lparam);
			event.modifier = modifier;
			pi_inputmanager_notify_event(mgr, PIET_MOUSE_KEY_RELEASED, &event);
		}
		break;
	case WM_MOUSEMOVE:
		{
			MouseEvent event;
			modifier = _get_modifier_key();
			event.button = msg_event->wparam;
			event.x = LOWORD(msg_event->lparam);
			event.y = HIWORD(msg_event->lparam);
			event.modifier = modifier;
			pi_inputmanager_notify_event(mgr, PIET_MOUSE_MOVED, &event);
		}
		break;
	case WM_MOUSEWHEEL:
		// wparam 高16位代表滚轮转动的距离，每一格距离120，正数代表远离用户方向滚动，负数代表超用户方向
		// wparam 低16位代表按键情况
		// lparam 低16位代表x坐标
		// lparam 高16位代表y坐标
		{
			MouseWheelEvent event;
			modifier = _get_modifier_key();
			event.amount = (short)HIWORD(msg_event->wparam) / 120;
			event.button = msg_event->wparam;
			event.x = LOWORD(msg_event->lparam);
			event.y = HIWORD(msg_event->lparam);
			event.modifier = modifier;
			pi_inputmanager_notify_event(mgr, PIET_MOUSE_WHEEL_SCROLLED, &event);
		}
		break;
	default:
		break;
	}
}