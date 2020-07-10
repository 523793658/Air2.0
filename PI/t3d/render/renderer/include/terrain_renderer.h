#ifndef INCLUDE_TERRIAN_RENDERER_H
#define INCLUDE_TERRIAN_RENDERER_H

#include <renderer.h>
#include <texture.h>

PI_BEGIN_DECLS

PiRenderer* PI_API pi_terrain_renderer_new();

void PI_API pi_terrain_renderer_deploy(PiRenderer* renderer, char* block_list_name, char* view_cam_name, char* output_name);

void PI_API pi_terrain_renderer_free(PiRenderer* renderer);

void PI_API pi_terrain_renderer_set_wireframe(PiRenderer* renderer, PiBool is_wireframe);

PI_END_DECLS

#endif /* INCLUDE_TERRIAN_RENDERER_H */