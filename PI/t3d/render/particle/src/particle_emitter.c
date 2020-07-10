
#include "particle_emitter.h"
#include <pi_random.h>
#include "pi_quaternion.h"

static float _random()
{
	return pi_random_float(0, 1);
}

static float _get_varying_float(float base_value, float random_value)
{
	return base_value * (1.0f - random_value * _random());
}

static float _get_varying_float_sequence(float base_value, float random_value, PiSequence *sequence)
{
	return base_value * (1.0f - random_value * _random()) * (*(float *)pi_sequence_get_value(sequence));
}

static void _get_varying_vector3(PiVector3 *base_value, PiVector3 *random_value, PiVector3 *dst)
{
	pi_vec3_copy(dst, base_value);
	dst->x += random_value->x * (_random() - 0.5f) * 2;
	dst->y += random_value->y * (_random() - 0.5f) * 2;
	dst->z += random_value->z * (_random() - 0.5f) * 2;
}

static void _get_emit_position_roundness3d(PiVector3 *offset, PiVector3 *offset_random, PiVector3 *roundness3d, float inner_fraction, PiVector3 *dst)
{
	//TODO:使用后2个参数参与计算
	float a = offset_random->x, b = offset_random->y, c = offset_random->z;
	float x = pi_random_float(-a, a), y = pi_random_float(-b, b), z = pi_random_float(-c, c);
	float tempx = 0.0f, tempy = 0.0f, tempz = 0.0f, tempw = 0.0f;
	float long_axle = roundness3d->x * a, short_axle = roundness3d->y * b, height_axle = roundness3d->z * c;
	float a2 = long_axle * long_axle, b2 = short_axle * short_axle, c2 = height_axle * height_axle;
	PiVector3 temp_dst, abs_temp_dst, temp_vec, cube_vec;

	/* 计算随机点到中心的比例，如果点在coord的边界则该比例为1 */
	if (a != 0.0f)
	{
		tempx = pi_math_abs(x / a);
	}

	if (b != 0.0f)
	{
		tempy = pi_math_abs(y / b);
	}

	if (c != 0.0f)
	{
		tempz = pi_math_abs(z / c);
	}

	tempw = MAX(MAX(tempx, tempy), tempz);

	if (tempw == 0.0f)
	{
		dst->x = 0.0f;
		dst->y = 0.0f;
		dst->z = 0.0f;
	}
	else
	{
		float s;
		/* temp_dst点所对应的立方体边界 */
		temp_dst.x = x / tempw;
		temp_dst.y = y / tempw;
		temp_dst.z = z / tempw;
		abs_temp_dst.x = pi_math_abs(temp_dst.x);
		abs_temp_dst.y = pi_math_abs(temp_dst.y);
		abs_temp_dst.z = pi_math_abs(temp_dst.z);
		/* 调整随机点的位置，将概率低的位置映射到相应概率高的位置 */
		s = tempw - inner_fraction;

		if (s >= 0.0f)
		{
			dst->x = x;
			dst->y = y;
			dst->z = z;
		}
		else
		{
			float temp = pi_random_float(0.0f, 1.0f);
			s  = (s + 0.1f) * 10;

			if (s >= temp)
			{
				dst->x = x;
				dst->y = y;
				dst->z = z;
			}
			else
			{
				if (inner_fraction >= 1.0f)
				{
					dst->x = abs_temp_dst.x <= a ? temp_dst.x : ((temp_dst.x > a) ? a : -a);
					dst->y = abs_temp_dst.y <= b ? temp_dst.y : ((temp_dst.y > b) ? b : -b);
					dst->z = abs_temp_dst.z <= c ? temp_dst.z : ((temp_dst.z > c) ? c : -c);
				}
				else
				{
					/*计算圈内和圈外的体积比，用于重置*/
					float volume_ratio = inner_fraction * inner_fraction * inner_fraction;
					float diff_ratio = volume_ratio / (1 - volume_ratio);
					float dst_pos = 1 - (tempw * tempw * tempw) / diff_ratio;
					dst_pos = pi_math_pow(dst_pos, 0.33333f);
					dst->x = temp_dst.x * dst_pos;
					dst->y = temp_dst.y * dst_pos;
					dst->z = temp_dst.z * dst_pos;
				}
			}
		}

		/* 如果为椭圆则点向中心收缩 */
		if (roundness3d->x > 0 || roundness3d->y > 0 || roundness3d->z > 0)
		{
			/* 计算立方体边界到椭球边界的距离比值 */
			float last_a = a - long_axle, last_b = b - short_axle, last_c = c - height_axle;
			PiVector3 abs_dst;
			abs_dst.x = pi_math_abs(dst->x);
			abs_dst.y = pi_math_abs(dst->y);
			abs_dst.z = pi_math_abs(dst->z);

			temp_vec.x = abs_temp_dst.x - last_a;
			temp_vec.y = abs_temp_dst.y - last_b;
			temp_vec.z = abs_temp_dst.z - last_c;

			if (abs_dst.x - last_a >= 0 && abs_dst.y - last_b >= 0 && abs_dst.z - last_c >= 0)
			{
				float length = 0.0f;
				if (a2 > 0)
				{
					length += temp_vec.x * temp_vec.x / a2;
				}

				if (b2 > 0)
				{
					length += temp_vec.y * temp_vec.y / b2;
				}

				if (c2 > 0)
				{
					length += temp_vec.z * temp_vec.z / c2;
				}

				length = pi_math_sqrt(length);

				if (dst->x > 0)
				{
					cube_vec.x = dst->x - last_a;
					cube_vec.x /= length;
					dst->x = last_a + cube_vec.x;
				}
				else
				{
					cube_vec.x = dst->x + last_a;
					cube_vec.x /= length;
					dst->x = cube_vec.x - last_a;
				}

				if (dst->y > 0)
				{
					cube_vec.y = dst->y - last_b;
					cube_vec.y /= length;
					dst->y = last_b + cube_vec.y;
				}
				else
				{
					cube_vec.y = dst->y + last_b;
					cube_vec.y /= length;
					dst->y = cube_vec.y - last_b;
				}

				if (dst->z > 0)
				{
					cube_vec.z = dst->z - last_c;
					cube_vec.z /= length;
					dst->z = last_c + cube_vec.z;
				}
				else
				{
					cube_vec.z = dst->z + last_c;
					cube_vec.z /= length;
					dst->z = cube_vec.z - last_c;
				}

				//dst->x = dst->x > 0 ?
			}
		}
	}

	dst->x += offset->x;
	dst->y += offset->y;
	dst->z += offset->z;
	//_get_varying_vector3(offset, offset_random, dst);
}

