#ifndef INCLUDE_LIGHTING_H
#define INCLUDE_LIGHTING_H

#include <renderer.h>

PI_BEGIN_DECLS

PiRenderer *PI_API pi_lighting_new();

PiRenderer *PI_API pi_lighting_new_with_name(char* name);


void PI_API pi_lighting_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *entity_list_name, char *env_name, PiBool is_vl);

void PI_API pi_lighting_deploy_shadow(PiRenderer *renderer, char *shadow_data_name);

void PI_API pi_lighting_deploy_decal(PiRenderer *renderer, char *decal_map_name, char *decal_matrix_name, char *decal_z_far_name);

void PI_API pi_lighting_deploy_refraction(PiRenderer *renderer, char *refraction_map_name);

void PI_API pi_lighting_free(PiRenderer *renderer);

void PI_API pi_lighting_set_view_space_lighting(PiRenderer *renderer, PiBool is_view_space);

void PI_API pi_lighting_set_light_pass(PiRenderer* renderer, PiBool is_vs);


PI_END_DECLS

#endif /* INCLUDE_LIGHTING_RENDERER_H */