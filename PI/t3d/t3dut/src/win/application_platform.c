#include "rendersystem.h"
#include "application.h"

#include "inputmanager.h"

#include <windows.h>

typedef struct
{
	HANDLE hwnd;
	uint umessage;
	uint wparam;
	uint lparam;
}WindowsEventData;

typedef struct
{	
	uint win_pos_x;
	uint win_pos_y;
	uint sleep;
	PiBool is_init;
	PiBool is_exit;
	PiBool is_running;
	HWND win_handle;
}WindowsApplication;

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

void PI_API pi_native_input_callback(void *user_data, void *event_data)
{
	uint i, size;
	sint modifier = 0;
	InputManager *mgr = (InputManager *)user_data;
	WindowsEventData *event = (WindowsEventData *)event_data;

	PI_USE_PARAM(event->hwnd);
	switch (event->umessage)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		modifier = _get_modifier_key();
		size = pi_vector_size(mgr->keyboard_listeners);
		for (i = 0; i < size; i++) {
			KeyboardListener* listener = (KeyboardListener*)pi_vector_get(mgr->keyboard_listeners, i);
			if (listener->key_pressed != NULL) {
				listener->key_pressed(listener->user_data, event->wparam, modifier);
			}
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		modifier = _get_modifier_key();
		size = pi_vector_size(mgr->keyboard_listeners);
		for (i = 0; i < size; i++) {
			KeyboardListener* listener = (KeyboardListener*)pi_vector_get(mgr->keyboard_listeners, i);
			if (listener->key_released != NULL) {
				listener->key_released(listener->user_data, event->wparam, modifier);
			}
		}
		break;
		// 鼠标消息
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		modifier = _get_modifier_key();
		size = pi_vector_size(mgr->mouse_key_listeners);
		for (i = 0; i < size; i++) {
			MouseKeyListener* listener = (MouseKeyListener*)pi_vector_get(mgr->mouse_key_listeners, i);
			if (listener->key_pressed != NULL) {
				listener->key_pressed(listener->user_data, event->wparam, LOWORD(event->lparam), HIWORD(event->lparam), modifier);
			}
		}
		break;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		modifier = _get_modifier_key();
		size = pi_vector_size(mgr->mouse_key_listeners);
		for (i = 0; i < size; i++) {
			MouseKeyListener* listener = (MouseKeyListener*)pi_vector_get(mgr->mouse_key_listeners, i);
			if (listener->key_released != NULL) {
				listener->key_released(listener->user_data, event->wparam, LOWORD(event->lparam), HIWORD(event->lparam), modifier);
			}
		}
		break;
	case WM_MOUSEMOVE:
		modifier = _get_modifier_key();
		size = pi_vector_size(mgr->mouse_motion_listeners);
		for (i = 0; i < size; i++) {
			MouseMotionListener* listener = (MouseMotionListener*)pi_vector_get(mgr->mouse_motion_listeners, i);
			if (listener->mouse_moved != NULL) {
				listener->mouse_moved(listener->user_data, event->wparam, LOWORD(event->lparam), HIWORD(event->lparam), modifier);
			}
		}
		break;
	case WM_MOUSEWHEEL:
		// wparam 高16位代表滚轮转动的距离，每一格距离120，正数代表远离用户方向滚动，负数代表超用户方向
		// wparam 低16位代表按键情况
		// lparam 低16位代表x坐标
		// lparam 高16位代表y坐标
		modifier = _get_modifier_key();
		size = pi_vector_size(mgr->mouse_wheel_listeners);
		for (i = 0; i < size; i++) {
			MouseWheelListener* listener = (MouseWheelListener*)pi_vector_get(mgr->mouse_wheel_listeners, i);
			if (listener->wheel_scrolled != NULL) {
				listener->wheel_scrolled(listener->user_data, (short)HIWORD(event->wparam) / 120, event->wparam, LOWORD(event->lparam), HIWORD(event->lparam), modifier);
			}
		}
		break;
	default:
		break;
	}
}

