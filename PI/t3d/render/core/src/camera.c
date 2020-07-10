#include "camera.h"

static void _update_view_projection(PiCamera* cam)
{
	pi_mat4_mul(&cam->view_projection_matrix, &cam->projection_matrix, &cam->view_matrix);
}

static void _update_frustum(PiCamera* cam)
{
	if (cam->is_ortho) {
		pi_mat4_ortho_rh(&cam->projection_matrix, cam->frustum_left, cam->frustum_right, cam->frustum_bottom, cam->frustum_top, cam->frustum_near, cam->frustum_far);
	}
	else {
		pi_mat4_frustum_rh(&cam->projection_matrix, cam->frustum_left, cam->frustum_right, cam->frustum_bottom, cam->frustum_top, cam->frustum_near, cam->frustum_far);
	}
	_update_view_projection(cam);
	//TODO:update frustumPanes if camera culling done needed in this module
}

static void _update_view(PiCamera* cam)
{
	cam->view_matrix.m[0][0] = cam->rotation.m[0][0];
	cam->view_matrix.m[0][1] = cam->rotation.m[0][1];
	cam->view_matrix.m[0][2] = cam->rotation.m[0][2];
	cam->view_matrix.m[0][3] = -(cam->rotation.m[0][0] * cam->location.x + cam->rotation.m[0][1] * cam->location.y + cam->rotation.m[0][2] * cam->location.z);
	cam->view_matrix.m[1][0] = cam->rotation.m[1][0];
	cam->view_matrix.m[1][1] = cam->rotation.m[1][1];
	cam->view_matrix.m[1][2] = cam->rotation.m[1][2];
	cam->view_matrix.m[1][3] = -(cam->rotation.m[1][0] * cam->location.x + cam->rotation.m[1][1] * cam->location.y + cam->rotation.m[1][2] * cam->location.z);
	cam->view_matrix.m[2][0] = cam->rotation.m[2][0];
	cam->view_matrix.m[2][1] = cam->rotation.m[2][1];
	cam->view_matrix.m[2][2] = cam->rotation.m[2][2];
	cam->view_matrix.m[2][3] = -(cam->rotation.m[2][0] * cam->location.x + cam->rotation.m[2][1] * cam->location.y + cam->rotation.m[2][2] * cam->location.z);
	_update_view_projection(cam);
}

static void _camera_init(PiCamera* cam)
{
	cam->location.x = 0;
	cam->location.y = 0;
	cam->location.z = 0;
	pi_mat4_set_identity(&cam->rotation);
	pi_mat4_set_identity(&cam->view_matrix);
	cam->frustum_left = -0.5;
	cam->frustum_right = 0.5;
	cam->frustum_bottom = -0.5;
	cam->frustum_top = 0.5;
	cam->frustum_near = 1.0;
	cam->frustum_far = 10.0;
	cam->is_ortho = FALSE;
	_update_frustum(cam);
}

PiCamera* PI_API pi_camera_new()
{
	PiCamera* cam = pi_new0(PiCamera, 1);
	_camera_init(cam);
	return cam;
}

void PI_API pi_camera_free(PiCamera* cam)
{
	if(cam != NULL)
	{
		pi_free(cam);
	}	
}

void PI_API pi_camera_set_frustum(PiCamera* cam, float left, float right, float bottom, float top, float near, float far, PiBool is_ortho)
{
	cam->frustum_left = left;
	cam->frustum_right = right;
	cam->frustum_bottom = bottom;
	cam->frustum_top = top;
	cam->frustum_near = near;
	cam->frustum_far = far;
	cam->is_ortho = is_ortho;

	_update_frustum(cam);
}

void PI_API pi_camera_set_perspective(PiCamera* cam, float fovy, float aspect, float near, float far)
{
	float halfHeight = pi_math_tan(fovy * (float)PI_PI / 180.0f * 0.5f) * near;
	float halfWidth = halfHeight * aspect;

	cam->frustum_left = -halfWidth;
	cam->frustum_right = halfWidth;
	cam->frustum_bottom = -halfHeight;
	cam->frustum_top = halfHeight;
	cam->frustum_near = near;
	cam->frustum_far = far;
	cam->is_ortho = FALSE;

	_update_frustum(cam);
}

