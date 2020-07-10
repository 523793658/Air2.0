#include "flycamera.h"

static void _rotate_camera(PiCamera *cam, const float value, const PiVector3* axis, PiVector3* up)
{
	PiQuaternion quat_src;
	PiQuaternion quat;
	pi_camera_get_rotation(cam, &quat_src);
	pi_quat_from_angle_axis(&quat, axis, value);
	pi_quat_mul(&quat, &quat, &quat_src);
	pi_camera_set_rotation(cam, quat.w, quat.x, quat.y, quat.z);
	pi_camera_set_up(cam, up->x, up->y, up->z);
}

static void _move_camera(PiCamera *cam, const float value, const PiVector3* axis)
{
	PiVector3 vec_src;
	PiVector3 vec;
	pi_vec3_copy(&vec_src, pi_camera_get_location(cam));
	vec.x = axis->x * value;
	vec.y = axis->y * value;
	vec.z = axis->z * value;
	pi_vec3_add(&vec, &vec_src, &vec);
	pi_camera_set_location(cam, vec.x, vec.y, vec.z);
}

static void _zoom_camera(PiCamera * cam, const float value)
{
	if(pi_camera_is_ortho(cam)) {
		float zoom = 1.0f + 1.0f / 10.0f * value;
		float left = cam->frustum_left * zoom;
		float right = cam->frustum_right * zoom;
		float bottom = cam->frustum_bottom * zoom;
		float top = cam->frustum_top * zoom;
		float frustum_near = cam->frustum_near;
		float frustum_far = cam->frustum_far;

		pi_camera_set_frustum(cam, left, right, bottom, top, frustum_near, frustum_far, TRUE);
	}
	else
	{
		float h = cam->frustum_top;
		float w = cam->frustum_right;
		float frustum_near = cam->frustum_near;
		float frustum_far = cam->frustum_far;
		float aspect = w / h;

		float fovy = pi_math_atan(h / frustum_near) / (float)PI_PI * 2 * 180.0f;
		fovy+= value * 0.1f;
		fovy = MAX(MIN(fovy, 89.95f), 0.05f);

		pi_camera_set_perspective(cam, fovy, aspect, frustum_near, frustum_far);
	}
}

static void _fly_camera_action(FlyCamera * fly_cam, FlyCameraAction action, float value)
{
	PI_ASSERT(action < EFCA_COUNT, "Illegal fly camera action!");
	fly_cam->action_status[action] = value;
}

static void _key_pressed(void *user_data, uint key, uint modifier)
{
	FlyCamera *fly_cam = (FlyCamera *)user_data;
	PI_USE_PARAM(modifier);
	switch(key)
	{
	case 'W':
		_fly_camera_action(fly_cam, EFCA_FORWARD, 1);
		break;
	case 'S':
		_fly_camera_action(fly_cam, EFCA_BACKWORD, 1);
		break;
	case 'A':
		_fly_camera_action(fly_cam, EFCA_LEFTWORD, 1);
		break;
	case 'D':
		_fly_camera_action(fly_cam, EFCA_RIGHTWORD, 1);
		break;
	case 'Q':
		_fly_camera_action(fly_cam, EFCA_LOWER, 1);
		break;
	case 'E':
		_fly_camera_action(fly_cam, EFCA_RISE, 1);
		break;
	default:
		break;
	}
}

static void _key_released(void *user_data, uint key, uint modifier)
{
	FlyCamera *fly_cam = (FlyCamera *)user_data;
	PI_USE_PARAM(modifier);
	switch(key)
	{
	case 'W':
		_fly_camera_action(fly_cam, EFCA_FORWARD, 0);
		break;
	case 'S':
		_fly_camera_action(fly_cam, EFCA_BACKWORD, 0);
		break;
	case 'A':
		_fly_camera_action(fly_cam, EFCA_LEFTWORD, 0);
		break;
	case 'D':
		_fly_camera_action(fly_cam, EFCA_RIGHTWORD, 0);
		break;
	case 'Q':
		_fly_camera_action(fly_cam, EFCA_LOWER, 0);
		break;
	case 'E':
		_fly_camera_action(fly_cam, EFCA_RISE, 0);
		break;
	default:
		break;
	}
}

static void _mouse_moved(void *user_data, uint button, uint x, uint y, uint modifier) 
{
	FlyCamera *fly_cam = (FlyCamera *)user_data;
	float aspect = fly_cam->cam->frustum_top / fly_cam->cam->frustum_right ;
	PI_USE_PARAM(modifier);
	if((button & MOUSE_RIGHT_BUTTON) != 0) {
		int offset_x = x - fly_cam->old_mouse_x;
		int offset_y = y - fly_cam->old_mouse_y;
		if (offset_x >0)
		{
			_fly_camera_action(fly_cam, EFCA_RIGHT, offset_x / 1000.0f);
		}
		else
		{
			_fly_camera_action(fly_cam, EFCA_LEFT, -offset_x / 1000.0f);
		}
		if (offset_y >0)
		{
			_fly_camera_action(fly_cam, EFCA_DOWN, offset_y / 1000.0f  / aspect);
		}
		else
		{
			_fly_camera_action(fly_cam, EFCA_UP, -offset_y / 1000.0f  / aspect);
		}
	}
	fly_cam->old_mouse_x = x;
	fly_cam->old_mouse_y = y;
}

