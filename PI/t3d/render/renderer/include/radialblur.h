#ifndef INCLUDE_RADIALBLUR_H
#define INCLUDE_RADIALBLUR_H

#include <renderer.h>

const static char* RS_RADIALBLUR_VS = "default.vs";
const static char* RS_RADIALBLUR_FS = "radialblur.fs";

PI_BEGIN_DECLS

PiRenderer* PI_API pi_radialblur_new();

void PI_API pi_radialblur_deploy(PiRenderer* renderer, uint width, uint height, char* input_name, char* output_name);

void PI_API pi_radialblur_free(PiRenderer* renderer);

/* 进度设置，范围是[0.0, 1.0]，默认是1.0，0表示没有效果，1表示最大效果 */
void PI_API pi_radialblur_set_progress(PiRenderer* renderer, float progress);

/* 采样距离设置，默认是1，值越大表示采样的点离当前像素点越远 */
void PI_API pi_radialblur_set_sample_dist(PiRenderer* renderer, float sample_dist);

/* 力度，默认是2.2，越大表示越糊 */
void PI_API pi_radialblur_set_sample_strength(PiRenderer* renderer, float sample_strength);

PI_END_DECLS

#endif /* INCLUDE_RADIALBLUR_H */