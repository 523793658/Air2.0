#ifndef INCLUDE_WIEWPORT_RENDERER_H
#define INCLUDE_WIEWPORT_RENDERER_H
#include <pi_lib.h>
#include "app_renderer.h"
#include "renderer.h"

PI_BEGIN_DECLS
PiRenderer* PI_API app_dino_renderer_new();
void PI_API app_dino_renderer_deploy(PiRenderer* renderer, char* target_name);
void PI_API app_dino_renderer_free(PiRenderer* renderer);

PI_END_DECLS


#endif