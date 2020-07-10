#ifndef INCLUDE_DEFAULT_RENDERER_H
#define INCLUDE_DEFAULT_RENDERER_H

#include <renderer.h>
#include <camera.h>

PI_BEGIN_DECLS

PiRenderer* PI_API pi_default_renderer_new_with_name(char* name);

PiRenderer* PI_API pi_default_renderer_new();

void PI_API pi_default_renderer_deploy(PiRenderer* renderer, char* target_name, char* view_cam_name, char* entity_list_name);

void PI_API pi_default_renderer_free(PiRenderer* renderer);

void PI_API pi_default_renderer_set_draw_order(PiRenderer* renderer, PiBool negtive);

PI_END_DECLS

#endif /* INCLUDE_DEFAULT_RENDERER_H */