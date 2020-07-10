#ifndef INCLUDE_VEGETATION_ANIM_H
#define INCLUDE_VEGETATION_ANIM_H

#include <controller.h>
#include <entity.h>
#include <environment.h>
#include <pi_sphere.h>

/**
 * ֲ������������
 */

PI_BEGIN_DECLS

/**
 * ����һ��ֲ������������
 * @return ������ָ��
 */
PiController* PI_API pi_vegetation_anim_new();

/**
 * �ͷ�ָ����ֲ������������
 * @param c ��Ҫ�ͷŵĿ�����ָ��
 */
void PI_API pi_vegetation_anim_free(PiController *c);

/**
 * ���������Ĳ�����ʼ��ָ����Entity����ʼ�������޸Ķ�������������ô˷���������Ч
 * @param c ����������
 * @param entity ��ҪӦ�ò�����Entity
 */
void PI_API pi_vegetation_anim_apply_params(PiController *c, PiEntity* entity);

/**
 * ����Ӱ��˿������Ļ�������
 * @param c ����������
 * @param env ��������
 */
void PI_API pi_vegetation_anim_set_environment(PiController *c, PiEnvironment* env);

/**
 * ����Ӱ��˿����������ɶ�������,���ĺ���Ҫʹ��pi_vegetation_anim_apply_paramsʹ֮��Ч
 * @param c ����������
 * @param trunk_flexibility ����ǿ�ȣ�Ĭ��Ϊ0.1
 * @param wind_scale �������ţ�Ĭ��Ϊ1
 * @param wind_frequency �ܷ���Ӱ��ʱ����Ƶ�ʣ�Ĭ��Ϊ1
 */
void PI_API pi_vegetation_anim_set_trunk_param(PiController *c, float trunk_flexibility, float wind_scale, float wind_frequency);

/**
 * �Ƿ�����Ҷ��/֦�ɶ���,���ĺ���Ҫʹ��pi_vegetation_anim_apply_paramsʹ֮��Ч
 * @param c ����������
 * @param is_enable �Ƿ����ã�Ĭ��ΪFALSE
 */
void PI_API pi_vegetation_anim_set_leaf_anim_enable(PiController *c, PiBool is_enable);

/**
 * ����Ӱ��˿�������Ҷ��/֦�ɶ�������,ֻ���ڿ���Ҷ�Ӷ���ʱ��Ч,���ĺ���Ҫʹ��pi_vegetation_anim_apply_paramsʹ֮��Ч
 * @param c ����������
 * @param leaf_amplitude Ҷ�������Ĭ��Ϊ1
 * @param leaf_frequency Ҷ����Ƶ�ʣ�Ĭ��Ϊ1
 * @param branch_amplitude ֦�������Ĭ��Ϊ1
 * @param branch_frequency ֦����Ƶ�ʣ�Ĭ��Ϊ1
 */
void PI_API pi_vegetation_anim_set_leaf_param(PiController *c, float leaf_amplitude, float leaf_frequency, float branch_amplitude, float branch_frequency);

/**
 * ����Ӱ��˿�������Ҷ��/֦�ɶ���˥������,ֻ���ڿ���Ҷ�Ӷ���ʱ��Ч,���ĺ���Ҫʹ��pi_vegetation_anim_apply_paramsʹ֮��Ч
 * @param c ����������
 * @param vertex_color �Ƿ�ʹ�ö���ɫ���ƣ�����ʱ��2���������ý�ǿ��ΪNULL,Ĭ��Ϊfalse
 * @param leaf_tex Ҷ��˥������ͼ��NULLΪʹ���Զ����������ƣ�Ĭ��ΪNULL
 * @param branch_tex ֦��˥������ͼ��NULLΪʹ���Զ����������ƣ� Ĭ��ΪNULL
 */
void PI_API pi_vegetation_anim_set_leaf_attenuation(PiController *c, PiBool vertex_color, PiTexture* leaf_tex, PiTexture* branch_tex);

/**
 * ����ֲ�������Ķ�����λ������ͬ�����µ�ֲ�ﶯ������ȫͬ��
 * @param c ����������
 * @param phase �������ŵ���λ��Ĭ��Ϊ0
 */
void PI_API pi_vegetation_anim_set_individual_phase(PiController *c, float phase);

/**
 * ���ò�̤Ч�������ߣ�ʹֲ����������̤�ĵ���Ч��
 * @param c ����������
 * @param generator Ч��������
 * @param fall_scale ��̤Ӱ�����
 */
void PI_API pi_vegetation_anim_set_fall_generator(PiController *c, PiSphere *generator, float fall_scale);


/*
	����ֲ������������spatial
*/
void PI_API pi_vegetation_anim_set_bind_spatial(PiController *c, PiSpatial* spatial);

PI_END_DECLS

#endif /* INCLUDE_VEGETATION_ANIM_H */