static void _wheel_scrooled(void *user_data, sint amount, uint button, uint x, uint y, uint modifier) 
{
	FlyCamera *fly_cam = (FlyCamera *)user_data;
	PI_USE_PARAM(button);
	PI_USE_PARAM(x);
	PI_USE_PARAM(y);
	PI_USE_PARAM(modifier);

	if(amount > 0) {
		_fly_camera_action(fly_cam, EFCA_ZOOM_IN, (float)amount);
	}
	else if (amount < 0)
	{
		_fly_camera_action(fly_cam, EFCA_ZOOM_OUT, -(float)amount);
	}
}

static void _regist_input(FlyCamera * fly_cam)
{
	fly_cam->keyboard_listener.user_data = fly_cam;
	fly_cam->keyboard_listener.key_pressed = _key_pressed;
	fly_cam->keyboard_listener.key_released = _key_released;

	fly_cam->mouse_motion_listener.user_data = fly_cam;
	fly_cam->mouse_motion_listener.mouse_moved = _mouse_moved;

	fly_cam->mouse_wheel_listener.user_data = fly_cam;
	fly_cam->mouse_wheel_listener.wheel_scrolled = _wheel_scrooled;

	pi_inputmanager_regist_keyboard_listener(fly_cam->input_mgr, &fly_cam->keyboard_listener);
	pi_inputmanager_regist_mouse_motion_listener(fly_cam->input_mgr, &fly_cam->mouse_motion_listener);
	pi_inputmanager_regist_mouse_wheel_listener(fly_cam->input_mgr, &fly_cam->mouse_wheel_listener);
}

static void _unregist_input(FlyCamera * fly_cam)
{
	pi_inputmanager_unregist_keyboard_listener(fly_cam->input_mgr, &fly_cam->keyboard_listener);
	pi_inputmanager_unregist_mouse_motion_listener(fly_cam->input_mgr, &fly_cam->mouse_motion_listener);
	pi_inputmanager_unregist_mouse_wheel_listener(fly_cam->input_mgr, &fly_cam->mouse_wheel_listener);
}

FlyCamera * PI_API pi_fly_camera_new(PiCamera * cam, InputManager* mgr)
{
	FlyCamera * fly_cam = pi_new0(FlyCamera, 1);
	fly_cam->input_mgr = mgr;
	fly_cam->cam = cam;
	fly_cam->speed_move = 5;
	fly_cam->speed_rotation = 3;
	fly_cam->speed_zoom = 1;
	pi_vec3_set(&fly_cam->up_vector, 0, 1, 0);
	fly_cam->is_enable = TRUE;

	_regist_input(fly_cam);

	return fly_cam;
}

void PI_API pi_fly_camera_free(FlyCamera * fly_cam)
{
	pi_free(fly_cam);
}

