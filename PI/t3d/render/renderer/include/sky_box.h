#ifndef INCLUDE_SKY_BOX_H
#define INCLUDE_SKY_BOX_H

#include <renderer.h>
#include <texture.h>

const static char* RS_SKY_VS = "sky_box.vs";
const static char* RS_SKY_FS = "sky_box.fs";

PI_BEGIN_DECLS

PiRenderer* PI_API pi_sky_box_new_with_name(char* name);

PiRenderer* PI_API pi_sky_box_new();

void PI_API pi_sky_box_deploy(PiRenderer* renderer, char* view_cam_name, char* output_name);

void PI_API pi_sky_box_free(PiRenderer* renderer);

void PI_API pi_sky_box_set_env_map(PiRenderer* renderer, PiTexture* cylinder_map, PiBool is_cylinder);

PI_END_DECLS

#endif /* INCLUDE_SKY_BOX_H */