#ifndef INCLUDE_EDITOR_HELPER_H
#define INCLUDE_EDITOR_HELPER_H

#include <renderer.h>
#include <texture.h>

const static char *RS_EDITOR_HELPER_VS = "default.vs";
const static char *RS_EDITOR_HELPER_FS = "editor_helper.fs";
const static char *RS_TEXTURE_MERGE_FS = "texture_merge.fs";

PI_BEGIN_DECLS

PiRenderer *PI_API pi_editor_helper_new();

void PI_API pi_editor_helper_deploy(PiRenderer *renderer, char *scene_color_name, char *scene_depth_name, char *output_name, char *scene_camera_name);

void PI_API pi_editor_helper_free(PiRenderer *renderer);

void PI_API pi_editor_helper_set_brush_enable(PiRenderer *renderer, PiBool brush_enable);

void PI_API pi_editor_helper_set_brush_pos(PiRenderer *renderer, float pos_x, float pos_y);

void PI_API pi_editor_helper_set_brush_size(PiRenderer *renderer, float size);

void PI_API pi_editor_helper_set_draw_shape_enable(PiRenderer *renderer, PiBool draw_shape_enable);

void PI_API pi_editor_helper_set_draw_shape_shapes( PiRenderer *renderer, PiVector *circleList, PiVector *rectangleList, PiVector *sectorList );

void PI_API pi_editor_helper_set_projection_enable(PiRenderer *renderer, PiBool projection_enable);

void PI_API pi_editor_helper_set_projection_texture(PiRenderer *renderer, PiTexture *tex);

void PI_API pi_editor_helper_set_projection_data(PiRenderer *renderer, float origin_x, float origin_y,
												 float length, float width, float factor);

void PI_API pi_editor_helper_set_grid_enable(PiRenderer *renderer, PiBool grid_enable);

void PI_API pi_editor_helper_set_grid_data(PiRenderer *renderer, float snap, float line_width, float factor, PiBool color_mode);

/**
 * 地形纹理的混合合并扩展功能的初始化，在编辑器启动后调用一次
 * @param renderer editor_helper渲染器
 * @param color_output 用来保存混合扩展后的BGR颜色的输出的纹理
 * @param alpha_output 用来保存混合扩展后的Alpha的输出的纹理
 */
void PI_API pi_editor_helper_texture_merge_init(PiRenderer *renderer, PiTexture *color_output) ;

/**
 * 设置地形纹理混合扩展功能的一些初始数据，比如合并地形的diffuse、glow需要单独设置两次
 * @param renderer editor_helper渲染器
 * @param scale 保存四张基本纹理的扩展系数的数组，{x,y}格式，共4个
 * @param base_color_maps 用来合并的四张基础颜色纹理的数组
 */
void PI_API pi_editor_helper_texture_merge_set(PiRenderer *renderer, float *scale, PiTexture **base_color_maps);

/**
 * 进行纹理混合扩展
 * @param renderer editor_helper渲染器
 * @param blending_alpha_maps 混合扩展需要的4层alpha纹理
 */
void PI_API pi_editor_helper_texture_merge_run(PiRenderer *renderer, PiTexture **blending_alpha_maps);

PI_END_DECLS

#endif /* INCLUDE_EDITOR_HELPER_H */