void PI_API pi_fly_camera_update(FlyCamera * fly_cam, float tpf)
{
	float effective_tpf = MIN(tpf, MAX_EFFECTIVE_TPF);

	if(fly_cam->action_status[EFCA_LEFT] != 0) {
		_rotate_camera(fly_cam->cam, -fly_cam->action_status[EFCA_LEFT] * fly_cam->speed_rotation, pi_vec3_get_yunit(), &fly_cam->up_vector);
		fly_cam->action_status[EFCA_LEFT] = 0;
	}
	if(fly_cam->action_status[EFCA_RIGHT] != 0) {
		_rotate_camera(fly_cam->cam, fly_cam->action_status[EFCA_RIGHT] * fly_cam->speed_rotation, pi_vec3_get_yunit(), &fly_cam->up_vector);
		fly_cam->action_status[EFCA_RIGHT] = 0;
	}
	if(fly_cam->action_status[EFCA_UP] != 0) {
		PiVector3 cam_dir;
		float dotvy;
		pi_camera_get_direction(fly_cam->cam, &cam_dir);
		dotvy = pi_vec3_dot(pi_vec3_get_yunit(), &cam_dir);
		if(dotvy < 0.9995f) {
			_rotate_camera(fly_cam->cam, -fly_cam->action_status[EFCA_UP] * fly_cam->speed_rotation, pi_vec3_get_xunit(), &fly_cam->up_vector);
		}
		fly_cam->action_status[EFCA_UP] = 0;
	}
	if(fly_cam->action_status[EFCA_DOWN] != 0) {
		PiVector3 cam_dir; 
		float dotvy;
		pi_camera_get_direction(fly_cam->cam, &cam_dir);
		dotvy = pi_vec3_dot(pi_vec3_get_yunit(), &cam_dir);
		if(dotvy > -0.9995f) {
			_rotate_camera(fly_cam->cam, fly_cam->action_status[EFCA_DOWN] * fly_cam->speed_rotation, pi_vec3_get_xunit(), &fly_cam->up_vector);
		}
		fly_cam->action_status[EFCA_DOWN] = 0;
	}
	if(fly_cam->action_status[EFCA_FORWARD] != 0) {
		PiVector3 direction;
		pi_camera_get_direction(fly_cam->cam, &direction);
		_move_camera(fly_cam->cam, fly_cam->action_status[EFCA_FORWARD] * fly_cam->speed_move * effective_tpf, &direction);
	}
	if(fly_cam->action_status[EFCA_BACKWORD] != 0) {
		PiVector3 direction;
		pi_camera_get_direction(fly_cam->cam, &direction);
		_move_camera(fly_cam->cam, -fly_cam->action_status[EFCA_BACKWORD] * fly_cam->speed_move * effective_tpf, &direction);
	}
	if(fly_cam->action_status[EFCA_LEFTWORD] != 0) {
		PiVector3 direction;
		PiVector3	up;
		PiVector3	left;
		pi_camera_get_direction(fly_cam->cam, &direction);
		pi_camera_get_up(fly_cam->cam, &up);
		pi_vec3_cross(&left, &direction, &up);
		_move_camera(fly_cam->cam, -fly_cam->action_status[EFCA_LEFTWORD] * fly_cam->speed_move * effective_tpf, &left);
	}
	if(fly_cam->action_status[EFCA_RIGHTWORD] != 0) {
		PiVector3 direction;
		PiVector3	up;
		PiVector3	left;
		pi_camera_get_direction(fly_cam->cam, &direction);
		pi_camera_get_up(fly_cam->cam, &up);
		pi_vec3_cross(&left, &direction, &up);
		_move_camera(fly_cam->cam, fly_cam->action_status[EFCA_RIGHTWORD] * fly_cam->speed_move * effective_tpf, &left);
	}
	if(fly_cam->action_status[EFCA_RISE] != 0) {
		PiVector3	up;
		pi_camera_get_up(fly_cam->cam, &up);
		_move_camera(fly_cam->cam, fly_cam->action_status[EFCA_RISE] * fly_cam->speed_move * effective_tpf, &up);
	}
	if(fly_cam->action_status[EFCA_LOWER] != 0) {
		PiVector3	up;
		pi_camera_get_up(fly_cam->cam, &up);
		_move_camera(fly_cam->cam, -fly_cam->action_status[EFCA_LOWER] * fly_cam->speed_move * effective_tpf, &up);
	}
	if(fly_cam->action_status[EFCA_ZOOM_IN] != 0) {
		_zoom_camera(fly_cam->cam, -fly_cam->action_status[EFCA_ZOOM_IN] * fly_cam->speed_zoom);
		fly_cam->action_status[EFCA_ZOOM_IN] = 0;
	}
	if(fly_cam->action_status[EFCA_ZOOM_OUT] != 0) {
		_zoom_camera(fly_cam->cam, fly_cam->action_status[EFCA_ZOOM_OUT] * fly_cam->speed_zoom);
		fly_cam->action_status[EFCA_ZOOM_OUT] = 0;
	}
}

void PI_API pi_fly_camera_set_move_speed(FlyCamera * fly_cam, float speed)
{
	fly_cam->speed_move = speed;
}

void PI_API pi_fly_camera_set_rotation_speed(FlyCamera * fly_cam, float speed)
{
	fly_cam->speed_rotation = speed;
}

void PI_API pi_fly_camera_set_zoom_speed(FlyCamera * fly_cam, float speed)
{
	fly_cam->speed_zoom = speed;
}

void PI_API pi_fly_camera_set_enable(FlyCamera * fly_cam, PiBool is_enable)
{
	fly_cam->is_enable = is_enable;
}

float PI_API pi_fly_camera_get_move_speed(FlyCamera * fly_cam)
{
	return fly_cam->speed_move;
}

float PI_API pi_fly_camera_get_rotation_speed(FlyCamera * fly_cam)
{
	return fly_cam->speed_rotation;
}

float PI_API pi_fly_camera_get_zoom_speed(FlyCamera * fly_cam)
{
	return fly_cam->speed_zoom;
}

PiBool PI_API pi_fly_camera_is_enable(FlyCamera * fly_cam)
{
	return fly_cam->is_enable;
}