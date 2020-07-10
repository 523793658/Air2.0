#include <environment.h>


PiEnvironment* PI_API pi_environment_new()
{
	
	PiEnvironment *env = pi_new0(PiEnvironment, 1);
	env->default_light.shadow_density = 1.0f;
	env->extra_light.shadow_density = 1.0f;
	return env;
}

void PI_API pi_environment_free(void *env)
{
	if(env != NULL) 
		pi_free(env);
}

void PI_API pi_environment_set_diffuse_color(PiEnvironment *env, float r, float g, float b)
{
	pi_vec3_set(&env->default_light.diffuse_color, r, g, b);
}

void PI_API pi_environment_set_diffuse_dir(PiEnvironment *env, float x, float y, float z)
{
	pi_vec3_set(&env->default_light.diffuse_dir, x, y, z);
	pi_vec3_normalise(&env->default_light.diffuse_dir, &env->default_light.diffuse_dir);
}

void PI_API pi_environment_set_ambient_color(PiEnvironment *env, float r, float g, float b)
{
	pi_vec3_set(&env->default_light.ambient_color, r, g, b);
}

void PI_API pi_environment_set_extra_diffuse_color1(PiEnvironment *env, float r, float g, float b)
{
	pi_vec3_set(&env->extra_light.diffuse_color1, r, g, b);
}

void PI_API pi_environment_set_extra_diffuse_dir1(PiEnvironment *env, float x, float y, float z)
{
	pi_vec3_set(&env->extra_light.diffuse_dir1, x, y, z);
}

void PI_API pi_environment_set_extra_ambient_color(PiEnvironment *env, float r, float g, float b)
{
	pi_vec3_set(&env->extra_light.ambient_color, r, g, b);
}

void PI_API pi_environment_set_extra_diffuse_color2(PiEnvironment* env, float r, float g, float b)
{
	pi_vec3_set(&env->extra_light.diffuse_color2, r, g, b);
}

void PI_API pi_environment_set_extra_diffuse_dir2(PiEnvironment* env, float x, float y, float z)
{
	pi_vec3_set(&env->extra_light.diffuse_dir2, x, y, z);
}


void PI_API pi_environment_set_wind(PiEnvironment *env, float wind_dir_0, float wind_dir_1, float wind_intensity)
{
    env->wind_data.wind_dir[0] = wind_dir_0;
	env->wind_data.wind_dir[1] = wind_dir_1;
	env->wind_data.wind_intensity = wind_intensity;
}

void PI_API pi_environment_set_environment_map(PiEnvironment *env, PiTexture *env_map, PiBool is_cylinder)
{
	env->env_tex = env_map;
	env->is_cylinder = is_cylinder;
}

void PI_API pi_environment_set_shadow_intensity(PiEnvironment *env, float value)
{
	env->default_light.shadow_density = value;
	env->extra_light.shadow_density = value;
}