#include "lights.h"

AmbientLight *PI_API pi_ambient_light_new()
{
	AmbientLight *light = pi_new0(AmbientLight, 1);
	light->color.x = .5f;
	light->color.y = .5f;
	light->color.z = .5f;

	return light;
}

void PI_API pi_light_free(void *light)
{
	pi_free(light);
}

void PI_API pi_ambient_light_set_color(AmbientLight *light, float r, float g, float b)
{
	light->color.x = r;
	light->color.y = g;
	light->color.z = b;
}

DirectionalLight *PI_API pi_directional_light_new()
{
	DirectionalLight *light = pi_new0(DirectionalLight, 1);
	light->color.x = .5f;
	light->color.y = .5f;
	light->color.z = .5f;
	light->dir.x = 0;
	light->dir.y = .5f;
	light->dir.z = 0;

	return light;
}

void PI_API pi_directional_light_set_color(DirectionalLight *light, float r, float g, float b)
{
	light->color.x = r;
	light->color.y = g;
	light->color.z = b;
}

void PI_API pi_directional_light_set_dir(DirectionalLight *light, float x, float y, float z)
{
	light->dir.x = x;
	light->dir.y = y;
	light->dir.z = z;
}

PointLight *PI_API pi_point_light_new()
{
	PointLight *light = pi_new0(PointLight, 1);
	light->color.x = .5f;
	light->color.y = .5f;
	light->color.z = .5f;
	light->decay = 1;

	return light;
}

void PI_API pi_point_light_set_pos(PointLight *light, float x, float y, float z)
{
	light->pos.x = x;
	light->pos.y = y;
	light->pos.z = z;
}

void PI_API pi_point_light_set_color(PointLight *light, float r, float g, float b)
{
	light->color.x = r;
	light->color.y = g;
	light->color.z = b;
}

void PI_API pi_point_light_set_decay(PointLight *light, float decay)
{
	light->decay = decay;
}

void PI_API pi_point_light_set_radius(PointLight *light, float radius)
{
	light->radius = radius;
}

void PI_API pi_point_light_get_shape(PointLight *light, float *radius, PiVector3 *pos)
{
	radius[0] = light->radius;
	pos->x = light->pos.x;
	pos->y = light->pos.y;
	pos->z = light->pos.z;
}

SpotLight *PI_API pi_spot_light_new()
{
	SpotLight *light = pi_new0(SpotLight, 1);

	light->color.x = 1.0f;
	light->color.y = 1.0f;
	light->color.z = 1.0f;
	
	light->range = 1.0f;
	light->exponent = 1.0f;

	light->direction.x = 0.0f;
	light->direction.y = 1.0f;
	light->direction.z = 0.0f;

	light->inner_cone_angle = 0.5f;
	light->outer_cone_angle = 1.0f;
	light->penumbra_exponent = 1.0f;

	return light;
}

void PI_API pi_spot_light_set_color(SpotLight *light, float r, float g, float b)
{
	light->color.x = r;
	light->color.y = g;
	light->color.z = b;
}

void PI_API pi_spot_light_set_position(SpotLight *light, float x, float y, float z)
{
	light->position.x = x;
	light->position.y = y;
	light->position.z = z;
}

void PI_API pi_spot_light_set_falloff(SpotLight *light, float range, float exponent)
{
	light->range = range;
	light->exponent = exponent;
}

void PI_API pi_spot_light_set_direction(SpotLight *light, float x, float y, float z)
{
	light->direction.x = x;
	light->direction.y = y;
	light->direction.z = z;
}

void PI_API pi_spot_light_set_cone(SpotLight *light, float inner_cone_angle, float outer_cone_angle, float penumbra_exponent)
{
	light->inner_cone_angle = inner_cone_angle;
	light->outer_cone_angle = outer_cone_angle;
	light->penumbra_exponent = penumbra_exponent;
}
