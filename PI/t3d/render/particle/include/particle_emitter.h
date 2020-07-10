#ifndef INCLUDE_PARTICLE_EMITTER_H
#define INCLUDE_PARTICLE_EMITTER_H

#include "particles.h"

/**
 * 粒子发射器
 */
typedef struct
{
	ParticleCluster *cluster;
	PiBool particle_follow;
	PiSpatial *spatial;

	PiBool continue_emit;

	//StateTiming
	float time;                        //发射器的当前时间
	float delay_time;                  //发射器的延迟时间
	float emit_time;                   //下一次发射粒子的时间
	float current_life_time;           //粒子的当前生命时长，无限生命时无意义

	//ParticleCount
	uint particle_count;
	float particle_count_random;
	PiSequence *particle_count_sequence;

	//EmitterLife
	float life_time;
	float life_time_random;

	//ParticleLife
	float particle_life_time;
	float particle_life_time_random;
	PiSequence *particle_life_time_sequence;

	//Velocity
	float velocity;
	float velocity_random;
	PiSequence *velocity_sequence;

	//acceleration
	float acceleration;
	float acceleration_random;
	PiSequence *acceleration_sequence;

	//Alpha
	float alpha;
	float alpha_random;
	PiSequence *alpha_sequence;

	//Size
	PiBool size_sync;
	PiVector3 size;
	PiVector3 size_random;

	//Vortex
	PiVector3 vortex_direction;
	float vortex_angle;
	float vortex_angle_random;
	float vortex_rate;
	float vortex_rate_random;

	//Position
	PiVector3 position_offset;
	PiVector3 position_offset_random;
	float position_roundness;
	float inner_fraction;

	//Direction
	PiVector3 direction;
	PiQuaternion direction_rotation;         // 把Y轴(0,1,0)旋转到direction的四元数
	float emit_angles;
	float emit_angles_random;
	PiBool uniform_distribution;

	//Rotation
	PiVector3 rotation_angles;
	PiVector3 rotation_angles_random;

	//RotationRate
	PiVector3 rotation_rate;
	PiVector3 rotation_rate_random;

	//Gravitate
	PiVector3 gravitate_origin;
	float gravitate_force;

	//RotationRate
	PiVector3 force;
	PiVector3 force_random;

	EmitInfo emit_info;
} Emitter;

PI_BEGIN_DECLS

Emitter *PI_API pi_emitter_create();
void PI_API pi_emitter_free(Emitter *emitter);
void PI_API pi_emitter_spawn(Emitter *emitter);
void PI_API pi_emitter_update(Emitter *emitter, float tpf);
void PI_API pi_emitter_set_particle_cluster(Emitter *emitter, ParticleCluster *cluster);

PiSpatial *PI_API pi_emitter_get_spatial(Emitter *emitter);

void PI_API pi_emitter_set_particle_follow(Emitter *emitter, PiBool particle_follow);

void PI_API pi_emitter_set_count(Emitter *emitter, uint particle_count, float particle_count_random, PiSequence *particle_count_sequence);

void PI_API pi_emitter_set_continue_emit(Emitter *emitter, PiBool continue_emit);
void PI_API pi_emitter_set_emit_delay_time(Emitter *emitter, float delay_time);
//Timing
void PI_API pi_emitter_set_lift_time(Emitter *emitter, float life_time, float life_time_random);
void PI_API pi_emitter_set_particle_lift_time(Emitter *emitter, float particle_life_time, float particle_life_time_random, PiSequence *particle_life_time_sequence);

//Location
void PI_API pi_emitter_set_position_offset(Emitter *emitter, PiVector3 *position_offset);
void PI_API pi_emitter_set_position_offset_random(Emitter *emitter, PiVector3 *position_offset_random, float position_roundness, float inner_fraction);

//Angles
void PI_API pi_emitter_set_emit_angle(Emitter *emitter, float angles, float angles_random);
void PI_API pi_emitter_set_direction(Emitter *emitter, PiVector3 *direction);
void PI_API pi_emitter_set_uniform_distribution(Emitter *emitter, PiBool uniform_distribution);

//Size
void PI_API pi_emitter_set_size(Emitter *emitter, PiBool size_sync, PiVector3 *size, PiVector3 *size_random);

void PI_API pi_emitter_set_velocity(Emitter *emitter, float velocity, float velocity_random, PiSequence *velocity_sequence);

void PI_API pi_emitter_set_acceleration(Emitter *emitter, float acceleration, float acceleration_random, PiSequence *acceleration_sequence);

void PI_API pi_emitter_set_alpha(Emitter *emitter, float alpha, float alpha_random, PiSequence *alpha_sequence);

// Vortex
void PI_API pi_emitter_set_vortex_direction(Emitter *emitter, PiVector3 *direction);
void PI_API pi_emitter_set_vortex_rate(Emitter *emitter, float vortex_rate, float vortex_rate_random);
void PI_API pi_emitter_set_vortex_angle(Emitter *emitter, float vortex_angle, float vortex_angle_random);

void PI_API pi_emitter_set_rotation_rate(Emitter *emitter, PiVector3 *rotation_rate, PiVector3 *rotation_rate_random);

void PI_API pi_emitter_set_gravitate(Emitter *emitter, PiVector3 *gravitate_origin, float gravitate_force);

void PI_API pi_emitter_set_rotation_angles(Emitter *emitter, PiVector3 *rotation_angles, PiVector3 *rotation_angles_random);

void PI_API pi_emitter_set_force(Emitter *emitter, PiVector3 *force, PiVector3 *force_random);

Emitter *PI_API pi_emitter_clone(Emitter *emitter);

PI_END_DECLS

#endif /* INCLUDE_PARTICLE_EMITTER_H */