#include <application.h>

typedef struct
{
	PiBool is_running;
	PiBool is_init;
}DummyApplication;

void* PI_API dummy_application_create(struct PiApplication *app)
{
	DummyApplication *impl = pi_new0(DummyApplication, 1);
	impl->is_running = FALSE;
	impl->is_init = FALSE;

	return impl;
}

void PI_API dummy_application_destroy(struct PiApplication *app, DummyApplication* impl)
{
	if(app->shutdown_func != NULL)
	{
		app->shutdown_func(app, app->user_data);
	}
	pi_free(app->shader_dir);

	pi_vector_free(app->msg_listeners);
	pi_free(impl);
	pi_free(app);
}

void PI_API dummy_application_run(struct PiApplication *app, DummyApplication* impl)
{
	impl->is_running = TRUE;
}

void PI_API dummy_application_stop(struct PiApplication *app, DummyApplication* impl)
{
	impl->is_running = FALSE;
}

void PI_API pi_dummy_application_set_target(PiApplication *app, PiRenderTarget *target)
{
	app->win_rt = target;
}

void PI_API pi_dummy_application_set_time(PiApplication *app, double time)
{
	app->time = time;
}

void PI_API pi_dummy_application_init(PiApplication *app)
{
	DummyApplication *impl = (DummyApplication *)app->impl;
	
	if (impl->is_running)
	{
		app->init_func(app, app->user_data);
		impl->is_init = TRUE;	
	}
}

void PI_API pi_dummy_application_update(PiApplication *app, float tpf)
{
	DummyApplication *impl = (DummyApplication *)app->impl;
	
	if (impl->is_init && impl->is_running)
	{
		app->update_func(app, app->user_data, tpf);
	}
}

void PI_API pi_dummy_application_shutdown(PiApplication *app)
{
	app->shutdown_func(app, app->user_data);
}

void PI_API pi_dummy_application_resize(PiApplication *app, uint width, uint height)
{
	DummyApplication *impl = (DummyApplication *)app->impl;

	if (impl->is_init)
	{
		app->resize_func(app, app->user_data, width, height);
	}
}