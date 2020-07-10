#ifndef INCLUDE_SSAO_H
#define INCLUDE_SSAO_H

#include <renderer.h>

/**
 * SSAO(HDAO)��Ⱦ��
 */

const static char* RS_SSAO_VS = "default.vs";
const static char* RS_SSAO_FS = "ssao.fs";

PI_BEGIN_DECLS

/**
 * ������Ⱦ��
 * @returns ��Ⱦ��ָ��
 */
PiRenderer* PI_API pi_ssao_new();

/**
 * ������Ⱦ��
 * @param renderer ��Ⱦ��ָ��
 * @param depth_map_name Դ�������������
 * @param output_name ���Ŀ������ע�⣺����Ⱦ��������independent_outputģʽ�´�Ŀ������Ⱦ���Լ����������ͬ������
 * @param scene_camera_name ���������
 */
void PI_API pi_ssao_deploy(PiRenderer* renderer, char* depth_map_name, char* output_name, char* scene_camera_name);

/**
 * ������Ⱦ�����߹��ܣ����з�����Ϣ������£�����Ҳ���Բ���AOЧ���������������Ŀ���
 * @param renderer ��Ⱦ��ָ��
 * @param normal_map_name ��������������
 */
void PI_API pi_ssao_deploy_normal(PiRenderer* renderer, char* normal_map_name);

/**
 * �ͷ���Ⱦ��
 * @param renderer ��Ⱦ��ָ��
 */
void PI_API pi_ssao_free(PiRenderer* renderer);

/**
 * �������ܾ��뾶��Ӱ��AO������Χ
 * @param renderer ��Ⱦ��ָ��
 * @param radius �뾶��С��Ĭ��Ϊ0.8
 */
void PI_API pi_ssao_set_reject_radius(PiRenderer* renderer, float radius);

/**
 * ������С���ܰ뾶������������Խ�ƽ�������ϲ�����ϣ����AO
 * @param renderer ��Ⱦ��ָ��
 * @param radius �뾶��С��Ĭ��Ϊ0.0003
 */
void PI_API pi_ssao_set_accept_radius(PiRenderer* renderer, float radius);

/**
 * ���÷������ţ���ǿ��������߲���AO��������ֻ�������÷��ߵ��������Ч
 * @param renderer ��Ⱦ��ָ��
 * @param scale ����ֵ��Ĭ��Ϊ1
 */
void PI_API pi_ssao_set_normal_scale(PiRenderer* renderer, float scale);

/**
 * ����AOǿ��
 * @param renderer ��Ⱦ��ָ��
 * @param intensity AOǿ�ȣ�Ĭ��Ϊ2
 */
void PI_API pi_ssao_set_intensity(PiRenderer* renderer, float intensity);

/**
 * ����AO����Ĳ����뾶���ţ�һ���̶��ϵ���AOӰ�췶Χ
 * ע�⣺���ȵ��ڴ�����ܻ�����൱��覴�
 * @param renderer ��Ⱦ��ָ��
 * @param scale �����뾶����ֵ��Ĭ��Ϊ1
 */
void PI_API pi_ssao_set_sample_radius_scale(PiRenderer* renderer, float scale);

/**
 * ����AOƷ�ʣ�Ч���ɵ͵���ʹ�ü���1~4
 * @param renderer ��Ⱦ��ָ��
 * @param level Ч������Ĭ��Ϊ4
 */
void PI_API pi_ssao_set_quality(PiRenderer* renderer, uint level);

/**
 * ������Ⱦ��ʹ�ö������ģʽ��������AOЧ���ᱻ�����������Ⱦ���Լ�������RT�в���������ʹ
 * �ò���ʱ��output_name�������ˮ��(Alpha8��ʽ)��������Ⱦ��ֱ�ӽ�AO���ʹ��ColorMult���
 * ��output_nameָ����RT��
 * ע�⣺�����ø��ı�������ˮ�߳�ʼ��֮ǰ��Ч
 * @param renderer ��Ⱦ��ָ��
 * @param is_enable �Ƿ����ã�Ĭ��ΪFalse
 */
void PI_API pi_ssao_set_independent_output(PiRenderer* renderer, PiBool is_enable);

/**
 * ������Ⱦ��ʹ�ð�ֱ�������AO�������ڽ���Ч����ͬʱ���������Ч�ʣ�ֻ�����ö������ģʽʱ��Ч
 * �����ģʽ��ʹ�ø������Թ��˶Դ�AO�������Ӧ�ûس�������
 * ����ʹ�ò����output_name�������ˮ��
 * ע�⣺�����ø��ı�������ˮ�߳�ʼ��֮ǰ��Ч
 * @param renderer ��Ⱦ��ָ��
 * @param is_enable �Ƿ����ã�Ĭ��ΪFalse
 */
void PI_API pi_ssao_set_half_resolution_mode(PiRenderer* renderer, PiBool is_enable);

PI_END_DECLS

#endif /* INCLUDE_SSAO_H */