void PI_API pi_camera_resize(PiCamera* cam, float aspect)
{
	cam->frustum_left = -cam->frustum_top * aspect;
	cam->frustum_right = -cam->frustum_left;

	_update_frustum(cam);
}

float PI_API pi_camera_get_aspect(PiCamera* cam)
{
	return cam->frustum_right / cam->frustum_top;
}

void PI_API pi_camera_set_location(PiCamera* cam, float x, float y, float z)
{
	cam->location.x = x;
	cam->location.y = y;
	cam->location.z = z;
	_update_view(cam);
}

void PI_API pi_camera_set_rotation(PiCamera* cam, float w, float x, float y, float z)
{
	PiQuaternion quat;
	quat.w = w;
	quat.x = x;
	quat.y = y;
	quat.z = z;
	pi_mat4_build_rotate(&cam->rotation, &quat);
	_update_view(cam);
}

void PI_API pi_camera_set_direction(PiCamera* cam, float x, float y, float z)
{
	PiVector3 direction;
	PiVector3 up;
	PiVector3 left;

	pi_camera_get_up(cam, &up);
	pi_vec3_set(&direction, x, y, z);
	pi_vec3_normalise(&direction, &direction);
	pi_vec3_cross(&left, &direction, &up);
	pi_vec3_normalise(&left, &left);
	pi_vec3_cross(&up, &left, &direction);

	cam->rotation.m[0][0] = left.x;
	cam->rotation.m[0][1] = left.y;
	cam->rotation.m[0][2] = left.z;
	cam->rotation.m[1][0] = up.x;
	cam->rotation.m[1][1] = up.y;
	cam->rotation.m[1][2] = up.z;
	cam->rotation.m[2][0] = -direction.x;
	cam->rotation.m[2][1] = -direction.y;
	cam->rotation.m[2][2] = -direction.z;

	_update_view(cam);
}

void PI_API pi_camera_set_look_at(PiCamera* cam, float x, float y, float z)
{
	PiVector3 direction;
	
	pi_vec3_set(&cam->lookat, x, y, z);
	pi_vec3_sub(&direction, &cam->lookat, &cam->location);
	pi_vec3_normalise(&direction, &direction);
	pi_camera_set_direction(cam, direction.x, direction.y, direction.z);
}

PiVector3* PI_API pi_camera_get_look_at(PiCamera* cam)
{
	return &cam->lookat;
}

void PI_API pi_camera_set_up(PiCamera* cam, float x, float y, float z)
{
	PiVector3 direction;
	PiVector3 up;
	PiVector3 left;

	pi_camera_get_direction(cam, &direction);
	pi_vec3_set(&up, x, y, z);
	pi_vec3_normalise(&up, &up);
	pi_vec3_cross(&left, &direction, &up);
	pi_vec3_normalise(&left, &left);
	pi_vec3_cross(&up, &left, &direction);

	cam->rotation.m[0][0] = left.x;
	cam->rotation.m[0][1] = left.y;
	cam->rotation.m[0][2] = left.z;
	cam->rotation.m[1][0] = up.x;
	cam->rotation.m[1][1] = up.y;
	cam->rotation.m[1][2] = up.z;
	cam->rotation.m[2][0] = -direction.x;
	cam->rotation.m[2][1] = -direction.y;
	cam->rotation.m[2][2] = -direction.z;

	_update_view(cam);
}

void PI_API pi_camera_set_projection_matrix(PiCamera* cam, PiMatrix4* proj_mat)
{
	pi_mat4_copy(&cam->projection_matrix, proj_mat);
	_update_view_projection(cam);
}

float PI_API pi_camera_get_frustum_left(PiCamera* cam)
{
	return cam->frustum_left;
}

