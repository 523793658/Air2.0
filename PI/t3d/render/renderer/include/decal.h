#ifndef INCLUDE_DECAL_H
#define INCLUDE_DECAL_H

#include <renderer.h>
#include <camera.h>

PI_BEGIN_DECLS

PiRenderer* PI_API pi_decal_new_with_name(char* name);

PiRenderer* PI_API pi_decal_new();

void PI_API pi_decal_deploy(PiRenderer* renderer, char* decal_map_name, char* decal_matrix_name,  char* decal_z_far_name, char* view_cam_name, char* decal_cam_name, char* entity_list_name);

void PI_API pi_decal_free(PiRenderer* renderer);

void PI_API pi_decal_update_camera(PiRenderer* renderer, PiCamera* view_cam, PiCamera* decal_cam);

void PI_API pi_decal_set_mapsize(PiRenderer* renderer, uint size);

void PI_API pi_decal_set_zfar(PiRenderer* renderer, float z_far);

PI_END_DECLS

#endif /* INCLUDE_DECAL_H */