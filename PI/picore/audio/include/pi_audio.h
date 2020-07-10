
#ifndef INCLUDE_PI_AUDIO_H
#define INCLUDE_PI_AUDIO_H

#include <pi_lib.h>
#include <pi_vector3.h>
#include "audio_decoder.h"

typedef enum
{
	DAM_NONE,							/* 不做距离衰减 */
	DAM_INVERSE_DISTANCE,				/* 倒数衰减模式 */
	DAM_INVERSE_DISTANCE_CLAMPED,		/* 倒数衰减模式，距离会被clamp到min/max范围 */
	DAM_LINEAR_DISTANCE,				/* 线性衰减模式 */
	DAM_LINEAR_DISTANCE_CLAMPED,		/* 倒数衰减模式，距离会被clamp到min/max范围 */
	DAM_EXPONENT_DISTANCE,				/* 指数衰减模式 */
	DAM_EXPONENT_DISTANCE_CLAMPED		/* 倒数衰减模式，距离会被clamp到min/max范围 */
} DistanceAttenuationModel;				/* 距离衰减模式 */

typedef enum
{
	ASIR_OK,							/* 成功 */
	ASIR_DEVICE_FAILED,					/* 设备创建失败 */
	ASIR_CONTEXT_FAILED,				/* 上下文创建失败 */
	ASIR_LIBMPG123_FAILED,				/* mpg123 library 初始化失败 */
} AudioSystemInitRet;					/* 音频系统初始化返回结果 */

typedef struct PiAudio PiAudio;

PI_BEGIN_DECLS

/**
 * 初始化音频系统
 * @returns 是否成功
 */
AudioSystemInitRet PI_API pi_audio_context_init();

/**
 * 关闭音频系统，释放各种context
 * @returns 是否成功
 */
void PI_API pi_audio_context_free();

/**
 * 创建声源
 * @param format 音频格式
 * @returns 声源对象指针
 */
PiAudio *PI_API pi_audio_create();

/**
 * 释放声源
 * @param audio 声源对象指针
 * @param is_free 是否释放原始音频数据
 * @returns 是否成功
 */
PiBool PI_API pi_audio_delete(PiAudio *audio, PiBool is_free);

/**
* 设置音频数据
* @param audio 声源对象指针
* @param 波形数据
* @returns 被移除的音频数据指针
*/
PiBool PI_API pi_audio_set_data(PiAudio *audio, AudioWaveData* data);

/**
 * 移除声源对象中最先加入的音频数据
 * @param audio 声源对象指针
 * @param size 返回被移除的音频数据的字节数
 * @returns 被移除的音频数据指针
 */
void *PI_API pi_audio_pop(PiAudio *audio, uint *size);

/**
 * 播放声源
 * @param audio 声源对象指针
 * @returns 是否成功，失败表示内部出错
 */
PiBool PI_API pi_audio_play(PiAudio *audio);

/**
 * 更新声源
 * @param audio 声源对象指针
 * @returns 是否成功，失败表示结束或者内部出错
 */
PiBool PI_API pi_audio_update(PiAudio *audio);

/**
 * 获取声源的音频数据个数
 * @param audio 声源对象指针
 * @returns 音频数据个数
 */
uint PI_API pi_audio_size(PiAudio *audio);

/**
 * 获得指定索引的音频数据及长度
 * @param audio 声源对象指针
 * @param index 音频数据的索引
 * @param size 返回音频数据的字节数
 * @returns 音频数据指针
 */
void *PI_API pi_audio_get_data(PiAudio *audio, uint index, uint *size);

/**
 * 获得声源的时间长度
 * @param audio 声源对象指针
 * @returns 单位秒(s)，返回值如果是0表示audio没有确定的时长
 */
double PI_API pi_audio_get_duration(PiAudio *audio);

/**
 * 设置声源是否循环
 * @param audio 声源对象指针
 * @param is_loop 是否循环，默认FALSE
 * @returns 是否成功
 */
PiBool PI_API pi_audio_set_loop(PiAudio *audio, PiBool is_loop);

/**
 * 声源是否循环
 * @param audio 声源对象指针
 * @returns 是否循环，默认FALSE
 */
