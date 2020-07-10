#ifndef INCLUDE_POST_PROCESSING_H
#define INCLUDE_POST_PROCESSING_H
#include "texture.h"
#include <renderer.h>


/* ���ģʽ */
typedef enum
{
	FM_NONE = 0,
	FM_EXP,
	FM_EXP2,
	FM_LINEAR
} FogMode;

PI_BEGIN_DECLS

/**
 * ��������Ч����Ⱦ��
 * @returns ����Ч����Ⱦ��
 */
PiRenderer *PI_API pi_post_processing_new();

/**
 * �������Ч����Ⱦ��
 * @param renderer ����Ч����Ⱦ��ָ��
 * @param scene_color_name ��������������֣�������һ����Ⱦ����rt��color����
 * @param target_name ���rt������
 */
void PI_API pi_post_processing_deploy(PiRenderer *renderer, char *scene_color_name, char *scene_depth_name, char *target_name, char* view_camera_name);

/**
 * �ͷź���Ч����Ⱦ��
 * @param renderer ����Ч����Ⱦ��ָ��
 */
void PI_API pi_post_processing_free(PiRenderer *renderer);


/**
* ������Ч����ɫ
* @param renderer ��Ⱦ��ָ��
* @param r ��Ч����ɫ��r����
* @param g ��Ч����ɫ��g����
* @param b ��Ч����ɫ��b����
*/
void PI_API pi_post_processing_set_fog_color(PiRenderer *renderer, float r, float g, float b);

/**
* ������Ч��ģʽ
* @param renderer ��Ⱦ��ָ��
* @param fog_mode ��Ч��ģʽ��Ĭ��FM_EXP
*/
void PI_API pi_post_processing_set_fog_mode(PiRenderer *renderer, FogMode mode);

/**
* ������Ч�ľ���
* @param renderer ��Ⱦ��ָ��
* @param start ��Ч����Ч�Ŀ�ʼ����
* @param end ��Ч����ȫ��Ч�Ľ�������
*/
void PI_API pi_post_processing_set_fog_start_end(PiRenderer *renderer, float start, float end);

/**
* ������Ч��Ũ��
* @param renderer ��Ⱦ��ָ��
* @param density ��Ч��Ũ����ֵ
*/
void PI_API pi_post_processing_set_fog_density(PiRenderer *renderer, float density);

/**
* ���û��ڷ�Χ����
* @param renderer ��Ⱦ��ָ��
* @param density ��Ч��Ũ����ֵ
*/
void PI_API pi_post_processing_set_fog_range_enable(PiRenderer *renderer, PiBool enable);

/**
* ����������
* @param renderer ��Ⱦ��ָ��
* @param density �Ƿ�����
*/
void PI_API pi_post_processing_set_fog_flow(PiRenderer *renderer, PiBool isFlow, float flow_speed, float scale_ui);

/*
	���ÿ���ݿ���
*/
void PI_API pi_post_processing_set_fxaa_enable(PiRenderer* renderer, PiBool enable);

void PI_API pi_post_processing_set_color_grading_enable(PiRenderer* renderer, PiBool enable);

void PI_API pi_post_processing_set_color_grading_clut(PiRenderer* renderer, PiTexture* texture);

void PI_API pi_post_processing_set_bloom_params(PiRenderer* renderer, float threshold, float scale);

void PI_API pi_post_processing_set_bloom_enable(PiRenderer* renderer, PiBool enable);

PI_END_DECLS

#endif /* INCLUDE_POST_PROCESSING_H */
