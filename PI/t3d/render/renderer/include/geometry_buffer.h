#ifndef INCLUDE_GEOMETRY_BUFFERING_H
#define INCLUDE_GEOMETRY_BUFFERING_H

#include <renderer.h>

/**
 * g-buffer渲染器
 */

PI_BEGIN_DECLS

/**
 * 创建渲染器
 * @returns 渲染器指针
 */
PiRenderer *PI_API pi_geometry_buffer_new();

/**
 * 部署渲染器
 * @param renderer 渲染器指针
 * @param target_name 渲染目标名
 * @param view_cam_name 场景相机名
 * @param entity_list_name g-buffer体物体列表，材质Shader必须为model_pl.vs/geometry_buffer.fs
 * @param g_buffer_tex0_name g-buffer的color0纹理名
 * @param g_buffer_tex1_name g-buffer的color1纹理名
 * @param g_buffer_tex2_name g-buffer的color2纹理名
 * @param g_buffer_tex3_name g-buffer的color3纹理名
 * @param g_buffer_depth_name g-buffer的depth纹理名
 */
void PI_API pi_geometry_buffer_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *entity_list_name, char *g_buffer_tex0_name, char *g_buffer_tex1_name, char *g_buffer_tex2_name, char *g_buffer_tex3_name, char *g_buffer_depth_name);

/**
 * 释放渲染器
 * @param renderer 渲染器指针
 */
void PI_API pi_geometry_buffer_free(PiRenderer *renderer);

PI_END_DECLS

#endif /* INCLUDE_GEOMETRY_BUFFERING_H */