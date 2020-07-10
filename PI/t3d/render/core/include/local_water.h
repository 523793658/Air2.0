#ifndef INCLUDE_LOCAL_WATER_H
#define INCLUDE_LOCAL_WATER_H

#include <entity.h>
#include <pi_vector3.h>
#include <texture.h>
#include <pi_plane.h>

typedef enum
{
	RT_WATER,
	RT_GLASS
}ReflectionType;

typedef struct
{
	PiEntity *entity;
	PiRenderMesh* mesh;
	PiMaterial* material;
	struct WaterParams
	{
		PiVector3 fog_color;
		float fog_density;

		PiVector3 sky_color;
		float shininess;

		float wave_dir[2];
		float wave_speed;
		float wave_scale;


		float wave_density;
		float refractive_density;
		float refractive_index;
		float caustics_density;

		float reflection_revise;
		float padding1;
		float padding2;
		float padding3;
	} params;
	PiBool reflection;
	ReflectionType reflection_type;
	PiPlane plane;
	PiBool underwater_effect;
	PiBool flowing;
	PiBool muddy;
	PiTexture *normal_map;
	SamplerState normal_ss;
	PiTexture *env_cube;
	SamplerState env_cube_ss;
} PiLocalWater;

PI_BEGIN_DECLS

PiLocalWater *PI_API pi_local_water_new();

void PI_API pi_local_water_free(PiLocalWater *water);

PiSpatial *PI_API pi_local_water_get_spatial(PiLocalWater *water);

void PI_API pi_local_water_set_shininess(PiLocalWater *water, float shininess);

void PI_API pi_local_water_set_fog(PiLocalWater *water, float fog_density, PiVector3 *color);

void PI_API pi_local_water_set_wave(PiLocalWater *water, float wave_dir_x, float wave_dir_y,
	float wave_speed, float wave_scale, float wave_density);

void PI_API pi_local_water_set_refraction(PiLocalWater *water, float refraction);

void PI_API pi_local_water_set_refractive_density(PiLocalWater *water, float density);

void PI_API pi_local_water_set_normal_map(PiLocalWater *water, PiTexture *normal_map);

void PI_API pi_local_water_set_environment_map(PiLocalWater *water, PiTexture *env_cube);

void PI_API pi_local_water_set_sky_color(PiLocalWater *water, PiVector3 *color);

void PI_API pi_local_water_set_muddy(PiLocalWater *water, PiBool b);

void PI_API pi_local_water_set_flowing(PiLocalWater *water, PiBool b);

void PI_API pi_local_water_set_caustics_density(PiLocalWater *water, float density);

void PI_API pi_local_water_set_reflection_enable(PiLocalWater* water, PiBool reflection);

void PI_API pi_local_water_set_reflection_type(PiLocalWater* water, ReflectionType type);

void PI_API pi_local_water_set_mesh(PiLocalWater* water, PiRenderMesh* mesh);

void PI_API pi_local_water_set_reflection_revise(PiLocalWater* water, float revise);

PI_END_DECLS

#endif /* INCLUDE_LOCAL_WATER_H */