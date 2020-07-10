#ifndef INCLUDE_WATER_H
#define INCLUDE_WATER_H

#include <pi_lib.h>
#include <renderer.h>
#include <camera.h>
#include <texture.h>

PI_BEGIN_DECLS

PiRenderer *PI_API pi_water_new_with_name(char* name);

PiRenderer *PI_API pi_water_new();

void PI_API pi_water_deploy(PiRenderer *renderer, char *view_cam_name, char *scene_color_name, char *scene_depth_name, char *output_name, char *env_name, char* vs_key, char* fs_key);

void PI_API pi_water_deploy_local_water(PiRenderer *renderer, char *local_water_list_name);

void PI_API pi_water_deploy_reflection(PiRenderer *renderer, char *reflection_cam_name, char *reflection_list_name);

void PI_API pi_water_free(PiRenderer *renderer);

void PI_API pi_water_update_camera(PiRenderer *renderer, PiCamera *view_cam, PiCamera *reflection_cam);

void PI_API pi_water_set_normal_map(PiRenderer *renderer, PiTexture *normal_map);

void PI_API pi_water_set_caustics_map(PiRenderer *renderer, PiTexture *caustics_map);

void PI_API pi_water_set_caustics_enable(PiRenderer *renderer, PiBool is_enable);

void PI_API pi_water_set_hdr_mode_enable(PiRenderer *renderer, PiBool is_enable);

void PI_API pi_water_set_environment_enable(PiRenderer* renderer, PiBool is_enable);

PI_END_DECLS

#endif /* INCLUDE_WATER_H */