/*首先随机生成在offset_random范围内的一点，重置该点的位置*/
static void _get_emit_position(PiVector3 *offset, PiVector3 *offset_random, float roundness, float inner_fraction, PiVector3 *dst)
{
	PiVector3 roundness3d;
	roundness3d.x = roundness;
	roundness3d.y = roundness;
	roundness3d.z = roundness;

	_get_emit_position_roundness3d(offset, offset_random, &roundness3d, inner_fraction, dst);
}

/* 计算粒子的发射方向，采用球面坐标系到笛卡尔坐标系的转换作为计算方法
 * rotation 把Y轴旋转到发射器的发射方向的单位四元数
 * angles 发射器的发射张角
 * angles_random 发射器的发射张角的随机范围
 * uniform_distribution
 * index
 * dst 返回的粒子发射发射方向
 */
static void _get_emit_direction(PiQuaternion *rotation, float angles, float angles_random, PiBool uniform_distribution, float index, PiVector3 *dst)
{
	/* phi_angle表示绕Y轴旋转的角度φ，theta_angle表示与Y轴的夹角θ */
	float phi_angle;
	float theta_angle = _get_varying_float(angles, angles_random);
	float sin_theta = pi_math_sin(theta_angle);

	if (uniform_distribution)
	{
		phi_angle = (float)(2 * PI_PI) * index;
	}
	else
	{
		phi_angle = pi_random_float(0.0f, (float)(2 * PI_PI));
	}

	dst->x = pi_math_cos(phi_angle) * sin_theta;
	dst->y = pi_math_cos(theta_angle);
	dst->z = -pi_math_sin(phi_angle) * sin_theta;

	pi_quat_rotate_vec3(dst, dst, rotation);
}

