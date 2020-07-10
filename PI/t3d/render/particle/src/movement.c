
#include "movement.h"

// 绕X轴、Y轴、Z轴的旋转角度，分别为Pitch、Yaw、Roll。
static void _quat_from_euler_angle(PiQuaternion *dst, float pitch, float yaw, float roll)
{
	float cofHalfPitch = pi_math_cos(pitch * 0.5f);
	float sinHalfPitch = pi_math_sin(pitch * 0.5f);
	float cofHalfYaw = pi_math_cos(yaw * 0.5f);
	float sinHalfYaw = pi_math_sin(yaw * 0.5f);
	float cofHalfRoll = pi_math_cos(roll * 0.5f);
	float sinHalfRoll = pi_math_sin(roll * 0.5f);

	dst->w = cofHalfPitch * cofHalfYaw * cofHalfRoll + sinHalfPitch * sinHalfYaw * sinHalfRoll;
	dst->x = sinHalfPitch * cofHalfYaw * cofHalfRoll - cofHalfPitch * sinHalfYaw * sinHalfRoll;
	dst->y = cofHalfPitch * sinHalfYaw * cofHalfRoll + sinHalfPitch * cofHalfYaw * sinHalfRoll;
	dst->z = cofHalfPitch * cofHalfYaw * sinHalfRoll - sinHalfPitch * sinHalfYaw * cofHalfRoll;
}

void pi_particle_move(CPUParticle *particle, EmitInfo *info, float tpf)
{
	PiVector3 acceleration;
	PiVector3 delta_displacement;
	PiVector3 delta_velocity;

	//计算上一时刻的加速度
	pi_vec3_scale(&acceleration, &info->direction, info->acceleration);
	//线性力提供的加速度
	pi_vec3_add(&acceleration, &acceleration, &info->force);

	//吸附提供的加速度
	if ((particle->update_mask & _CPU_PARTICLE_UPDATE_MASK_GRAVITATE) != 0)
	{
		PiVector3 gravitate_acceleration;
		pi_vec3_sub(&gravitate_acceleration, &info->gravitate_origin, &particle->displacement);
		pi_vec3_normalise(&gravitate_acceleration, &gravitate_acceleration);
		pi_vec3_scale(&gravitate_acceleration, &gravitate_acceleration, info->gravitate_force);
		pi_vec3_add(&acceleration, &acceleration, &gravitate_acceleration);
	}

	//计算速度在tpf时间内对位移的变化
	pi_vec3_scale(&delta_displacement, &particle->velocity, tpf);
	pi_vec3_add(&particle->displacement, &delta_displacement, &particle->displacement);

	//计算加速度在tpf时间内对位移的变化
	pi_vec3_scale(&delta_displacement, &acceleration, tpf * tpf / 2);
	pi_vec3_add(&particle->displacement, &delta_displacement, &particle->displacement);

	//计算当前时刻的新速度
	pi_vec3_scale(&delta_velocity, &acceleration, tpf);
	pi_vec3_add(&particle->velocity, &delta_velocity, &particle->velocity);
}

void pi_particle_set_size(PiSpatial *spatial, PiSequence *size_sequence, EmitInfo *info, float time)
{
	PiVector3 size;
	pi_sequence_set_time(size_sequence, time);
	pi_vec3_mul(&size, &info->size, (PiVector3 *)pi_sequence_get_value(size_sequence));
	pi_spatial_set_local_scaling(spatial, size.x, size.y, size.z);
}

void pi_particle_vortex(PiSpatial *spatial, CPUParticle *particle, EmitInfo *info, float time, PiVector3 *dst)
{
	PiQuaternion rotation;
	float current_angle;

	current_angle = info->vortex_angle + info->vortex_rate * time;

	pi_quat_from_angle_axis(&rotation, &info->vortex_direction, current_angle);

	pi_vec3_sub(dst, &particle->displacement, &info->emitter_pos);

	pi_quat_rotate_vec3(dst, dst, &rotation);

	pi_vec3_add(dst, dst, &info->emitter_pos);
}

void pi_particle_rotate(CPUParticle *particle, EmitInfo *info, float time)
{
	PiVector3 rotation_angles;

	pi_vec3_scale(&rotation_angles, &info->rotation_rate, time);
	pi_vec3_add(&rotation_angles, &rotation_angles, &info->rotation_angles);

	_quat_from_euler_angle(&particle->rotation, rotation_angles.x, rotation_angles.y, rotation_angles.z);

	pi_spatial_set_local_rotation(particle->spatial, particle->rotation.w, particle->rotation.x, particle->rotation.y, particle->rotation.z);
}
