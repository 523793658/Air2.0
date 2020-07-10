#ifndef INCLUDE_PRE_LIGHTING_H
#define INCLUDE_PRE_LIGHTING_H

#include <renderer.h>

/**
 * prelighting��Ⱦ��
 */
const static char *RS_PRE_LIGHTING_VS = "default.vs";
const static char *RS_PRE_LIGHTING_FS = "prelighting.fs";

const static char *RS_LIGHT_VS = "light.vs";
const static char *RS_POINT_LIGHT_FS = "point_light.fs";
const static char *RS_SPOT_LIGHT_FS = "spot_light.fs";

PI_BEGIN_DECLS

/**
 * ������Ⱦ��
 * @returns ��Ⱦ��ָ��
 */
PiRenderer *PI_API pi_prelighting_new();

/**
 * ������Ⱦ��
 * @param renderer ��Ⱦ��ָ��
 * @param target_name ��ȾĿ����
 * @param view_cam_name ���������
 * @param point_light_list_name ���Դ�б���
 * @param spot_light_list_name �۹���б���
 * @param lighting_diffuse_name lighting-buffer��diffuse��
 * @param lighting_sepcular_name lighting-buffer��specular��
 * @param g_buffer_tex0_name g-buffer��color0������
 * @param g_buffer_depth_name g-buffer��depth������
 */
void PI_API pi_prelighting_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *point_light_list_name, char *spot_light_list_name,
                                   char *lighting_diffuse_name, char *lighting_sepcular_name, char *g_buffer_tex0_name, char *g_buffer_depth_name);

/**
 * �ͷ���Ⱦ��
 * @param renderer ��Ⱦ��ָ��
 */
void PI_API pi_prelighting_free(PiRenderer *renderer);

PI_END_DECLS

#endif /* INCLUDE_PRE_LIGHTING_H */