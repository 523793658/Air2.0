#include "local_water.h"
typedef void(*OnUpdateFunc)(struct PiSpatial* spatial, void* user_data);

static void on_update(PiSpatial* spatial, void* user_data)
{
	PiLocalWater *water = (PiLocalWater*)user_data;
	PiVector3 normal = { 0.0f, 0.0f, 1.0f };
	pi_quat_rotate_vec3(&normal, &normal, &spatial->world_rotation);
	pi_vec3_copy(&water->plane.v, &normal);
	water->plane.d = -pi_vec3_dot(&normal, &spatial->world_translation);
}

PiLocalWater *PI_API pi_local_water_new(void)
{
	PiLocalWater *water = pi_new0(PiLocalWater, 1);
	water->params.shininess = 32;
	water->entity = pi_entity_new();
	pi_vec3_set(&water->params.fog_color, 0.14f, 0.25f, 0.3f);
	pi_vec3_set(&water->params.sky_color, 0.44f, 0.55f, 0.6f);
	water->params.fog_density = 2;
	pi_local_water_set_wave(water, -1, 1, 1, 1, 1);
	water->params.refractive_index = 1.3330f;
	water->params.caustics_density = 1;
	water->params.refractive_density = 1;
	pi_vec3_set(&water->plane.v, 0.0f, 1.0f, 0.0f);
	water->flowing = TRUE;
	water->muddy = FALSE;
	pi_renderstate_set_default_sampler(&water->normal_ss);
	pi_sampler_set_filter(&water->normal_ss, TFO_MIN_MAG_LINEAR);

	pi_spatial_set_update_operation(water->entity->spatial, (OnUpdateFunc)on_update, water, FALSE);
	return water;
}

void PI_API pi_local_water_set_mesh(PiLocalWater* water, PiRenderMesh* mesh)
{
	water->mesh = mesh;
}

void PI_API pi_local_water_set_reflection_revise(PiLocalWater* water, float revise)
{
	water->params.reflection_revise = revise;
}


void PI_API pi_local_water_free(PiLocalWater *water)
{
	if (water != NULL)
	{
		if (water->material) 
		{
			pi_material_free(water->material);
		}
		pi_entity_free(water->entity);
		pi_free(water);
	}
}

void PI_API pi_local_water_set_fog(PiLocalWater *water, float fog_density, PiVector3 *color)
{
	water->params.fog_density = fog_density;
	pi_vec3_copy(&water->params.fog_color, color);
}

void PI_API pi_local_water_set_shininess(PiLocalWater *water, float shininess)
{
	water->params.shininess = shininess;
}

void PI_API pi_local_water_set_refraction(PiLocalWater *water, float refraction)
{
	water->params.refractive_index = refraction;
}

void PI_API pi_local_water_set_wave(PiLocalWater *water, float wave_dir_x, float wave_dir_y, float wave_speed, float wave_scale, float wave_density)
{
	water->params.wave_dir[0] = wave_dir_x;
	water->params.wave_dir[1] = wave_dir_y;
	water->params.wave_density = wave_density;
	water->params.wave_scale = wave_scale;
	water->params.wave_speed = wave_speed;
}

PiSpatial *PI_API pi_local_water_get_spatial(PiLocalWater *water)
{
	return water->entity->spatial;
}

void PI_API pi_local_water_set_normal_map(PiLocalWater *water, PiTexture *normal_map)
{
	water->normal_map = normal_map;
}

void PI_API pi_local_water_set_environment_map(PiLocalWater *water, PiTexture *env_cube)
{
	water->env_cube = env_cube;
}

void PI_API pi_local_water_set_sky_color(PiLocalWater *water, PiVector3 *color)
{
	pi_vec3_copy(&water->params.sky_color, color);
}

void PI_API pi_local_water_set_muddy(PiLocalWater *water, PiBool b)
{
	water->muddy = b;
}

void PI_API pi_local_water_set_flowing(PiLocalWater *water, PiBool b)
{
	water->flowing = b;
}

void PI_API pi_local_water_set_caustics_density(PiLocalWater *water, float density)
{
	water->params.caustics_density = density;
}

void PI_API pi_local_water_set_refractive_density(PiLocalWater *water, float density)
{
	water->params.refractive_density = density;
}

void PI_API pi_local_water_set_reflection_enable(PiLocalWater* water, PiBool reflection)
{
	water->reflection = reflection;
}


void PI_API pi_local_water_set_reflection_type(PiLocalWater* water, ReflectionType type)
{
	water->reflection_type = type;
}