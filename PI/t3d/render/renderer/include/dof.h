#ifndef INCLUDE_DOF_H
#define INCLUDE_DOF_H

#include <renderer.h>

const static char *RS_DOF_VS = "default.vs";
const static char *RS_DOF_FS = "dof.fs";

PI_BEGIN_DECLS


/**
* ������Ⱦ��
* @returns ��Ⱦ��ָ��
*/
PiRenderer *PI_API pi_dof_new_with_name(char* name);

/**
 * ������Ⱦ��
 * @returns ��Ⱦ��ָ��
 */
PiRenderer *PI_API pi_dof_new();

/**
 * ������Ⱦ��
 * @param renderer ��Ⱦ��ָ��
 * @param target_name ��������� ��ȾĿ��
 * @param view_cam_name ���ھ���������б���
 * @param color_tex_name ������ȾĿ�����ɫ����
 * @param depth_tex_name ������ȾĿ����������
 */
void PI_API pi_dof_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *color_tex_name, char *depth_tex_name);

/**
 * �ͷ���Ⱦ��
 * @param renderer ��Ⱦ��ָ��
 */
void PI_API pi_dof_free(PiRenderer *renderer);

/**
* ���ý���
* @param renderer ��Ⱦ��ָ��
* @param depth_tex_name �������
*/
void PI_API pi_dof_set_focal_distance(PiRenderer *renderer, float distance);



void PI_API pi_dof_set_focal_region(PiRenderer *renderer, float region);

void PI_API pi_dof_set_focal_range(PiRenderer *renderer, float region);

void PI_API pi_dof_set_focal_scale(PiRenderer *renderer, float region);

void PI_API pi_dof_set_point(PiRenderer* renderer, float x, float y);


PI_END_DECLS

#endif /* INCLUDE_DOF_H */
