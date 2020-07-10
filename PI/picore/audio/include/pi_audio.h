
#ifndef INCLUDE_PI_AUDIO_H
#define INCLUDE_PI_AUDIO_H

#include <pi_lib.h>
#include <pi_vector3.h>
#include "audio_decoder.h"

typedef enum
{
	DAM_NONE,							/* ��������˥�� */
	DAM_INVERSE_DISTANCE,				/* ����˥��ģʽ */
	DAM_INVERSE_DISTANCE_CLAMPED,		/* ����˥��ģʽ������ᱻclamp��min/max��Χ */
	DAM_LINEAR_DISTANCE,				/* ����˥��ģʽ */
	DAM_LINEAR_DISTANCE_CLAMPED,		/* ����˥��ģʽ������ᱻclamp��min/max��Χ */
	DAM_EXPONENT_DISTANCE,				/* ָ��˥��ģʽ */
	DAM_EXPONENT_DISTANCE_CLAMPED		/* ����˥��ģʽ������ᱻclamp��min/max��Χ */
} DistanceAttenuationModel;				/* ����˥��ģʽ */

typedef enum
{
	ASIR_OK,							/* �ɹ� */
	ASIR_DEVICE_FAILED,					/* �豸����ʧ�� */
	ASIR_CONTEXT_FAILED,				/* �����Ĵ���ʧ�� */
	ASIR_LIBMPG123_FAILED,				/* mpg123 library ��ʼ��ʧ�� */
} AudioSystemInitRet;					/* ��Ƶϵͳ��ʼ�����ؽ�� */

typedef struct PiAudio PiAudio;

PI_BEGIN_DECLS

/**
 * ��ʼ����Ƶϵͳ
 * @returns �Ƿ�ɹ�
 */
AudioSystemInitRet PI_API pi_audio_context_init();

/**
 * �ر���Ƶϵͳ���ͷŸ���context
 * @returns �Ƿ�ɹ�
 */
void PI_API pi_audio_context_free();

/**
 * ������Դ
 * @param format ��Ƶ��ʽ
 * @returns ��Դ����ָ��
 */
PiAudio *PI_API pi_audio_create();

/**
 * �ͷ���Դ
 * @param audio ��Դ����ָ��
 * @param is_free �Ƿ��ͷ�ԭʼ��Ƶ����
 * @returns �Ƿ�ɹ�
 */
PiBool PI_API pi_audio_delete(PiAudio *audio, PiBool is_free);

/**
* ������Ƶ����
* @param audio ��Դ����ָ��
* @param ��������
* @returns ���Ƴ�����Ƶ����ָ��
*/
PiBool PI_API pi_audio_set_data(PiAudio *audio, AudioWaveData* data);

/**
 * �Ƴ���Դ���������ȼ������Ƶ����
 * @param audio ��Դ����ָ��
 * @param size ���ر��Ƴ�����Ƶ���ݵ��ֽ���
 * @returns ���Ƴ�����Ƶ����ָ��
 */
void *PI_API pi_audio_pop(PiAudio *audio, uint *size);

/**
 * ������Դ
 * @param audio ��Դ����ָ��
 * @returns �Ƿ�ɹ���ʧ�ܱ�ʾ�ڲ�����
 */
PiBool PI_API pi_audio_play(PiAudio *audio);

/**
 * ������Դ
 * @param audio ��Դ����ָ��
 * @returns �Ƿ�ɹ���ʧ�ܱ�ʾ���������ڲ�����
 */
PiBool PI_API pi_audio_update(PiAudio *audio);

/**
 * ��ȡ��Դ����Ƶ���ݸ���
 * @param audio ��Դ����ָ��
 * @returns ��Ƶ���ݸ���
 */
uint PI_API pi_audio_size(PiAudio *audio);

/**
 * ���ָ����������Ƶ���ݼ�����
 * @param audio ��Դ����ָ��
 * @param index ��Ƶ���ݵ�����
 * @param size ������Ƶ���ݵ��ֽ���
 * @returns ��Ƶ����ָ��
 */
void *PI_API pi_audio_get_data(PiAudio *audio, uint index, uint *size);

/**
 * �����Դ��ʱ�䳤��
 * @param audio ��Դ����ָ��
 * @returns ��λ��(s)������ֵ�����0��ʾaudioû��ȷ����ʱ��
 */
double PI_API pi_audio_get_duration(PiAudio *audio);

/**
 * ������Դ�Ƿ�ѭ��
 * @param audio ��Դ����ָ��
 * @param is_loop �Ƿ�ѭ����Ĭ��FALSE
 * @returns �Ƿ�ɹ�
 */
PiBool PI_API pi_audio_set_loop(PiAudio *audio, PiBool is_loop);

