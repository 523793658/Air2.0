#ifndef INCLUDE_LIGHTS_H
#define INCLUDE_LIGHTS_H

#include <pi_vector3.h>

typedef struct
{
	PiVector3 color;
} AmbientLight;

typedef struct
{
	PiVector3 color;
	PiVector3 dir;
} DirectionalLight;

typedef struct
{
	PiVector3 pos;
	float radius;
	float decay;
	PiVector3 color;
} PointLight;

typedef struct
{
	PiVector3 color;
	PiVector3 position;
	float range;
	float exponent;
	
	PiVector3 direction;
	float inner_cone_angle;
	float outer_cone_angle;
	float penumbra_exponent;
} SpotLight;

PI_BEGIN_DECLS

AmbientLight *PI_API pi_ambient_light_new();

void PI_API pi_ambient_light_set_color(AmbientLight *light, float r, float g, float b);

DirectionalLight *PI_API pi_directional_light_new();

void PI_API pi_directional_light_set_color(DirectionalLight *light, float r, float g, float b);

void PI_API pi_directional_light_set_dir(DirectionalLight *light, float x, float y, float z);

PointLight *PI_API pi_point_light_new();

void PI_API pi_point_light_set_pos(PointLight *light, float x, float y, float z);

void PI_API pi_point_light_set_color(PointLight *light, float r, float g, float b);

void PI_API pi_point_light_set_decay(PointLight *light, float decay);

void PI_API pi_point_light_set_radius(PointLight *light, float radius);

void PI_API pi_point_light_get_shape(PointLight *light, float *radius, PiVector3 *pos);

SpotLight *PI_API pi_spot_light_new();

void PI_API pi_spot_light_set_color(SpotLight *light, float r, float g, float b);

void PI_API pi_spot_light_set_position(SpotLight *light, float x, float y, float z);

void PI_API pi_spot_light_set_falloff(SpotLight *light, float range, float exponent);

void PI_API pi_spot_light_set_direction(SpotLight *light, float x, float y, float z);

void PI_API pi_spot_light_set_cone(SpotLight *light, float inner_cone_angle, float outer_cone_angle, float penumbra_exponent);

void PI_API pi_light_free(void *light);

PI_END_DECLS

#endif /* INCLUDE_LIGHTS_H */