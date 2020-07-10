#ifndef INCLUDE_PRESHADOW_VSM_H
#define INCLUDE_PRESHADOW_VSM_H

#include <renderer.h>
#include <camera.h>
#include <texture.h>

/**
 * VSM ShadowMap��Ⱦ��
 */


const static char* RS_BOX_BLUR_VS = "default.vs";
const static char* RS_BOX_BLUR_FS = "boxblur.fs";

/**
 * Lighting ������Ⱦ����������
 */
typedef struct VSMShadowPipelineData
{
	PiMatrix4 shadow_matrix;
	PiTexture* shadow_map;
	float shadow_z_far;
} VSMShadowPipelineData;

PI_BEGIN_DECLS

/**
 * ������Ⱦ��
 * @returns ��Ⱦ��ָ��
 */
PiRenderer* PI_API pi_preshadow_vsm_new();

/**
 * ������Ⱦ��
 * @param renderer ��Ⱦ��ָ��
 * @param shadow_data_name ����Ӱ��Ⱦ���������Ӱ������,���ɹ�����Ⱦ����Ӱ����ӿڴ���
 * @param view_cam_name ���������
 * @param shadow_cam_name ��Ӱ�����
 * @param entity_list_name ��ӰͶ���������б�,����Shader����Ϊmodel_pl.vs/preshadow_pcf.fs
 * @param env_name ����������
 */
void PI_API pi_preshadow_vsm_deploy(PiRenderer* renderer, char* shadow_data_name, char* view_cam_name, char* shadow_cam_name, char* entity_list_name, char *env_name);

/**
 * �ͷ���Ⱦ��
 * @param renderer ��Ⱦ��ָ��
 */
void PI_API pi_preshadow_vsm_free(PiRenderer* renderer);

/**
 * ���»�ú�ָ���������ƥ�����Ӱ���
 * ע��: �˺������������̵߳����Ը������
 * @param renderer ��Ⱦ��ָ��
 * @param view_cam �������ָ��
 * @param shadow_cam ��Ӱ���ָ��
 */
void PI_API pi_preshadow_vsm_update_camera(PiRenderer* renderer, PiCamera* view_cam, PiCamera* shadow_cam);

/**
 * ������Ӱͼ�ߴ�,Խ����Ӱ��ϸ��Խ��
 * @param renderer ��Ⱦ��ָ��
 * @param size ��Ӱͼ�ߴ�, Ĭ��1024
 */
void PI_API pi_preshadow_vsm_set_shadow_mapsize(PiRenderer* renderer, uint size);

/**
 * ������Զ�ɲ�����Ӱ�ķ�Χ(����ռ䵥λ)
 * @param renderer ��Ⱦ��ָ��
 * @param z_far ��Ӱ����
 */
void PI_API pi_preshadow_vsm_set_zfar(PiRenderer* renderer, float z_far);

/**
 * ����VSM��Ӱͼģ������, ����Խ����Ӱ��ԵԽƽ��,��Ч��Խ��
 * @param renderer ��Ⱦ��ָ��
 * @param num ģ������, Ĭ��3
 */
void PI_API pi_preshadow_vsm_set_blur_pass(PiRenderer* renderer, uint num);

PI_END_DECLS

#endif /* INCLUDE_PRESHADOW_VSM_H */