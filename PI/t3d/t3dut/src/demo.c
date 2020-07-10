
#include "demo.h"
#include "load.h"
#include "key_define.h"

#include <windows.h>

void _pi_demo_default_init_func(PiDemo *demo, void *user_data) {}
void _pi_demo_default_resize_func(PiDemo *demo, void *user_data, uint width, uint height) {}
void _pi_demo_default_update_func(PiDemo *demo, void *user_data, float tpf) {}
void _pi_demo_default_shut_down_func(PiDemo *demo, void *user_data) {}

static void _toggle_fullscreen(PiDemo *demo)
{
	HWND hwnd = (HWND)(demo->app->hwnd);
	uint w = demo->width;
	uint h = demo->height;

	demo->is_window_mode = !demo->is_window_mode;
	
	if (demo->is_window_mode)
	{
		RECT rect;

		ChangeDisplaySettings(NULL, 0);
		
		rect.left = 0;
		rect.top = 0;
		rect.right = w;
		rect.bottom = h;
		AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_LEFT);

		SetWindowLongPtr(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		SetWindowPos(hwnd, HWND_TOP, 0, 0, w, h, SWP_NOZORDER | SWP_SHOWWINDOW);
	}
	else
	{
		DEVMODE devMode;

		w = 1920;
		h = 1080;

		devMode.dmSize = sizeof(DEVMODE);
		devMode.dmDriverExtra = 0;
		devMode.dmBitsPerPel = 32;
		devMode.dmPelsWidth = w;
		devMode.dmPelsHeight = h;
		devMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
		
		SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP);
		SetWindowPos(hwnd, HWND_TOP, 0, 0, w, h, SWP_NOZORDER | SWP_SHOWWINDOW);
	}

	pi_rendersystem_reset(demo->is_window_mode, w, h);
}

static void _on_key_event(void *user_data, uint key, uint modifier)
{
	PiDemo *demo = (PiDemo *)user_data;
	
	// ALT + Enter 切换全屏 
	if (key == KEY_RETURN && modifier == KEY_MBUTTON)
	{
		if (demo->app->hwnd != NULL)
			_toggle_fullscreen(demo);
	}
	if (key == KEY_R)
	{
		pi_camera_set_location(demo->screen_cam,  0.0f, 0.0f, 0.0f);
		pi_camera_set_direction(demo->screen_cam, 0.0f, 0.0f, -1.0f);
		pi_camera_set_up(demo->screen_cam, 0.0f, 1.0f, 0.0f);
	}
	if (key == KEY_ESCAPE && demo->esc_exit)
	{
		pi_application_destroy(demo->app);
	}
}

static void _init_camera(PiDemo *demo)
{
	uint width = demo->app->win_rt->width;
	uint height = demo->app->win_rt->height;
	demo->screen_cam = pi_camera_new();
	pi_camera_set_perspective(demo->screen_cam, 45.0f, width / (float)height, 0.1f, 10000.0f);
	//pi_camera_set_frustum(data->cam, -102.4f, 102.4f, -76.8f, 76.8f, -1000.0f, 1000.0f, TRUE);
	pi_camera_set_location(demo->screen_cam,  0.0f, 60.0f, 300.0f);
	pi_camera_set_direction(demo->screen_cam, 0.0f, 0.0f, -1.0f);
	pi_camera_set_up(demo->screen_cam, 0.0f, 1.0f, 0.0f);

	demo->fly_cam = pi_fly_camera_new(demo->screen_cam, demo->input_mgr);
	pi_fly_camera_set_move_speed(demo->fly_cam, 50.0f);
	pi_fly_camera_set_zoom_speed(demo->fly_cam, 5.0f);
}

static void PI_API _init(PiApplication *app, void *user_data)
{
	PiDemo *demo = (PiDemo *)user_data;

	demo->input_mgr = pi_inputmanager_create(app);

	_init_camera(demo);
	demo->pipeline = pi_renderpipeline_ext_new();
	demo->screen_rt = app->win_rt;

	demo->reset_cam_listener.user_data = demo;
	demo->reset_cam_listener.key_pressed = _on_key_event;

	if (demo->init_func)
	{
		demo->init_func(demo, demo->user_data);
	}

	pi_inputmanager_regist_keyboard_listener(demo->input_mgr, &demo->reset_cam_listener);
}