static void _load_shader(wchar *key, wchar *path)
{
	int64 size;
	byte *data;
	void *f = pi_file_open(path, FILE_OPEN_READ);
	pi_file_size(f, &size);
	data = (byte *)pi_malloc((uint)size);
	pi_file_read(f, 0, FALSE, (char *)data, (uint)size);
	pi_file_close(f);
	
	pi_rendersystem_add_shader_source(key, data, (uint)size);
	pi_free(data);
}

/* 扫描目录，对所有文件进行加载 */
static void _load_shader_path(wchar *shader_dir)
{
	PiFileNameInfo info;
	wchar path[NAME_LENGTH];
	void *h = pi_file_dir_open(shader_dir);
	while(pi_file_dir_read(h, &info))
	{
		if(info.info.type == FILE_TYPE_REGULAR)
		{
			pi_wstrcpy(path, shader_dir, pi_wstrlen(shader_dir));
			pi_wstr_cat(path, NAME_LENGTH, L"/");
			pi_wstr_cat(path, NAME_LENGTH, info.name);

			_load_shader(info.name, path);
		}
	}
	pi_file_dir_close(h);
}

LRESULT CALLBACK _WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) 
{
	uint size, i;
	PiApplication *app = NULL;
	WindowsApplication *impl = NULL;

	app = (PiApplication *)GetWindowLong(hwnd, GWL_USERDATA);

	if (app)
	{
		impl = (WindowsApplication *)app->impl;
	}

	switch(umessage)
	{
	case WM_PAINT:
	{
		pi_rendersystem_swapbuffer();
	}	
	break;
	case WM_SIZE:
		if(app != NULL)
		{
			app->width = LOWORD(lparam);
			app->height = HIWORD(lparam);
			if(impl->is_init && app->resize_func)
			{
				app->resize_func(app, app->user_data, app->width, app->height);
			}
		}	
		break;
	case WM_CLOSE:
		PostQuitMessage(0);		
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		if(app != NULL) {
			size = pi_vector_size(app->msg_listeners);
			for (i = 0; i < size; i++)
			{
				WindowsEventData data;
				WinMsgListener* listener = (WinMsgListener*)pi_vector_get(app->msg_listeners, i);
				data.hwnd = hwnd;
				data.umessage = umessage;
				data.wparam = wparam;
				data.lparam = lparam;
				listener->callback(listener->wm_user_data, &data);
			}
		}
		break;
	}
	switch (umessage)
	{
	case WM_ERASEBKGND:

	{
		int x = DefWindowProc(hwnd, umessage, wparam, lparam);
		x = x;
		return 0;
	}
		
	default:
		return DefWindowProc(hwnd, umessage, wparam, lparam);
	}
}

static HWND _create_window(PiApplication* app, WindowsApplication *impl)
{
	HWND hwnd;
	WNDCLASSEX wc;

	// 得到应用程序实例句柄
	HINSTANCE hinstance = GetModuleHandle(NULL);

	// 得到windows桌面分辨率
	uint screen_width  = GetSystemMetrics(SM_CXSCREEN);
	uint screen_height = GetSystemMetrics(SM_CYSCREEN);

	// 设置窗口类参数.
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc   = _WndProc; 
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hinstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm       = wc.hIcon;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = app->title;
	wc.cbSize        = sizeof(WNDCLASSEX);

	// 注册窗口类
	RegisterClassEx(&wc);

	// 窗口位置,posX, posY窗口左上角坐标
	impl->win_pos_x = (GetSystemMetrics(SM_CXSCREEN) - screen_width)  / 2;
	impl->win_pos_y = (GetSystemMetrics(SM_CYSCREEN) - screen_height) / 2;
	hwnd = CreateWindowEx(WS_EX_APPWINDOW, 
		app->title, app->title, WS_OVERLAPPEDWINDOW, impl->win_pos_x, impl->win_pos_y, app->width, app->height, NULL, NULL, hinstance, NULL);

	// 设置用户数据
	SetWindowLong(hwnd, GWL_USERDATA, (LONG)app);

	return hwnd;
}

