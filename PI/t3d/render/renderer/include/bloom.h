#ifndef INCLUDE_BLOOM_RENDERER_H
#define INCLUDE_BLOOM_RENDERER_H

#include <renderer.h>

/**
 * 全屏泛光
 *		输入：Texture，上一阶段的场景纹理
 *		输出：Texture，全屏泛光后的场景纹理
 */

const static char* RS_BLOOM_BRIGHT_VS = "default.vs";
const static char* RS_BLOOM_BRIGHT_FS = "bright.fs";

const static char* RS_BLOOM_BLUR_H_VS = "default.vs";
const static char* RS_BLOOM_BLUR_H_FS = "blur_h.fs";

const static char* RS_BLOOM_BLUR_V_VS = "default.vs";
const static char* RS_BLOOM_BLUR_V_FS = "blur_v.fs";

const static char* RS_BLOOM_BLEND_VS = "default.vs";
const static char* RS_BLOOM_BLEND_FS = "bloom_blend.fs";

PI_BEGIN_DECLS

PiRenderer* PI_API pi_bloom_new();

void PI_API pi_bloom_deploy(PiRenderer *renderer, char *input_name, char *output_name);

void PI_API pi_bloom_set_weights(PiRenderer *renderer, float sharp, float blur);

void PI_API pi_bloom_free(PiRenderer *renderer);

PI_END_DECLS

#endif /* INCLUDE_BLOOM_H */