static void _generate_emit_info(Emitter *emitter, PiBool uniform_distribution, float index)
{
	//Size
	_get_varying_vector3(&emitter->size, &emitter->size_random, &emitter->emit_info.size);

	if (emitter->size_sync)
	{
		emitter->emit_info.size.y = emitter->emit_info.size.z = emitter->emit_info.size.x;
	}

	//Position
	_get_emit_position(&emitter->position_offset, &emitter->position_offset_random, emitter->position_roundness, emitter->inner_fraction, &emitter->emit_info.position);

	//Direction
	_get_emit_direction(&emitter->direction_rotation, emitter->emit_angles, emitter->emit_angles_random, uniform_distribution, index, &emitter->emit_info.direction);
	//Velocity
	emitter->emit_info.velocity = _get_varying_float_sequence(emitter->velocity, emitter->velocity_random, emitter->velocity_sequence);
	//Acceleration
	emitter->emit_info.acceleration = _get_varying_float_sequence(emitter->acceleration, emitter->acceleration_random, emitter->acceleration_sequence);

	//gravitate_origin
	pi_vec3_copy(&emitter->emit_info.gravitate_origin, &emitter->gravitate_origin);
	//gravitate_force
	emitter->emit_info.gravitate_force = emitter->gravitate_force;

	//Alpha
	emitter->emit_info.alpha = _get_varying_float_sequence(emitter->alpha, emitter->alpha_random, emitter->alpha_sequence);
	//Force
	_get_varying_vector3(&emitter->force, &emitter->force_random, &emitter->emit_info.force);

	//vortex_direction
	pi_vec3_copy(&emitter->emit_info.vortex_direction, &emitter->vortex_direction);
	//Vortex_angle
	emitter->emit_info.vortex_angle = _get_varying_float(emitter->vortex_angle, emitter->vortex_angle_random);
	//Vortex_rate
	emitter->emit_info.vortex_rate = _get_varying_float(emitter->vortex_rate, emitter->vortex_rate_random);

	//RotationAngles
	_get_varying_vector3(&emitter->rotation_angles, &emitter->rotation_angles_random, &emitter->emit_info.rotation_angles);
	//RotationRate
	_get_varying_vector3(&emitter->rotation_rate, &emitter->rotation_rate_random, &emitter->emit_info.rotation_rate);

	if (emitter->particle_follow)
	{
		pi_vec3_set(&emitter->emit_info.emitter_pos, 0, 0, 0);

		emitter->emit_info.emitter_spatial = emitter->spatial;
	}
	else
	{
		PiVector3 position_temp;
		PiVector3 *emitter_scaling = pi_spatial_get_world_scaling(emitter->spatial);
		PiVector3 *emitter_pos = pi_spatial_get_world_translation(emitter->spatial);
		PiQuaternion *emitter_rotation = pi_spatial_get_world_rotation(emitter->spatial);

		// particle world translation
		pi_vec3_mul(&position_temp, &emitter->emit_info.position, emitter_scaling);
		pi_quat_rotate_vec3(&position_temp, &position_temp, emitter_rotation);
		pi_vec3_add(&emitter->emit_info.position, &position_temp, emitter_pos);

		// particle world scaling
		pi_vec3_mul(&emitter->emit_info.size, &emitter->emit_info.size, emitter_scaling);

		// particle world rotation
		//pi_quat_rotate_vec3(&emitter->emit_info.rotation_angles, &emitter->emit_info.rotation_angles, emitter_rotation);
		//pi_quat_rotate_vec3(&emitter->emit_info.rotation_rate, &emitter->emit_info.rotation_rate, emitter_rotation);

		// particle world emit direction
		pi_quat_rotate_vec3(&emitter->emit_info.direction, &emitter->emit_info.direction, emitter_rotation);

		// particle world gravitate_origin
		pi_vec3_mul(&position_temp, &emitter->emit_info.gravitate_origin, emitter_scaling);
		pi_quat_rotate_vec3(&position_temp, &position_temp, emitter_rotation);
		pi_vec3_add(&emitter->emit_info.gravitate_origin, &position_temp, emitter_pos);

		// particle world vortex direction
		pi_quat_rotate_vec3(&emitter->emit_info.vortex_direction, &emitter->emit_info.vortex_direction, emitter_rotation);

		// emitter world translation
		pi_vec3_copy(&emitter->emit_info.emitter_pos, emitter_pos);

		emitter->emit_info.emitter_spatial = NULL;
	}
}

