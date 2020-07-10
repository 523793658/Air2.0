#include "inputmanager.h"
#include "key_define.h"

void PI_API pi_inputmanager_window_msg_callback(void *user_data, void *event_data);
void PI_API pi_inputmanager_dummy_msg_callback(void *user_data, void *event_data) {}

InputManager* PI_API pi_inputmanager_create(PiApplication* app)
{
	InputManager* mgr = pi_new0(InputManager, 1);
	mgr->app = app;
	mgr->keyboard_listeners = pi_vector_new();
	mgr->mouse_key_listeners = pi_vector_new();
	mgr->mouse_motion_listeners = pi_vector_new();
	mgr->mouse_wheel_listeners = pi_vector_new();
	mgr->msg_listener = pi_new0(WinMsgListener, 1);

	mgr->msg_listener->wm_user_data = mgr;
	switch (app->type)
	{
	case PAT_DUMMY:
		mgr->msg_listener->callback = pi_inputmanager_dummy_msg_callback;
		break;
	case PAT_WINDOWS:
		mgr->msg_listener->callback = pi_inputmanager_window_msg_callback;
		break;
	default:
		break;
	}
	if (mgr->msg_listener->callback)
	{
		pi_application_regist_msg_listener(app, mgr->msg_listener);
	}

	return mgr;
}

void PI_API pi_inputmanager_destroy(InputManager* mgr)
{
	pi_vector_free(mgr->keyboard_listeners);
	pi_vector_free(mgr->mouse_key_listeners);
	pi_vector_free(mgr->mouse_motion_listeners);
	pi_vector_free(mgr->mouse_wheel_listeners);
	pi_application_unregist_msg_listener(mgr->app, mgr->msg_listener);
	pi_free(mgr->msg_listener);
	pi_free(mgr);
}

static PiBool PI_API _vector_remove_func(void *listener, const void *data)
{
	return data == listener;
}

void PI_API pi_inputmanager_regist_keyboard_listener(InputManager *mgr, KeyboardListener *listener)
{
	pi_vector_push(mgr->keyboard_listeners, listener);
}

void PI_API pi_inputmanager_unregist_keyboard_listener(InputManager *mgr, KeyboardListener *listener)
{
	pi_vector_remove_if(mgr->keyboard_listeners, _vector_remove_func, listener);
}

void PI_API pi_inputmanager_regist_mouse_key_listener(InputManager *mgr, MouseKeyListener *listener)
{
	pi_vector_push(mgr->mouse_key_listeners, listener);
}

void PI_API pi_inputmanager_unregist_mouse_key_listener(InputManager *mgr, MouseKeyListener *listener)
{
	pi_vector_remove_if(mgr->mouse_key_listeners, _vector_remove_func, listener);
}

void PI_API pi_inputmanager_regist_mouse_motion_listener(InputManager *mgr, MouseMotionListener *listener)
{
	pi_vector_push(mgr->mouse_motion_listeners, listener);
}

void PI_API pi_inputmanager_unregist_mouse_motion_listener(InputManager *mgr, MouseMotionListener *listener)
{
	pi_vector_remove_if(mgr->mouse_motion_listeners, _vector_remove_func, listener);
}

void PI_API pi_inputmanager_regist_mouse_wheel_listener(InputManager *mgr, MouseWheelListener *listener)
{
	pi_vector_push(mgr->mouse_wheel_listeners, listener);
}

void PI_API pi_inputmanager_unregist_mouse_wheel_listener(InputManager *mgr, MouseWheelListener *listener)
{
	pi_vector_remove_if(mgr->mouse_wheel_listeners, _vector_remove_func, listener);
}

typedef void (*NorifyFunc)(void* listener, void* event);

void _norify_keyboard_key_pressed(KeyboardListener* listener, KeyEvent* event)
{
	if (listener->key_pressed != NULL) {
		listener->key_pressed(listener->user_data, event->key, event->modifier);
	}
}

