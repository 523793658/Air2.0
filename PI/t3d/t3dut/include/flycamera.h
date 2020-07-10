#ifndef INCLUDE_FLYCAMERA_H
#define INCLUDE_FLYCAMERA_H

#include <pi_lib.h>
#include <camera.h>

#include "inputmanager.h"

/**
 * ·ÉÐÐÉãÏñ»ú¿ØÖÆÆ÷
 */

#define MAX_EFFECTIVE_TPF 1.0f

typedef enum
{
	//Rotation
	EFCA_LEFT = 0,
	EFCA_RIGHT = 1,
	EFCA_UP = 2,
	EFCA_DOWN = 3,

	//Translation
	EFCA_FORWARD = 4,
	EFCA_BACKWORD = 5,
	EFCA_LEFTWORD = 6,
	EFCA_RIGHTWORD = 7,
	EFCA_RISE = 8,
	EFCA_LOWER = 9,

	//Zoom
	EFCA_ZOOM_IN = 10,
	EFCA_ZOOM_OUT = 11,

	EFCA_COUNT = 12
} FlyCameraAction;

typedef struct
{
	PiCamera *cam;
	InputManager *input_mgr;
	float speed_move;
	float speed_rotation;
	float speed_zoom;
	int old_mouse_x;
	int old_mouse_y;
	PiVector3 up_vector;
	PiBool is_enable;
	float action_status[EFCA_COUNT];
	KeyboardListener keyboard_listener;
	MouseMotionListener mouse_motion_listener;
	MouseWheelListener mouse_wheel_listener;
} FlyCamera;

PI_BEGIN_DECLS

FlyCamera *PI_API pi_fly_camera_new(PiCamera *cam, InputManager *mgr);

void PI_API pi_fly_camera_free(FlyCamera *fly_cam);

void PI_API pi_fly_camera_update(FlyCamera *fly_cam, float tpf);

void PI_API pi_fly_camera_set_move_speed(FlyCamera *fly_cam, float speed);

void PI_API pi_fly_camera_set_rotation_speed(FlyCamera *fly_cam, float speed);

void PI_API pi_fly_camera_set_zoom_speed(FlyCamera *fly_cam, float speed);

void PI_API pi_fly_camera_set_enable(FlyCamera *fly_cam, PiBool is_enable);

float PI_API pi_fly_camera_get_move_speed(FlyCamera *fly_cam);

float PI_API pi_fly_camera_get_rotation_speed(FlyCamera *fly_cam);

float PI_API pi_fly_camera_get_zoom_speed(FlyCamera *fly_cam);

PiBool PI_API pi_fly_camera_is_enable(FlyCamera *fly_cam);

PI_END_DECLS

#endif /* INCLUDE_FLYCAMERA_H */