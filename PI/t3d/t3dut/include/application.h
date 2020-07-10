#ifndef INCLUDE_APPLICATION_H
#define INCLUDE_APPLICATION_H

#include <pi_lib.h>
#include <rendertarget.h>

typedef enum
{
	PAT_DUMMY = 0,
	PAT_WINDOWS = 1,
} PiApplicationType;

typedef struct PiApplication PiApplication;

typedef void (PI_API *AppInitFunc)(PiApplication *app, void *user_data);
typedef void (PI_API *AppUpdateFunc)(PiApplication *app, void *user_data, float tpf);
typedef void (PI_API *AppResizeFunc)(PiApplication *app, void *user_data, uint width, uint height);
typedef void (PI_API *AppShutdownFunc)(PiApplication *app, void *user_data);
typedef void (PI_API *MsgListenerCallback)(void *user_data, void *event_data);

typedef void * (PI_API *_AppImplCreateFunc)(PiApplication *app);
typedef void (PI_API *_AppImplDestroyFunc)(PiApplication *app, void *impl);
typedef void (PI_API *_AppImplRunFunc)(PiApplication *app, void *impl);
typedef void (PI_API *_AppImplStopFunc)(PiApplication *app, void *impl);

typedef struct
{
	void *wm_user_data;
	MsgListenerCallback callback;
} WinMsgListener;

struct PiApplication
{
	PiApplicationType type;

	void *hwnd;	// ´°¿Ú¾ä±ú

	wchar *shader_dir;
	wchar *title;
	uint width;
	uint height;
	PiRenderTarget *win_rt;
	double time;
	PiVector *msg_listeners;
	void *user_data;
	
	AppInitFunc init_func;
	AppUpdateFunc update_func;
	AppResizeFunc resize_func;
	AppShutdownFunc shutdown_func;

	void *impl;
	_AppImplCreateFunc create_func;
	_AppImplDestroyFunc destroy_func;
	_AppImplRunFunc run_func;
	_AppImplStopFunc stop_func;
};

PI_BEGIN_DECLS

void PI_API pi_application_set_type(PiApplicationType type);

PiApplication *PI_API pi_application_create(char *title, char *shader_dir, uint width, uint height, 
	AppInitFunc init_func, AppUpdateFunc update_func, AppResizeFunc resize_func, AppShutdownFunc shutdown_func, void *user_data);

void PI_API pi_application_set_angle_enable(PiApplication *app, PiBool b);

void PI_API pi_application_run(PiApplication *app);

void PI_API pi_application_stop(PiApplication *app);

void PI_API pi_application_destroy(PiApplication *app);

void PI_API pi_application_regist_msg_listener(PiApplication *app, WinMsgListener *listener);

void PI_API pi_application_unregist_msg_listener(PiApplication *app, WinMsgListener *listener);

//Dummy
void PI_API pi_dummy_application_set_target(PiApplication *app, PiRenderTarget *target);

void PI_API pi_dummy_application_set_time(PiApplication *app, double time);

void PI_API pi_dummy_application_init(PiApplication *app);

void PI_API pi_dummy_application_update(PiApplication *app, float tpf);

void PI_API pi_dummy_application_shutdown(PiApplication *app);

void PI_API pi_dummy_application_resize(PiApplication *app, uint width, uint height);

void PI_API pi_dummy_application_notify_msg_listener(PiApplication *app, void *msg);

//Windows
void PI_API pi_windows_application_set_sleep(PiApplication *app, uint sleep);

PI_END_DECLS

#endif /* INCLUDE_APPLICATION_H */