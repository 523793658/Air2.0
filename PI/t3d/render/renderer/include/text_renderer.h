#ifndef INCLUDE_TEXT_RENDERER_H
#define INCLUDE_TEXT_RENDERER_H

#include "renderer.h"

PI_BEGIN_DECLS

PiRenderer *PI_API pi_text_renderer_new();

void PI_API pi_text_renderer_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *text_list_name);

void PI_API pi_text_renderer_delete(PiRenderer *renderer);

PI_END_DECLS

#endif /* INCLUDE_TEXT_RENDERER_H */
