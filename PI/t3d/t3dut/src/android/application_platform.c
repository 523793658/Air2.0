#include "rendersystem.h"
#include "application.h"

#include "inputmanager.h"

void PI_API pi_native_input_callback(void *user_data, void *event_data)
{
}

void* PI_API windows_application_create(struct PiApplication *app)
{
	return NULL;
}

void PI_API windows_application_destroy(struct PiApplication *app, WindowsApplication* impl)
{
}

void PI_API windows_application_run(struct PiApplication *app, WindowsApplication* impl)
{
}

void PI_API windows_application_stop(struct PiApplication *app, WindowsApplication* impl)
{
}

void PI_API pi_windows_application_set_sleep(PiApplication *app, uint sleep)
{
}