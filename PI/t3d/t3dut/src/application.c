#include "rendersystem.h"
#include "application.h"

static PiApplicationType PI_APPLICATION_TYPE = PAT_WINDOWS;

typedef struct WindowsApplication;
void PI_API windows_application_create(struct PiApplication *app);
void PI_API windows_application_destroy(struct PiApplication *app, struct WindowsApplication* impl);
void PI_API windows_application_run(struct PiApplication *app, struct WindowsApplication* impl);
void PI_API windows_application_stop(struct PiApplication *app, struct WindowsApplication* impl);

typedef struct DummyApplication;
void PI_API dummy_application_create(struct PiApplication *app);
void PI_API dummy_application_destroy(struct PiApplication *app, struct WindowsApplication* impl);
void PI_API dummy_application_run(struct PiApplication *app, struct WindowsApplication* impl);
void PI_API dummy_application_stop(struct PiApplication *app, struct WindowsApplication* impl);

PiApplication* PI_API pi_application_create(char *title, char *shader_dir, uint width, uint height, AppInitFunc init_func, AppUpdateFunc update_func, AppResizeFunc resize_func, AppShutdownFunc shutdown_func, void* user_data)
{
	PiApplication* app = pi_new0(PiApplication, 1);
	app->title = pi_utf8_to_wstr(title);
	app->width = width;
	app->height = height;
	app->user_data = user_data;
	app->init_func = init_func;
	app->update_func = update_func;
	app->resize_func = resize_func;
	app->shutdown_func = shutdown_func;
	app->type = PI_APPLICATION_TYPE;
	if(shader_dir != NULL)
	{
		app->shader_dir = pi_utf8_to_wstr(shader_dir);
	}
	app->msg_listeners = pi_vector_new();

	switch (PI_APPLICATION_TYPE)
	{
	case PAT_DUMMY:
		app->create_func = (_AppImplCreateFunc)dummy_application_create;
		app->destroy_func = (_AppImplDestroyFunc)dummy_application_destroy;
		app->run_func = (_AppImplRunFunc)dummy_application_run;
		app->stop_func = (_AppImplStopFunc)dummy_application_stop;
		break;
	case PAT_WINDOWS:
		app->create_func = (_AppImplCreateFunc)windows_application_create;
		app->destroy_func = (_AppImplDestroyFunc)windows_application_destroy;
		app->run_func = (_AppImplRunFunc)windows_application_run;
		app->stop_func = (_AppImplStopFunc)windows_application_stop;
		break;
	default:
		pi_vector_free(app->msg_listeners);
		pi_free(app);
		return NULL;
	}

	app->impl = app->create_func(app);

	return app;
}

void PI_API pi_application_run(PiApplication *app)
{
	app->run_func(app, app->impl);
}

void PI_API pi_application_stop(PiApplication *app)
{
	app->stop_func(app, app->impl);
}

void PI_API pi_application_destroy(PiApplication *app)
{
	app->destroy_func(app, app->impl);
}

void PI_API pi_application_regist_msg_listener(PiApplication *app, WinMsgListener *listener)
{
	pi_vector_push(app->msg_listeners, listener);
}

static PiBool PI_API _vector_remove_func(void *listener, const void *data)
{
	return data == listener;
}

void PI_API pi_application_unregist_msg_listener(PiApplication *app, WinMsgListener *listener)
{
	pi_vector_remove_if(app->msg_listeners, _vector_remove_func, listener);
}

void PI_API pi_application_set_type(PiApplicationType type)
{
	PI_APPLICATION_TYPE = type;
}