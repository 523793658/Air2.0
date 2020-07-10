#ifndef INCLUDE_GEOMETRY_BUFFERING_H
#define INCLUDE_GEOMETRY_BUFFERING_H

#include <renderer.h>

/**
 * g-buffer��Ⱦ��
 */

PI_BEGIN_DECLS

/**
 * ������Ⱦ��
 * @returns ��Ⱦ��ָ��
 */
PiRenderer *PI_API pi_geometry_buffer_new();

/**
 * ������Ⱦ��
 * @param renderer ��Ⱦ��ָ��
 * @param target_name ��ȾĿ����
 * @param view_cam_name ���������
 * @param entity_list_name g-buffer�������б�����Shader����Ϊmodel_pl.vs/geometry_buffer.fs
 * @param g_buffer_tex0_name g-buffer��color0������
 * @param g_buffer_tex1_name g-buffer��color1������
 * @param g_buffer_tex2_name g-buffer��color2������
 * @param g_buffer_tex3_name g-buffer��color3������
 * @param g_buffer_depth_name g-buffer��depth������
 */
void PI_API pi_geometry_buffer_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *entity_list_name, char *g_buffer_tex0_name, char *g_buffer_tex1_name, char *g_buffer_tex2_name, char *g_buffer_tex3_name, char *g_buffer_depth_name);

/**
 * �ͷ���Ⱦ��
 * @param renderer ��Ⱦ��ָ��
 */
void PI_API pi_geometry_buffer_free(PiRenderer *renderer);

PI_END_DECLS

#endif /* INCLUDE_GEOMETRY_BUFFERING_H */