/**
 * ��Դ�Ƿ�ѭ��
 * @param audio ��Դ����ָ��
 * @returns �Ƿ�ѭ����Ĭ��FALSE
 */
PiBool PI_API pi_audio_get_loop(PiAudio *audio);

/**
 * ������Դ������
 * @param audio ��Դ����ָ��
 * @param volume ������С��Ĭ��1.0
 * @returns �Ƿ�ɹ�
 */
PiBool PI_API pi_audio_set_volume(PiAudio *audio, float volume);

/**
 * ��ȡ��Դ������
 * @param audio ��Դ����ָ��
 * @returns ������С��Ĭ��1.0
 */
float PI_API pi_audio_get_volume(PiAudio *audio);

/**
 * ������Դ��λ��
 * @param audio ��Դ����ָ��
 * @param position λ������ָ�룬Ĭ��{0.0, 0.0, 0.0}
 * @returns �Ƿ�ɹ�
 */
PiBool PI_API pi_audio_set_position(PiAudio *audio, const PiVector3 *position);

/**
 * ��ȡ��Դ��λ��
 * @param audio ��Դ����ָ��
 * @param position ����λ������ָ�룬Ĭ��{0.0, 0.0, 0.0}
 * @returns �Ƿ�ɹ�
 */
PiBool PI_API pi_audio_get_position(PiAudio *audio, PiVector3 *position);

/**
 * ������Դ�ľ���˥���������
 * @param audio ��Դ����ָ��
 * @param min_distance ��С���룬��Դ��������ľ���С����С����ʱ��˥����Ĭ��1.0
 * @param max_distance �����룬��Դ��������ľ������������ʱ��˥��
 * @param rolloff_factor ˥��ϵ����0.0��ʾ��˥����Ĭ��1.0
 * @returns �Ƿ�ɹ�
 */
PiBool PI_API pi_audio_set_distance_attenuation(PiAudio *audio, float min_distance, float max_distance, float rolloff_factor);

/**
 * ������Դ��Բ׶˥��
 * @param audio ��Դ����ָ��
 * @param inner_angle Բ׶���ڽ�
 * @param outer_angle Բ׶�����
 * @param outer_volume Բ׶���������
 * @returns �Ƿ�ɹ�
 */
PiBool PI_API pi_audio_set_cone_attenuation(PiAudio *audio, float inner_angle, float outer_angle, float outer_volume);

/**
 * ������Դ�ĳ���
 * @param audio ��Դ����ָ��
 * @param direction ��Դ�ĳ���Ĭ��{0.0, 0.0, 0.0}�������Ĭ��ֵ������Դ�ļн����Բ���Ч
 * @returns �Ƿ�ɹ�
 */
PiBool PI_API pi_audio_set_direction(PiAudio *audio, const PiVector3 *direction);

/**
 * ������Դ��������Χ
 * @param audio ��Դ����ָ��
 * @param min_volume ��Դ�����������Сֵ��Ĭ��0.0
 * @param max_volume ��Դ������������ֵ��Ĭ��1.0
 * @returns �Ƿ�ɹ�
 */
PiBool PI_API pi_audio_set_volume_range(PiAudio *audio, float min_volume, float max_volume);

/**
 * ���ü�������λ��
 * @param position λ������ָ�룬Ĭ��{0.0, 0.0, 0.0}
 */
void PI_API pi_audio_set_listener_position(const PiVector3 *position);

/**
 * ���ü�������λ��
 * @param position ����λ������ָ�룬Ĭ��{0.0, 0.0, 0.0}
 */
void PI_API pi_audio_get_listener_position(PiVector3 *position);

/**
 * ���ü������ĳ���
 * @param forward ��������������ָ�룬����player������Ĭ��{0.0, 0.0, -1.0}
 * @param up ��������ֱ����ָ�룬Ĭ��{0.0, 1.0, 0.0}
 */
void PI_API pi_audio_set_listener_orientation(const PiVector3 *forward, const PiVector3 *up);

/**
 * ��ȡ�������ĳ���
 * @param forward ���ؼ�������������ָ�룬����player������Ĭ��{0.0, 0.0, -1.0}
 * @param up ���ؼ�������ֱ����ָ�룬Ĭ��{0.0, 1.0, 0.0}
 */
void PI_API pi_audio_get_listener_orientation(PiVector3 *forward, PiVector3 *up);

/**
 * ���þ���˥��ģʽ
 * @param model ����˥��ģʽ�����DistanceAttenuationModel�Ķ��壬Ĭ��DAM_INVERSE_DISTANCE_CLAMPED
 */
void PI_API pi_audio_set_distance_attenuation_model(DistanceAttenuationModel model);

PI_END_DECLS

#endif /* INCLUDE_PI_AUDIO_H */
