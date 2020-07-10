#ifndef INCLUDE_DEFERRED_SHADING_H
#define INCLUDE_DEFERRED_SHADING_H

#include <renderer.h>

/**
 * �ӳ���Ⱦ��Ⱦ��
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
 * ������Ⱦ��
 * @returns ��Ⱦ��ָ��
 */
PiRenderer *PI_API pi_deferred_shading_new();

/**
 * ������Ⱦ��
 * @param renderer ��Ⱦ��ָ��
 * @param target_name ��ȾĿ����
 * @param view_cam_name ���������
 * @param env_name ����������
 * @param lighting_diffuse_name lighting-buffer��diffuse��
 * @param lighting_sepcular_name lighting-buffer��specular��
 * @param g_buffer_tex0_name g-buffer��color0������
 * @param g_buffer_tex1_name g-buffer��color1������
 * @param g_buffer_tex2_name g-buffer��color2������
 * @param g_buffer_tex3_name g-buffer��color3������
 * @param g_buffer_depth_name g-buffer��depth������
 */
void PI_API pi_deferred_shading_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *env_name, char *lighting_diffuse_name, char *lighting_sepcular_name,
                                       char *g_buffer_tex0_name, char *g_buffer_tex1_name, char *g_buffer_tex2_name, char *g_buffer_tex3_name, char *g_buffer_depth_name);

/**
 * ������Ӱ
 * @param renderer ��Ⱦ��ָ��
 * @param type ��Ӱ����
 * @param shadow_data_name ��Ӱ��������
 */
void PI_API pi_deferred_shading_deploy_shadow(PiRenderer *renderer, DeferredShadingShadowMapType type, char *shadow_data_name);

/**
 * ��������
 * @param renderer ��Ⱦ��ָ��
 * @param decal_map_name ����������
 * @param decal_matrix_name �����ľ�����
 * @param decal_z_far_name ������z-farֵ��
 */
void PI_API pi_deferred_shading_deploy_decal(PiRenderer *renderer, char *decal_map_name, char *decal_matrix_name, char *decal_z_far_name);

/**
 * �ͷ���Ⱦ��
 * @param renderer ��Ⱦ��ָ��
 */
void PI_API pi_deferred_shading_free(PiRenderer *renderer);

PI_END_DECLS

#endif /* INCLUDE_DEFERRED_SHADING_H */