static void _window_set_visible(PiApplication* app, WindowsApplication *impl)
{
	RECT rect; 

	// 显示窗口并设置其为焦点.
	rect.left = rect.top = 0;
	rect.right = app->width;
	rect.bottom = app->height;
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);
	MoveWindow(impl->win_handle, impl->win_pos_x, impl->win_pos_y, rect.right - rect.left, rect.bottom - rect.top, FALSE);

	ShowWindow(impl->win_handle, SW_SHOW);
	SetForegroundWindow(impl->win_handle);
	SetFocus(impl->win_handle);
}

static void _destroy(PiApplication* app, WindowsApplication *impl)
{
	HINSTANCE hinstance = GetModuleHandle(NULL);

	if(app->shutdown_func != NULL)
	{
		app->shutdown_func(app, app->user_data);
	}

	pi_rendersystem_clear();
	
	pi_free(app->shader_dir);

	DestroyWindow(impl->win_handle);
	UnregisterClass(app->title, hinstance);

	pi_vector_free(app->msg_listeners);
	pi_free(impl);
	pi_free(app);
}

void* PI_API windows_application_create(struct PiApplication *app)
{
	WindowsApplication *impl = pi_new0(WindowsApplication, 1);
	impl->is_exit = FALSE;
	impl->is_init = FALSE;
	impl->is_running = FALSE;

	return impl;
}

void PI_API windows_application_destroy(struct PiApplication *app, WindowsApplication* impl)
{
	impl->is_exit =TRUE;
}

void PI_API windows_application_run(struct PiApplication *app, WindowsApplication* impl)
{
	MSG msg;
	PiBool run = TRUE;
	int64 curr_time, last_time;
	double tpf;	/* 每帧的渲染间隔时间，单位：秒 */

	RenderContextLoadType type = CONTEXT_LOAD_NATIVE;

	if (!impl->is_init)
	{
		impl->is_running = TRUE;
		impl->win_handle = _create_window(app, impl);
		
		app->hwnd = (void *)impl->win_handle;

		_window_set_visible(app, impl);
		app->win_rt = pi_rendersystem_init(type, app->width, app->height, impl->win_handle, NULL);

		if (app->shader_dir != NULL)
		{
			_load_shader_path(app->shader_dir);
		}

		// 初始化回调
		if (app->init_func)
			app->init_func(app, app->user_data);

		impl->is_init = TRUE;

		// 重置大小回调
		if (app->resize_func)
			app->resize_func(app, app->user_data, app->width, app->height);

		// 初始化消息结构.
		ZeroMemory(&msg, sizeof(MSG));

		timeBeginPeriod(1);

		impl->is_exit = FALSE;
		app->time = 0;
		last_time = pi_time_now();

		while (run)
		{
			if (impl->is_running)
			{
				// 处理windows消息.
				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

				// 渲染回调
				if (app->update_func)
				{
					curr_time = pi_time_now();
					tpf = (double)(curr_time - last_time);
					tpf /= 1000.0 * 1000.0;
					app->time += tpf;
					last_time = curr_time;

					pi_rendersystem_set_frame_time((float)app->time);

					app->update_func(app, app->user_data, (float)tpf);
				}

				run = (!impl->is_exit && msg.message != WM_QUIT);

				if (impl->sleep > 0)
				{
					Sleep(impl->sleep);
				}
			}
			else
			{
				Sleep(500);
			}
		}

		_destroy(app, impl);
	}
	else
	{
		impl->is_running = TRUE;
	}
}

void PI_API windows_application_stop(struct PiApplication *app, WindowsApplication* impl)
{
	impl->is_running = FALSE;
}

void PI_API pi_windows_application_set_sleep(PiApplication *app, uint sleep)
{
	WindowsApplication *impl = (WindowsApplication *)app->impl;

	impl->sleep = sleep;
}