static void _sequence_update(Emitter *emitter)
{
	float t = emitter->emit_time / emitter->life_time;
	pi_sequence_set_time(emitter->particle_life_time_sequence, t);
	pi_sequence_set_time(emitter->particle_count_sequence, t);
	pi_sequence_set_time(emitter->velocity_sequence, t);
	pi_sequence_set_time(emitter->acceleration_sequence, t);
	pi_sequence_set_time(emitter->alpha_sequence, t);
}

Emitter *PI_API pi_emitter_create()
{
	float defalut_sequence_value = 1;
	Emitter *emitter = pi_new0(Emitter, 1);

	emitter->spatial = pi_spatial_node_create();

	//emitter->particle_follow = TRUE;
	emitter->particle_count = 1;
	emitter->particle_count_sequence = pi_sequence_create(EST_FLOAT);
	pi_sequence_add_node(emitter->particle_count_sequence, 0, &defalut_sequence_value);

	emitter->particle_life_time_sequence = pi_sequence_create(EST_FLOAT);
	pi_sequence_add_node(emitter->particle_life_time_sequence, 0, &defalut_sequence_value);

	emitter->velocity_sequence = pi_sequence_create(EST_FLOAT);
	pi_sequence_add_node(emitter->velocity_sequence, 0, &defalut_sequence_value);

	emitter->acceleration_sequence = pi_sequence_create(EST_FLOAT);
	pi_sequence_add_node(emitter->acceleration_sequence, 0, &defalut_sequence_value);

	emitter->alpha = 1;
	emitter->alpha_sequence = pi_sequence_create(EST_FLOAT);
	pi_sequence_add_node(emitter->alpha_sequence, 0, &defalut_sequence_value);

	pi_vec3_set(&emitter->size, 1, 1, 1);

	pi_vec3_set(&emitter->vortex_direction, 0, 1, 0);

	emitter->uniform_distribution = FALSE;

	pi_vec3_set(&emitter->direction, 0.0f, 1.0f, 0.0f);

	pi_quat_set(&emitter->direction_rotation, 1.0f, 0.0f, 0.0f, 0.0f);

	return emitter;
}

void PI_API pi_emitter_free(Emitter *emitter)
{
	pi_spatial_destroy(emitter->spatial);
	pi_sequence_free(emitter->particle_count_sequence);
	pi_sequence_free(emitter->particle_life_time_sequence);
	pi_sequence_free(emitter->velocity_sequence);
	pi_sequence_free(emitter->acceleration_sequence);
	pi_sequence_free(emitter->alpha_sequence);
	pi_free(emitter);
}

void PI_API pi_emitter_spawn(Emitter *emitter)
{
	emitter->time = 0.0f;
	emitter->emit_time = emitter->delay_time;
	emitter->current_life_time = _get_varying_float(emitter->life_time, emitter->life_time_random) + emitter->delay_time;
}

void PI_API pi_emitter_update(Emitter *emitter, float tpf)
{
	/* 如果发射器的生命有限，且下一次发射时间已经超过了生命，则发射器不再进行更新 */
	if (emitter->life_time > 0.0f && emitter->current_life_time < emitter->emit_time)
	{
		return;
	}

	emitter->time += tpf;

	/* 如果发射器的时间已经达到或者超过了下一次发射粒子的时间则需要进行粒子发射 */
	while (emitter->time >= emitter->emit_time)
	{
		float particle_life_time, particle_count, particle_start_time;

		if (emitter->life_time > 0.0f)
		{
			_sequence_update(emitter);
		}

		particle_life_time = _get_varying_float_sequence(emitter->particle_life_time, emitter->particle_life_time_random, emitter->particle_life_time_sequence);
		particle_count = _get_varying_float_sequence((float)emitter->particle_count, emitter->particle_count_random, emitter->particle_count_sequence);

		particle_start_time = emitter->time - emitter->emit_time;

		/* 新粒子的生命还大于初始的时间，则可以生成 */
		if (particle_start_time < particle_life_time)
		{
			//Particle Lifetime
			emitter->emit_info.life_time = particle_life_time;

			if (emitter->continue_emit)
			{
				_generate_emit_info(emitter, emitter->uniform_distribution, emitter->emit_time / particle_life_time);
				particle_cluster_emit(emitter->cluster, &emitter->emit_info, particle_start_time);
			}
			else
			{
				uint i;

				for (i = 0; i < particle_count; i++)
				{
					_generate_emit_info(emitter, emitter->uniform_distribution, ((float)i) / particle_count);
					particle_cluster_emit(emitter->cluster, &emitter->emit_info, particle_start_time);
				}
			}
		}

		if (emitter->continue_emit)
		{
			emitter->emit_time += particle_life_time / particle_count;
		}
		else
		{
			emitter->emit_time += particle_life_time;
		}
	}
}