float PI_API pi_camera_get_frustum_right(PiCamera* cam)
{
	return cam->frustum_right;
}

float PI_API pi_camera_get_frustum_bottom(PiCamera* cam)
{
	return cam->frustum_bottom;
}

float PI_API pi_camera_get_frustum_top(PiCamera* cam)
{
	return cam->frustum_top;
}

float PI_API pi_camera_get_frustum_near(PiCamera* cam)
{
	return cam->frustum_near;
}

float PI_API pi_camera_get_frustum_far(PiCamera* cam)
{
	return cam->frustum_far;
}

PiVector3* PI_API pi_camera_get_location(PiCamera* cam)
{
	return &cam->location;
}

void PI_API pi_camera_get_rotation(PiCamera* cam, PiQuaternion* result)
{
	pi_quat_from_mat4(result, &cam->rotation);
}

void PI_API pi_camera_get_direction(PiCamera* cam, PiVector3* result)
{
	pi_vec3_set(result, -cam->rotation.m[2][0], -cam->rotation.m[2][1], -cam->rotation.m[2][2]);
}

void PI_API pi_camera_get_up(PiCamera* cam, PiVector3* result)
{
	pi_vec3_set(result, cam->rotation.m[1][0], cam->rotation.m[1][1], cam->rotation.m[1][2]);
}

PiMatrix4* PI_API pi_camera_get_view_matrix(PiCamera* cam)
{
	return &cam->view_matrix;
}

PiMatrix4* PI_API pi_camera_get_projection_matrix(PiCamera* cam)
{
	return &cam->projection_matrix;
}

PiMatrix4* PI_API pi_camera_get_view_projection_matrix(PiCamera* cam)
{
	return &cam->view_projection_matrix;
}

PiBool PI_API pi_camera_is_ortho(PiCamera* cam)
{
	return cam->is_ortho;
}

void PI_API pi_camera_world2projection(PiCamera* cam, float x, float y, float z, PiVector3* result)
{
	PiVector3 world_pos;
	pi_vec3_set(&world_pos, x, y, z);
	pi_mat4_apply_point(result, &world_pos, &cam->view_projection_matrix);
}

void PI_API pi_camera_projection2world(PiCamera* cam, float x, float y, float z, PiVector3* result)
{
	PiVector3 projection_pos;
	PiMatrix4 view_proj_mat_inverse;
	pi_vec3_set(&projection_pos, x, y, z);
	pi_mat4_inverse(&view_proj_mat_inverse, &cam->view_projection_matrix);
	pi_mat4_apply_point(result, &projection_pos, &view_proj_mat_inverse);
}

void PI_API pi_camera_screen2projection(uint screen_width, uint screen_height, float x, float y, PiVector3* result)
{
	result->x = x / (float)screen_width * 2.0f - 1.0f;
	result->y = y / (float)screen_height * 2.0f - 1.0f;
}

void PI_API pi_camera_projection2screen(uint screen_width, uint screen_height, float x, float y, PiVector3* result)
{
	result->x = ((x + 1.0f) / 2.0f) * (float)screen_width;
	result->y = ((y + 1.0f) / 2.0f) * (float)screen_height;
}

void PI_API pi_camera_world2screen(PiCamera* cam, uint screen_width, uint screen_height, float x, float y, float z, PiVector3* result)
{
	pi_camera_world2projection(cam, x, y, z, result);
	pi_camera_projection2screen(screen_width, screen_height, result->x, result->y, result);
}

void PI_API pi_camera_screen2world(PiCamera* cam, uint screen_width, uint screen_height, float x, float y, float z, PiVector3* result)
{
	pi_camera_screen2projection(screen_width, screen_height, x, y, result);
	pi_camera_projection2world(cam, result->x, result->y, z, result);
}

PiCamera* PI_API pi_camera_clone(PiCamera* src)
{
	PiCamera* camera = pi_new(PiCamera, 1);
	pi_memcpy(camera, src, sizeof(PiCamera));
	return camera;
}