void _norify_keyboard_key_released(KeyboardListener* listener, KeyEvent* event)
{
	if (listener->key_released != NULL) {
		listener->key_released(listener->user_data, event->key, event->modifier);
	}
}

void _norify_mouse_key_key_pressed(MouseKeyListener* listener, MouseEvent* event)
{
	if (listener->key_pressed != NULL) {
		listener->key_pressed(listener->user_data, event->button, event->x, event->y, event->modifier);
	}
}

void _norify_mouse_key_key_released(MouseKeyListener* listener, MouseEvent* event)
{
	if (listener->key_released != NULL) {
		listener->key_released(listener->user_data, event->button, event->x, event->y, event->modifier);
	}
}

void _norify_mouse_moved(MouseMotionListener* listener, MouseEvent* event)
{
	if (listener->mouse_moved != NULL) {
		listener->mouse_moved(listener->user_data, event->button, event->x, event->y, event->modifier);
	}
}

void _norify_wheel_scrolled(MouseWheelListener* listener, MouseWheelEvent* event)
{
	if (listener->wheel_scrolled != NULL) {
		listener->wheel_scrolled(listener->user_data, event->amount, event->button, event->x, event->y, event->modifier);
	}
}

void PI_API pi_inputmanager_notify_event(InputManager *mgr, PiInputEventType type, void *event)
{
	uint i, size;
	PiVector* listeners = NULL;
	NorifyFunc norify_func =NULL;

	switch (type)
	{
	case PIET_KEYBOARD_KEY_PRESSED:
		norify_func = (NorifyFunc)_norify_keyboard_key_pressed;
		listeners = mgr->keyboard_listeners;
		break;
	case PIET_KEYBOARD_KEY_RELEASED:
		norify_func = (NorifyFunc)_norify_keyboard_key_released;
		listeners = mgr->keyboard_listeners;
		break;
	case PIET_MOUSE_KEY_PRESSED:
		norify_func = (NorifyFunc)_norify_mouse_key_key_pressed;
		listeners = mgr->mouse_key_listeners;
		break;
	case PIET_MOUSE_KEY_RELEASED:
		norify_func = (NorifyFunc)_norify_mouse_key_key_released;
		listeners = mgr->mouse_key_listeners;
		break;
	case PIET_MOUSE_MOVED:
		norify_func = (NorifyFunc)_norify_mouse_moved;
		listeners = mgr->mouse_motion_listeners;
		break;
	case PIET_MOUSE_WHEEL_SCROLLED:
		norify_func = (NorifyFunc)_norify_wheel_scrolled;
		listeners = mgr->mouse_wheel_listeners;
		break;
	default:
		break;
	}
	size = pi_vector_size(listeners);
	for (i = 0; i < size; i++) {
		void* listener = pi_vector_get(listeners, i);
		norify_func(listener, event);
	}
}

KeyEvent* PI_API pi_inputmanager_new_key_event()
{
	return pi_new0(KeyEvent, 1);
}

MouseEvent* PI_API pi_inputmanager_new_mouse_event()
{
	return pi_new0(MouseEvent, 1);
}

MouseWheelEvent* PI_API pi_inputmanager_new_mouse_wheel_event()
{
	return pi_new0(MouseWheelEvent, 1);
}

void PI_API pi_inputmanager_event_free(void *event)
{
	pi_free(event);
}

void PI_API pi_inputmanager_key_event_set(KeyEvent *event, uint key, uint modifier)
{
	event->key = key;
	event->modifier = modifier;
}

void PI_API pi_inputmanager_mouse_event_set(MouseEvent *event, uint button, uint x, uint y, uint modifier)
{
	event->button = button;
	event->x = x;
	event->y = y;
	modifier = modifier;
}

void PI_API pi_inputmanager_mouse_wheel_event_set(MouseWheelEvent *event, sint amount, uint button, uint x, uint y, uint modifier)
{
	event->amount = amount;
	event->button = button;
	event->x = x;
	event->y = y;
	event->modifier = modifier;
}
