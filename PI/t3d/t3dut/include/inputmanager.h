#ifndef INCLUDE_INPUT_MANAGER_H
#define INCLUDE_INPUT_MANAGER_H

#include <pi_lib.h>
#include "application.h"

#define KEY_MODIFIER_SHIFT 0x1
#define KEY_MODIFIER_CTRL  0x2
#define KEY_MODIFIER_ALT   0x4

#define MOUSE_LEFT_BUTTON    0x1
#define MOUSE_RIGHT_BUTTON   0x2
#define MOUSE_MIDDLE_BUTTON  0x10

typedef void (*KeyboardEventCallback)(void *user_data, uint key, uint modifier) ;
typedef void (*MouseEventCallback)(void *user_data, uint button, uint x, uint y, uint modifier) ;
typedef void (*MouseWheelEventCallback)(void *user_data, sint amount, uint button, uint x, uint y, uint modifier) ;

typedef struct
{
	void *user_data;
	KeyboardEventCallback key_pressed;
	KeyboardEventCallback key_released;
} KeyboardListener;

typedef struct
{
	void *user_data;
	MouseEventCallback key_pressed;
	MouseEventCallback key_released;
} MouseKeyListener;

typedef struct
{
	void *user_data;
	MouseEventCallback mouse_moved;
} MouseMotionListener;

typedef struct
{
	void *user_data;
	MouseWheelEventCallback wheel_scrolled;
} MouseWheelListener;

typedef struct
{
	PiApplication *app;
	PiVector *keyboard_listeners;
	PiVector *mouse_key_listeners;
	PiVector *mouse_motion_listeners;
	PiVector *mouse_wheel_listeners;

	WinMsgListener *msg_listener;
} InputManager;

typedef enum
{
	PIET_KEYBOARD_KEY_PRESSED,				 //对应KeyEvent
	PIET_KEYBOARD_KEY_RELEASED,				 //对应KeyEvent
	PIET_MOUSE_KEY_PRESSED,						//对应MouseEvent
	PIET_MOUSE_KEY_RELEASED,				   //对应MouseEvent
	PIET_MOUSE_MOVED,							   //对应MouseEvent
	PIET_MOUSE_WHEEL_SCROLLED,			   //对应MouseWheelEvent
} PiInputEventType;

typedef struct
{
	uint key;
	uint modifier;
} KeyEvent;

typedef struct
{
	uint button;
	uint x;
	uint y;
	uint modifier;
} MouseEvent;

typedef struct
{
	sint amount;
	uint button;
	uint x;
	uint y;
	uint modifier;
} MouseWheelEvent;

PI_BEGIN_DECLS

InputManager *PI_API pi_inputmanager_create(PiApplication *app);

void PI_API pi_inputmanager_destroy(InputManager *mgr);

void PI_API pi_inputmanager_regist_keyboard_listener(InputManager *mgr, KeyboardListener *listener);

void PI_API pi_inputmanager_unregist_keyboard_listener(InputManager *mgr, KeyboardListener *listener);

void PI_API pi_inputmanager_regist_mouse_key_listener(InputManager *mgr, MouseKeyListener *listener);

void PI_API pi_inputmanager_unregist_mouse_key_listener(InputManager *mgr, MouseKeyListener *listener);

void PI_API pi_inputmanager_regist_mouse_motion_listener(InputManager *mgr, MouseMotionListener *listener);

void PI_API pi_inputmanager_unregist_mouse_motion_listener(InputManager *mgr, MouseMotionListener *listener);

void PI_API pi_inputmanager_regist_mouse_wheel_listener(InputManager *mgr, MouseWheelListener *listener);

void PI_API pi_inputmanager_unregist_mouse_wheel_listener(InputManager *mgr, MouseWheelListener *listener);

void PI_API pi_inputmanager_notify_event(InputManager *mgr, PiInputEventType type, void *event);

KeyEvent *PI_API pi_inputmanager_new_key_event();

MouseEvent *PI_API pi_inputmanager_new_mouse_event();

MouseWheelEvent *PI_API pi_inputmanager_new_mouse_wheel_event();

void PI_API pi_inputmanager_event_free(void *event);

void PI_API pi_inputmanager_key_event_set(KeyEvent *event, uint key, uint modifier);

void PI_API pi_inputmanager_mouse_event_set(MouseEvent *event, uint button, uint x, uint y, uint modifier);

void PI_API pi_inputmanager_mouse_wheel_event_set(MouseWheelEvent *event, sint amount, uint button, uint x, uint y, uint modifier);

PI_END_DECLS

#endif /* INCLUDE_INPUT_MANAGER_H */