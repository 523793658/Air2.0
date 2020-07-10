#ifndef INCLUDE_DEFERRED_SHADING_H
#define INCLUDE_DEFERRED_SHADING_H

#include <renderer.h>

/**
 * 延迟渲染渲染器
 */
const static char *RS_DEFERRED_SHADING_VS = "default.vs";
const static char *RS_DEFERRED_SHADING_FS = "deferred_shading.fs";

typedef enum
{
	DS_SMT_NONE,
	DS_SMT_VSM,
	DS_SMT_PCF
} DeferredShadingShadowMapType;

PI_BEGIN_DECLS

/**
 * 创建渲染器
 * @returns 渲染器指针
 */
PiRenderer *PI_API pi_deferred_shading_new();

/**
 * 部署渲染器
 * @param renderer 渲染器指针
 * @param target_name 渲染目标名
 * @param view_cam_name 场景相机名
 * @param env_name 环境数据名
 * @param lighting_diffuse_name lighting-buffer的diffuse名
 * @param lighting_sepcular_name lighting-buffer的specular名
 * @param g_buffer_tex0_name g-buffer的color0纹理名
 * @param g_buffer_tex1_name g-buffer的color1纹理名
 * @param g_buffer_tex2_name g-buffer的color2纹理名
 * @param g_buffer_tex3_name g-buffer的color3纹理名
 * @param g_buffer_depth_name g-buffer的depth纹理名
 */
void PI_API pi_deferred_shading_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *env_name, char *lighting_diffuse_name, char *lighting_sepcular_name,
                                       char *g_buffer_tex0_name, char *g_buffer_tex1_name, char *g_buffer_tex2_name, char *g_buffer_tex3_name, char *g_buffer_depth_name);

/**
 * 部署阴影
 * @param renderer 渲染器指针
 * @param type 阴影类型
 * @param shadow_data_name 阴影部署数据
 */
void PI_API pi_deferred_shading_deploy_shadow(PiRenderer *renderer, DeferredShadingShadowMapType type, char *shadow_data_name);

/**
 * 部署贴花
 * @param renderer 渲染器指针
 * @param decal_map_name 贴花纹理名
 * @param decal_matrix_name 贴花的矩阵名
 * @param decal_z_far_name 贴花的z-far值名
 */
void PI_API pi_deferred_shading_deploy_decal(PiRenderer *renderer, char *decal_map_name, char *decal_matrix_name, char *decal_z_far_name);

/**
 * 释放渲染器
 * @param renderer 渲染器指针
 */
void PI_API pi_deferred_shading_free(PiRenderer *renderer);

PI_END_DECLS

#endif /* INCLUDE_DEFERRED_SHADING_H */