void PI_API pi_emitter_set_particle_cluster(Emitter *emitter, ParticleCluster *cluster)
{
	emitter->cluster = cluster;
}

PiSpatial *PI_API pi_emitter_get_spatial(Emitter *emitter)
{
	return emitter->spatial;
}

void PI_API pi_emitter_set_particle_follow(Emitter *emitter, PiBool particle_follow)
{
	emitter->particle_follow = particle_follow;
}

void PI_API pi_emitter_set_count(Emitter *emitter, uint particle_count, float particle_count_random, PiSequence *particle_count_sequence)
{
	emitter->particle_count = particle_count;
	emitter->particle_count_random = particle_count_random;
	pi_sequence_copy(emitter->particle_count_sequence, particle_count_sequence);
}

void PI_API pi_emitter_set_continue_emit(Emitter *emitter, PiBool continue_emit)
{
	emitter->continue_emit = continue_emit;
}

void PI_API pi_emitter_set_emit_delay_time(Emitter *emitter, float delay_time)
{
	emitter->delay_time = delay_time;
	emitter->emit_time = delay_time;
}

void PI_API pi_emitter_set_lift_time(Emitter *emitter, float life_time, float life_time_random)
{
	emitter->life_time = life_time;
	emitter->life_time_random = life_time_random;
	emitter->current_life_time = _get_varying_float(life_time, life_time_random);
}

void PI_API pi_emitter_set_particle_lift_time(Emitter *emitter, float particle_life_time, float particle_life_time_random, PiSequence *particle_life_time_sequence)
{
	emitter->particle_life_time = particle_life_time;
	emitter->particle_life_time_random = particle_life_time_random;
	pi_sequence_copy(emitter->particle_life_time_sequence, particle_life_time_sequence);
}

void PI_API pi_emitter_set_position_offset(Emitter *emitter, PiVector3 *position_offset)
{
	pi_vec3_copy(&emitter->position_offset, position_offset);
}

void PI_API pi_emitter_set_position_offset_random(Emitter *emitter, PiVector3 *position_offset_random, float position_roundness, float inner_fraction)
{
	pi_vec3_copy(&emitter->position_offset_random, position_offset_random);
	emitter->position_roundness = position_roundness;
	emitter->inner_fraction = inner_fraction;
}

void PI_API pi_emitter_set_emit_angle(Emitter *emitter, float angles, float angles_random)
{
	emitter->emit_angles = angles;
	emitter->emit_angles_random = angles_random;
}

void PI_API pi_emitter_set_uniform_distribution(Emitter *emitter, PiBool uniform_distribution)
{
	emitter->uniform_distribution = uniform_distribution;
}

void PI_API pi_emitter_set_direction(Emitter *emitter, PiVector3 *direction)
{
	PiVector3 y_axis;
	pi_vec3_set(&y_axis, 0.0f, 1.0f, 0.0f);
	pi_vec3_normalise(&emitter->direction, direction);
	pi_quat_rotate_to(&emitter->direction_rotation, &y_axis, direction);
}

void PI_API pi_emitter_set_size(Emitter *emitter, PiBool size_sync, PiVector3 *size, PiVector3 *size_random)
{
	emitter->size_sync = size_sync;
	pi_vec3_copy(&emitter->size, size);
	pi_vec3_copy(&emitter->size_random, size_random);
}

void PI_API pi_emitter_set_velocity(Emitter *emitter, float velocity, float velocity_random, PiSequence *velocity_sequence)
{
	emitter->velocity = velocity;
	emitter->velocity_random = velocity_random;
	pi_sequence_copy(emitter->velocity_sequence, velocity_sequence);
}

