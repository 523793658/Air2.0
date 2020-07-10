#include <billboard.h>

typedef struct
{
	PiCamera* camera;
	EFacingType facing_type;

	PiVector3 dir, up, left;
} Billboard;

static PiBool _update(struct PiController *c, float tpf)
{
	Billboard *impl = (Billboard*)c->impl;
	if(impl->facing_type != EFT_FREE)
	{
		pi_camera_get_direction(impl->camera, &impl->dir);
		pi_camera_get_up(impl->camera, &impl->up);
		pi_vec3_cross(&impl->left, &impl->dir, &impl->up);
	}
	return FALSE;
}

static PiBool _apply(PiController *c, ControllerApplyType type, void *obj)
{
	Billboard *impl = (Billboard*)c->impl;
	if(impl->facing_type != EFT_FREE)
	{
		PiVector3 scale, tmp;
		PiMatrix4 world_matrix;
		PiEntity* entity = (PiEntity*)obj;
		PiMatrix4 *mat = pi_entity_get_world_matrix(entity);
		
		pi_mat4_copy(&world_matrix, mat);
		
		pi_vec3_set(&tmp, world_matrix.m[0][0], world_matrix.m[1][0], world_matrix.m[2][0]);
		scale.x = pi_vec3_len(&tmp);
		pi_vec3_set(&tmp, world_matrix.m[0][1], world_matrix.m[1][1], world_matrix.m[2][1]);
		scale.y = pi_vec3_len(&tmp);
		pi_vec3_set(&tmp, world_matrix.m[0][2], world_matrix.m[1][2], world_matrix.m[2][2]);
		scale.z= pi_vec3_len(&tmp);
		world_matrix.m[0][0] = impl->left.x * scale.x;
		world_matrix.m[1][0] = impl->left.y * scale.x;
		world_matrix.m[2][0] = impl->left.z * scale.x;
		world_matrix.m[0][2] = impl->dir.x * scale.z;
		world_matrix.m[1][2] = impl->dir.y * scale.z;
		world_matrix.m[2][2] = impl->dir.z * scale.z;
		if(impl->facing_type == EFT_CAMERA)
		{
			world_matrix.m[0][1] = impl->up.x * scale.y;
			world_matrix.m[1][1] = impl->up.y * scale.y;
			world_matrix.m[2][1] = impl->up.z * scale.y;
		}
		else if(impl->facing_type == EFT_CAMERA_Z_AXIS)
		{
			world_matrix.m[0][1] = 0.0;
			world_matrix.m[1][1] = scale.y;
			world_matrix.m[2][1] = 0.0;
		}
		
		pi_mat4_copy(mat, &world_matrix);
	}

	return TRUE;
}

PiController* PI_API pi_billboard_new()
{
	Billboard *impl = pi_new0(Billboard, 1);
	PiController *c = pi_controller_new(CT_BILLBOARD, _apply, _update, impl);

	impl->facing_type = EFT_CAMERA;

	return c;
}

void PI_API pi_billboard_free(PiController *c)
{
	Billboard *impl = (Billboard*)c->impl;

	pi_free(impl);
	pi_controller_free(c);
}

void PI_API pi_billboard_set_camera(PiController *c, PiCamera* camera)
{
	Billboard *impl = (Billboard*)c->impl;

	impl->camera = camera;
}

void PI_API pi_billboard_set_facing(PiController *c, EFacingType type)
{
	Billboard *impl = (Billboard*)c->impl;

	impl->facing_type = type;
}
