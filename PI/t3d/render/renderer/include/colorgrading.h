#ifndef INCLUDE_COLORGRADING_H
#define INCLUDE_COLORGRADING_H

#include <renderer.h>
#include <texture.h>

/**
 * Color Grading: 颜色映射表
 * 作用：使用lut（look up table）作为颜色映射，后处理的一种效果
 * 输入：Texture，场景纹理
 * 输出：Texture，经过color grading映射之后的场景纹理
 * 映射表：3D Texture，表示RGB空间的映射关系
 */

const static char* RS_COLOR_GRADING_VS = "default.vs";
const static char* RS_COLOR_GRADING_FS = "color_grading.fs";

PI_BEGIN_DECLS

PiRenderer* PI_API pi_colorgrading_new();

void PI_API pi_colorgrading_deploy(PiRenderer* renderer, char* input_name, char* output_name);

void PI_API pi_colorgrading_free(PiRenderer* renderer);

/* lut：look up table，颜色空间映射表，一般用16*16*16的三维纹理即可 */
void PI_API pi_colorgrading_set_lut(PiRenderer *renderer, PiTexture *lut);

PI_END_DECLS

#endif /* INCLUDE_COLORGRADING_H */