void PI_API pi_emitter_set_acceleration(Emitter *emitter, float acceleration, float acceleration_random, PiSequence *acceleration_sequence)
{
	emitter->acceleration = acceleration;
	emitter->acceleration_random = acceleration_random;
	pi_sequence_copy(emitter->acceleration_sequence, acceleration_sequence);
}

void PI_API pi_emitter_set_alpha(Emitter *emitter, float alpha, float alpha_random, PiSequence *alpha_sequence)
{
	emitter->alpha = alpha;
	emitter->alpha_random = alpha_random;
	pi_sequence_copy(emitter->alpha_sequence, alpha_sequence);
}

void PI_API pi_emitter_set_vortex_direction(Emitter *emitter, PiVector3 *direction)
{
	pi_vec3_normalise(&emitter->vortex_direction, direction);
}

void PI_API pi_emitter_set_vortex_rate(Emitter *emitter, float vortex_rate, float vortex_rate_random)
{
	emitter->vortex_rate = vortex_rate;
	emitter->vortex_rate_random = vortex_rate_random;
}

void PI_API pi_emitter_set_vortex_angle(Emitter *emitter, float vortex_angle, float vortex_angle_random)
{
	emitter->vortex_angle = vortex_angle;
	emitter->vortex_angle_random = vortex_angle_random;
}

void PI_API pi_emitter_set_rotation_rate(Emitter *emitter, PiVector3 *rotation_rate, PiVector3 *rotation_rate_random)
{
	pi_vec3_copy(&emitter->rotation_rate, rotation_rate);
	pi_vec3_copy(&emitter->rotation_rate_random, rotation_rate_random);
	emitter->emit_info.hasRotation = TRUE;
}

void PI_API pi_emitter_set_gravitate(Emitter *emitter, PiVector3 *gravitate_origin, float gravitate_force)
{
	pi_vec3_copy(&emitter->gravitate_origin, gravitate_origin);
	emitter->gravitate_force = gravitate_force;
}

void PI_API pi_emitter_set_rotation_angles(Emitter *emitter, PiVector3 *rotation_angles, PiVector3 *rotation_angles_random)
{
	pi_vec3_copy(&emitter->rotation_angles, rotation_angles);
	pi_vec3_copy(&emitter->rotation_angles_random, rotation_angles_random);
	emitter->emit_info.hasRotation = TRUE;
}

void PI_API pi_emitter_set_force(Emitter *emitter, PiVector3 *force, PiVector3 *force_random)
{
	pi_vec3_copy(&emitter->force, force);
	pi_vec3_copy(&emitter->force_random, force_random);
}

Emitter *PI_API pi_emitter_clone(Emitter *emitter)
{
	Emitter *clone = pi_new0(Emitter, 1);
	pi_memcpy_inline(clone, emitter, sizeof(Emitter));

	clone->spatial = pi_spatial_node_create();
	pi_spatial_set_local_translation(clone->spatial, emitter->spatial->local_translation.x, emitter->spatial->local_translation.y, emitter->spatial->local_translation.z);
	pi_spatial_set_local_rotation(clone->spatial, emitter->spatial->local_rotation.w, emitter->spatial->local_rotation.x, emitter->spatial->local_rotation.y, emitter->spatial->local_rotation.z);
	pi_spatial_set_local_scaling(clone->spatial, emitter->spatial->local_scaling.x, emitter->spatial->local_scaling.y, emitter->spatial->local_scaling.z);
	pi_spatial_update(clone->spatial);

	clone->particle_count_sequence = pi_sequence_create(EST_FLOAT);
	pi_sequence_copy(clone->particle_count_sequence, emitter->particle_count_sequence);
	clone->particle_life_time_sequence = pi_sequence_create(EST_FLOAT);
	pi_sequence_copy(clone->particle_life_time_sequence, emitter->particle_life_time_sequence);
	clone->velocity_sequence = pi_sequence_create(EST_FLOAT);
	pi_sequence_copy(clone->velocity_sequence, emitter->velocity_sequence);
	clone->acceleration_sequence = pi_sequence_create(EST_FLOAT);
	pi_sequence_copy(clone->acceleration_sequence, emitter->acceleration_sequence);
	clone->alpha_sequence = pi_sequence_create(EST_FLOAT);
	pi_sequence_copy(clone->alpha_sequence, emitter->alpha_sequence);

	return clone;
}