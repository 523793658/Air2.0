#ifndef INCLUDE_REFRACTION_H
#define INCLUDE_REFRACTION_H

#include <renderer.h>

PI_BEGIN_DECLS

PiRenderer *PI_API pi_refraction_new_with_name(char* name);

PiRenderer *PI_API pi_refraction_new();

void PI_API pi_refraction_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *entity_list_name, char *scene_color_name);

void PI_API pi_refraction_free(PiRenderer *renderer);

PI_END_DECLS

#endif /* INCLUDE_REFRACTION_H */
