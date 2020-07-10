#ifndef INCLUDE_PARTICLES_H
#define INCLUDE_PARTICLES_H

#include <pi_spatial.h>
#include <pi_sequence.h>
#include <texture.h>
#include "vector4.h"
#include "renderstate.h"
#include "instance_entity.h"
// BlendMode
typedef enum
{
	EPBM_ALPHA,
	EPBM_ADDITIVE,
	EPBM_MODULATE,
	EPBM_NORMAL,
	EPBM_ALPHA_R,
	EPBM_ADDITIVE_R,
	EPBM_MODULATE_R,
	EPBM_NORMAL_R
} EParticleBlendMode;

// Appearance
typedef enum
{
	EPA_TEXTURE_MAPPING,
	EPA_FLUCTUATION,
} EParticleAppearance;

/**
 * 发射信息
 */
typedef struct EmitInfo
{
	PiSpatial *emitter_spatial;		//发射器空间

	float life_time;				//粒子生命

	PiVector3 position;				//初始相对发射器的位置

	PiVector3 emitter_pos;           //发射器的位置

	PiVector3 direction;			//初速度方向
	float velocity;					//初速度
	float acceleration;             //加速度

	PiVector3 gravitate_origin;     //吸附点相对发射器的位置
	float gravitate_force;          //吸附力大小

	PiVector3 vortex_direction;     //螺旋方向
	float vortex_rate;              //螺旋速度
	float vortex_angle;             //初始螺旋角度

	PiVector3 rotation_angles;		//初始旋转角
	PiVector3 rotation_rate;		//初始旋转速度

	float alpha;					//初始Alpha

	PiBool invert;					//是否逆向混合

	PiVector3 size;					//初始大小

	PiVector3 force;                //初始的力

	PiBool hasRotation;
} EmitInfo;

//粒子原型

typedef struct Particle Particle;
typedef struct ParticleCluster ParticleCluster;

typedef void (*ClusterUpdateFunc)(ParticleCluster *cluster, float tpf);
typedef void (*ParticleInitFunc)(ParticleCluster *cluster, Particle *particle);
typedef void (*ParticleDestroyFunc)(ParticleCluster *cluster, Particle *particle);
typedef void (*ParticleUpdateFunc)(ParticleCluster *cluster, Particle *particle, float tpf);
typedef void (*ParticleSpawnFunc)(ParticleCluster *cluster, Particle *particle);
typedef void (*ParticleDieFunc)(ParticleCluster *cluster, Particle *particle);
typedef void (*ParticleGetEntitiesFunc)(ParticleCluster *cluster, PiVector *dst);

/**
 * 粒子簇
 */
struct ParticleCluster
{
	PiVector *active_list;
	PiVector *idle_list;
	InstanceEntity* instance_entity;
	PiAABBBox aabb;

	PiSequence *alpha_sequence;
	float		global_alpha;
	PiSequence *color_sequence;
	PiSequence *size_sequence;

	void *impl;

	ClusterUpdateFunc cluster_update_func;
	ParticleInitFunc particle_init_func;
	ParticleDestroyFunc particle_destroy_func;
	ParticleUpdateFunc particle_update_func;
	ParticleSpawnFunc particle_spawn_func;
	ParticleDieFunc particle_die_func;
	ParticleGetEntitiesFunc particle_get_entities_func;
	PiVector4 tile_info;

	int priority;
	PiBool random_start;
	PiBool blend_enable;
	PiTexture *texture;
	SamplerState ss_tex;

	EParticleBlendMode blend_mode;

	EParticleAppearance appearance;

	PiBool depth_enable;
	PiBool depth_write_enable;
	uint max_count;
};

/**
 * 粒子
 */
struct Particle
{
	float life_time;
	float time;

	EmitInfo info;
	PiVector4 other_data;

	void *impl;
};

PI_BEGIN_DECLS

void PI_API pi_particle_cluster_update(ParticleCluster *cluster, float tpf);

void PI_API pi_particle_cluster_spawn(ParticleCluster *cluster);

void PI_API pi_particle_cluster_get_entities(ParticleCluster *cluster, PiVector *dst);

void PI_API pi_particle_cluster_set_alpha_sequence(ParticleCluster *cluster, PiSequence *alpha_sequence);

void PI_API pi_particle_cluster_set_color_sequence(ParticleCluster *cluster, PiSequence *color_sequence);

void PI_API pi_particle_cluster_set_size_sequence(ParticleCluster *cluster, PiSequence *size_sequence);

void PI_API pi_particle_cluster_set_texture(ParticleCluster *cluster, PiTexture *tex, uint tile_x, uint tile_y, float rate, PiBool blead_enable, PiBool random_start);

void PI_API pi_particle_cluster_set_blend_mode(ParticleCluster *cluster, EParticleBlendMode blend_mode);

void PI_API pi_particle_cluster_set_appearance(ParticleCluster *cluster, EParticleAppearance appearance);

void PI_API pi_particle_cluster_set_depth_enable(ParticleCluster *cluster, PiBool depth_enable);

void PI_API pi_particle_cluster_set_depth_write_enable(ParticleCluster* cluster, PiBool depth_write_enable);

ParticleCluster *particle_cluster_create(ClusterUpdateFunc cluster_update_func, ParticleInitFunc init_func, ParticleDestroyFunc destroy_func, ParticleUpdateFunc update_func, ParticleSpawnFunc spawn_func, ParticleDieFunc die_func, ParticleGetEntitiesFunc get_entities_func, void *impl);

void PI_API particle_cluster_free(ParticleCluster *cluster);

void PI_API pi_particle_cluster_set_priority(ParticleCluster* cluster, int priority);

void particle_cluster_emit(ParticleCluster *cluster, EmitInfo *info, float start_time);

void PI_API pi_particle_cluster_set_global_alpha(ParticleCluster* cluster, float global_alpha);

void PI_API particle_cluster_set_max_count(ParticleCluster *cluster, uint count);

void PI_API pi_particle_cluster_set_instance(ParticleCluster* cluster, InstanceEntity* instance);

PI_END_DECLS

#endif /* INCLUDE_PARTICLES_H */