PiBool PI_API pi_audio_get_loop(PiAudio *audio);

/**
 * 设置声源的音量
 * @param audio 声源对象指针
 * @param volume 音量大小，默认1.0
 * @returns 是否成功
 */
PiBool PI_API pi_audio_set_volume(PiAudio *audio, float volume);

/**
 * 获取声源的音量
 * @param audio 声源对象指针
 * @returns 音量大小，默认1.0
 */
float PI_API pi_audio_get_volume(PiAudio *audio);

/**
 * 设置声源的位置
 * @param audio 声源对象指针
 * @param position 位置向量指针，默认{0.0, 0.0, 0.0}
 * @returns 是否成功
 */
PiBool PI_API pi_audio_set_position(PiAudio *audio, const PiVector3 *position);

/**
 * 获取声源的位置
 * @param audio 声源对象指针
 * @param position 返回位置向量指针，默认{0.0, 0.0, 0.0}
 * @returns 是否成功
 */
PiBool PI_API pi_audio_get_position(PiAudio *audio, PiVector3 *position);

/**
 * 设置声源的距离衰减相关属性
 * @param audio 声源对象指针
 * @param min_distance 最小距离，声源与监听器的距离小于最小距离时不衰减，默认1.0
 * @param max_distance 最大距离，声源与监听器的距离大于最大距离时不衰减
 * @param rolloff_factor 衰减系数，0.0表示不衰减，默认1.0
 * @returns 是否成功
 */
PiBool PI_API pi_audio_set_distance_attenuation(PiAudio *audio, float min_distance, float max_distance, float rolloff_factor);

/**
 * 设置声源的圆锥衰减
 * @param audio 声源对象指针
 * @param inner_angle 圆锥的内角
 * @param outer_angle 圆锥的外角
 * @param outer_volume 圆锥的外角音量
 * @returns 是否成功
 */
PiBool PI_API pi_audio_set_cone_attenuation(PiAudio *audio, float inner_angle, float outer_angle, float outer_volume);

/**
 * 设置声源的朝向
 * @param audio 声源对象指针
 * @param direction 声源的朝向，默认{0.0, 0.0, 0.0}，如果是默认值，则声源的夹角属性不生效
 * @returns 是否成功
 */
PiBool PI_API pi_audio_set_direction(PiAudio *audio, const PiVector3 *direction);

/**
 * 设置声源的音量范围
 * @param audio 声源对象指针
 * @param min_volume 声源音量允许的最小值，默认0.0
 * @param max_volume 声源音量允许的最大值，默认1.0
 * @returns 是否成功
 */
PiBool PI_API pi_audio_set_volume_range(PiAudio *audio, float min_volume, float max_volume);

/**
 * 设置监听器的位置
 * @param position 位置向量指针，默认{0.0, 0.0, 0.0}
 */
void PI_API pi_audio_set_listener_position(const PiVector3 *position);

/**
 * 设置监听器的位置
 * @param position 返回位置向量指针，默认{0.0, 0.0, 0.0}
 */
void PI_API pi_audio_get_listener_position(PiVector3 *position);

/**
 * 设置监听器的朝向
 * @param forward 监听器面向向量指针，比如player的面向，默认{0.0, 0.0, -1.0}
 * @param up 监听器竖直向量指针，默认{0.0, 1.0, 0.0}
 */
void PI_API pi_audio_set_listener_orientation(const PiVector3 *forward, const PiVector3 *up);

/**
 * 获取监听器的朝向
 * @param forward 返回监听器面向向量指针，比如player的面向，默认{0.0, 0.0, -1.0}
 * @param up 返回监听器竖直向量指针，默认{0.0, 1.0, 0.0}
 */
void PI_API pi_audio_get_listener_orientation(PiVector3 *forward, PiVector3 *up);

/**
 * 设置距离衰减模式
 * @param model 距离衰减模式，详见DistanceAttenuationModel的定义，默认DAM_INVERSE_DISTANCE_CLAMPED
 */
void PI_API pi_audio_set_distance_attenuation_model(DistanceAttenuationModel model);

PI_END_DECLS

#endif /* INCLUDE_PI_AUDIO_H */
