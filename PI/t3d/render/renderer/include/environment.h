#ifndef INCLUDE_ENVIRONMENT_H
#define INCLUDE_ENVIRONMENT_H

#include <pi_lib.h>
#include <pi_vector3.h>
#include <texture.h>

/* 风的数据结构 */
typedef struct 
{
	float wind_dir[2];
	float wind_intensity;
} WindData;
typedef struct  
{
	PiVector3 ambient_color;
	float shadow_density;
	PiVector3 diffuse_color;
	float shadow_z_far;
	PiVector3 diffuse_dir;
	float shadow_map_size_inv;
}DefaultLight;

typedef struct
{
	PiVector3 ambient_color;
	float shadow_density;
	PiVector3 diffuse_color1;
	float shadow_z_far;
	PiVector3 diffuse_dir1;
	float shadow_map_size_inv;
	PiVector3 diffuse_color2;
	float padding3;
	PiVector3 diffuse_dir2;
	float padding4;
}ExtraLight;

typedef struct
{
	DefaultLight default_light;
	ExtraLight extra_light;
	WindData wind_data;
	PiTexture *env_tex;
	PiBool is_cylinder;
}PiEnvironment;

PI_BEGIN_DECLS

PiEnvironment* PI_API pi_environment_new();

void PI_API pi_environment_free(void* env);

void PI_API pi_environment_set_diffuse_color(PiEnvironment *env, float r, float g, float b);

void PI_API pi_environment_set_diffuse_dir(PiEnvironment *env, float x, float y, float z);

void PI_API pi_environment_set_ambient_color(PiEnvironment *env, float r, float g, float b);

void PI_API pi_environment_set_extra_diffuse_color1(PiEnvironment *env, float r, float g, float b);

void PI_API pi_environment_set_extra_diffuse_dir1(PiEnvironment *env, float x, float y, float z);

void PI_API pi_environment_set_extra_diffuse_color2(PiEnvironment *env, float r, float g, float b);

void PI_API pi_environment_set_extra_diffuse_dir2(PiEnvironment *env, float x, float y, float z);

void PI_API pi_environment_set_extra_ambient_color(PiEnvironment *env, float r, float g, float b);


void PI_API pi_environment_set_wind(PiEnvironment *env, float wind_dir_0, float wind_dir_1, float wind_intensity);

void PI_API pi_environment_set_environment_map(PiEnvironment *env, PiTexture *env_map, PiBool is_cylinder);

void PI_API pi_environment_set_shadow_intensity(PiEnvironment *env, float value);

PI_END_DECLS

#endif /* INCLUDE_ENVIRONMENT_H */