static void PI_API _update(PiApplication *app, void *user_data, float tpf)
{
	//PiColor clr;
	//uint flags = TBM_COLOR | TBM_DEPTH | TBM_STENCIL;
	PiDemo *demo = (PiDemo *)user_data;

	PI_USE_PARAM(app);

	// 每帧渲染前做一次设备丢失检查: 如果可以，reset设备
	if (pi_rendersystem_check() != CHECK_LOST)
	{
		pi_fly_camera_update(demo->fly_cam, tpf);

		//color_set(&clr, 0.0f, 0.0f, 0.0f, 1.0f);
		//pi_rendersystem_set_target(demo->app->win_rt);
		//pi_rendersystem_clearview(flags, &clr, 1.0f, 0);

		if (demo->update_func)
		{
			demo->update_func(demo, demo->user_data, tpf);
		}
		
		pi_renderpipeline_ext_draw(demo->pipeline, tpf);

		pi_rendersystem_swapbuffer();
	}
}

static void PI_API _resize(PiApplication *app, void *user_data, uint width, uint height)
{
	PiDemo *demo = (PiDemo *)user_data;
	PI_USE_PARAM(app);

	demo->width = width;
	demo->height = height;

	// 重置渲染设备
	pi_rendersystem_reset(demo->is_window_mode, width, height);

	pi_renderpipeline_ext_resize(demo->pipeline, width, height);

	pi_rendertarget_set_viewport(demo->screen_rt, 0, 0, width, height);
	
	pi_camera_resize(demo->screen_cam, (float)width / (float)height);
	
	if (demo->resize_func)
	{
		demo->resize_func(demo, demo->user_data, width, height);
	}
}

static void PI_API _shutdown(PiApplication *app, void *user_data)
{
	PiDemo *demo = (PiDemo *)user_data;
	PI_USE_PARAM(app);
	if (demo->shutdown_func)
	{
		demo->shutdown_func(demo, demo->user_data);
	}
	pi_fly_camera_free(demo->fly_cam);
	pi_camera_free(demo->screen_cam);

	pi_renderpipeline_ext_free(demo->pipeline);
	pi_inputmanager_destroy(demo->input_mgr);
}

static void _print_usage(void)
{
	pi_log_print(LOG_INFO, "\n按键说明：");
	pi_log_print(LOG_INFO, "键 Esc: 退出程序");
	pi_log_print(LOG_INFO, "键 R: 重置相机");
	pi_log_print(LOG_INFO, "键 Alt + Enter: 切换全屏模式和窗口模式\n");
}

PiApplication *PI_API pi_demo_create(char *title, uint width, uint height, DemoInitFunc init_func, DemoUpdateFunc update_func, DemoResizeFunc resize_func, DemoShutDownFunc shutdown_func, void *user_data)
{
	PiDemo *demo = pi_new0(PiDemo, 1);
	demo->app = pi_application_create(title, "t3d/render/shader/hlsl", width, height, _init, _update, _resize, _shutdown, demo);
	
	_print_usage();

	demo->is_window_mode = TRUE;
	demo->width = width;
	demo->height = height;

	demo->user_data = user_data;
	demo->init_func = init_func ? init_func : _pi_demo_default_init_func;
	demo->update_func = update_func ? update_func : _pi_demo_default_update_func;
	demo->resize_func = resize_func ? resize_func : _pi_demo_default_resize_func;
	demo->shutdown_func = shutdown_func ? shutdown_func : _pi_demo_default_shut_down_func;
	demo->esc_exit = TRUE;

	return demo->app;
}

InputManager *PI_API pi_demo_get_inputmanager(PiApplication *app)
{
	PiDemo *demo = (PiDemo *)app->user_data;
	return demo->input_mgr;
}
