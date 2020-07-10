#ifndef INCLUDE_PRESHADOW_PCF_H
#define INCLUDE_PRESHADOW_PCF_H

#include <renderer.h>
#include <camera.h>
#include <texture.h>

/**
 * PCF ShadowMap��Ⱦ��
 */

/**
 * PCF �⻬��Ե��������
 */
typedef enum
{
	PCF_SS_HARD = 0,
	PCF_SS_8X,
	PCF_SS_16X,
	PCF_SS_32X,
	PCF_SS_48X,
	PCF_SS_64X,
} PCFShadowSamples;

/**
 * Lighting ������Ⱦ����������
 */
typedef struct PCFShadowPipelineData
{
	PiMatrix4 shadow_matrix;
	PiTexture *depth_stencil;
	PiTexture *shadow_map;
	PCFShadowSamples shadow_samples;
	float shadow_z_far;
	float sizeInv;
} PCFShadowPipelineData;

PI_BEGIN_DECLS

PiRenderer *PI_API pi_preshadow_pcf_new_with_name(char *name);

/**
 * ������Ⱦ��
 * @returns ��Ⱦ��ָ��
 */
PiRenderer *PI_API pi_preshadow_pcf_new();

/**
 * ������Ⱦ��
 * @param renderer ��Ⱦ��ָ��
 * @param shadow_data_name ����Ӱ��Ⱦ���������Ӱ������,���ɹ�����Ⱦ����Ӱ����ӿڴ���
 * @param view_cam_name ���������
 * @param shadow_cam_name ��Ӱ�����
 * @param entity_list_name ��ӰͶ���������б�,����Shader����Ϊmodel_pl.vs/preshadow_pcf.fs
 * @param env_name ����������
 */
void PI_API pi_preshadow_pcf_deploy(PiRenderer *renderer, char *shadow_data_name, char *view_cam_name, char *shadow_cam_name, char *entity_list_name, char *env_name);

/**
 * �ͷ���Ⱦ��
 * @param renderer ��Ⱦ��ָ��
 */
void PI_API pi_preshadow_pcf_free(PiRenderer *renderer);

/**
 * ���»�ú�ָ���������ƥ�����Ӱ���
 * ע��: �˺������������̵߳����Ը������
 * @param renderer ��Ⱦ��ָ��
 * @param view_cam �������ָ��
 * @param shadow_cam ��Ӱ���ָ��
 */
void PI_API pi_preshadow_pcf_update_camera(PiRenderer *renderer, PiCamera *view_cam, PiCamera *shadow_cam);

/**
 * ������Ӱͼ�ߴ�,Խ����Ӱ��ϸ��Խ��
 * @param renderer ��Ⱦ��ָ��
 * @param size ��Ӱͼ�ߴ�, Ĭ��1024
 */
void PI_API pi_preshadow_pcf_set_shadow_mapsize(PiRenderer *renderer, uint size);

/**
 * ������Զ�ɲ�����Ӱ�ķ�Χ(����ռ䵥λ)
 * @param renderer ��Ⱦ��ָ��
 * @param z_far ��Ӱ����
 */
void PI_API pi_preshadow_pcf_set_zfar(PiRenderer *renderer, float z_far);

/**
 * ����PCF��Ӱ����
 * @param renderer ��Ⱦ��ָ��
 * @param samples ��ѡ�Ĳ�������,����Խ����ȾЧ��Խ��
 */
void PI_API pi_preshadow_pcf_set_quality(PiRenderer *renderer, PCFShadowSamples samples);

/**
 * ����PCF��Ӱ���˷�Χ, Խ����Ӱ�İ�Ӱ��ΧԽ��
 * @param renderer ��Ⱦ��ָ��
 * @param size ��������˷�Χ, һ�㽨��0~0.01, Ĭ��0.0055
 */
void PI_API pi_preshadow_pcf_set_filter_size(PiRenderer *renderer, float size);

PI_END_DECLS

#endif /* INCLUDE_PRESHADOW_PCF_H */
