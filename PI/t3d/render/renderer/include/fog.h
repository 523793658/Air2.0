#ifndef INCLUDE_FOG_H
#define INCLUDE_FOG_H

#include <renderer.h>
#include "texture.h"
const static char *RS_FOG_VS = "default.vs";
const static char *RS_FOG_FS = "fog.fs";

/* ���ģʽ */
typedef enum
{
	FM_EXP,
	FM_EXP2,
	FM_LINEAR
} FogMode;

PI_BEGIN_DECLS


/**
* ������Ч��Ⱦ��
* @returns ��Ч��Ⱦ��ָ��
*/
PiRenderer *PI_API pi_fog_new_with_name(char* name);

/**
 * ������Ч��Ⱦ��
 * @returns ��Ч��Ⱦ��ָ��
 */
PiRenderer *PI_API pi_fog_new();

/**
 * ������Ч��Ⱦ��
 * @param renderer ��Ч��Ⱦ��ָ��
 * @param target_name ��ȾĿ�������
 * @param view_cam_name ����Ⱦ���������
 * @param color_tex_name ������ȾĿ�����ɫ����
 * @param depth_tex_name ������ȾĿ����������
 */
void PI_API pi_fog_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *color_tex_name, char *depth_tex_name);

/************************************************************************/
/* ���𻷾�                                                                     */
/************************************************************************/
void PI_API pi_fog_deploy_env(PiRenderer* renderer, char *env_name);

void PI_API pi_fog_set_texture(PiRenderer* renderer, PiTexture* tex);

/**
 * �ͷ���Ⱦ��
 * @param renderer ��Ⱦ��ָ��
 */
void PI_API pi_fog_free(PiRenderer *renderer);

/**
 * ������Ч����ɫ
 * @param renderer ��Ⱦ��ָ��
 * @param r ��Ч����ɫ��r����
 * @param g ��Ч����ɫ��g����
 * @param b ��Ч����ɫ��b����
 */
void PI_API pi_fog_set_color(PiRenderer *renderer, float r, float g, float b);

/**
 * ������Ч��ģʽ
 * @param renderer ��Ⱦ��ָ��
 * @param fog_mode ��Ч��ģʽ��Ĭ��FM_EXP
 */
void PI_API pi_fog_set_mode(PiRenderer *renderer, FogMode mode);

/**
 * ������Ч�ľ���
 * @param renderer ��Ⱦ��ָ��
 * @param start ��Ч����Ч�Ŀ�ʼ����
 * @param end ��Ч����ȫ��Ч�Ľ�������
 */
void PI_API pi_fog_set_start_end(PiRenderer *renderer, float start, float end);

/**
 * ������Ч��Ũ��
 * @param renderer ��Ⱦ��ָ��
 * @param density ��Ч��Ũ����ֵ
 */
void PI_API pi_fog_set_density(PiRenderer *renderer, float density);

/**
 * ���û��ڷ�Χ����
 * @param renderer ��Ⱦ��ָ��
 * @param density ��Ч��Ũ����ֵ
 */
void PI_API pi_fog_set_range_enable(PiRenderer *renderer, PiBool enable);


/**
* ����������
* @param renderer ��Ⱦ��ָ��
* @param density �Ƿ�����
*/
void PI_API pi_fog_set_flow(PiRenderer *renderer, PiBool isFlow, float flow_speed, float scale_ui);

PI_END_DECLS

#endif /* INCLUDE_FOG_H */
