#ifndef INCLUDE_SEQUENCE_H
#define INCLUDE_SEQUENCE_H

#include <pi_lib.h>

typedef enum
{
	BINARY_SEARCH	= 1,
	LINE_SEARCH		= 2,
	USE_CACHE		= 4,
	PRE_TREATMENT	= 8
} SearchMethod;

typedef enum
{
	EIT_SPLINE,
	EIT_LINEAR
} EInterpolateType;

typedef enum
{
	EST_FLOAT,
	EST_VECTOR_3F
} ESequenceType;

/**
 * 插值序列，前置声明
 */
typedef struct PiSequence PiSequence;

PI_BEGIN_DECLS

/* 创建 */
PiSequence *PI_API pi_sequence_create(ESequenceType type);

/* 释放 */
void PI_API pi_sequence_free(PiSequence *sequence);

/* 设置工作模式，同一序列不能重复设置 */
void PI_API pi_sequence_set_search_method(PiSequence *sequence, SearchMethod search_method);

/* 如果工作模式为预处理，需要设置预处理空间 */
void PI_API pi_sequence_set_pretreatment_length(PiSequence *sequence, uint length);

/* 如果工作模式为预处理，需要设置预处理空间 */
void* PI_API pi_sequence_get_pretreatment_data(PiSequence *sequence, uint *num);

/* 拷贝 */
PiBool PI_API pi_sequence_copy(PiSequence *dst, PiSequence *src);

/* 设置时间 */
void PI_API pi_sequence_set_time(PiSequence *sequence, float time);

/* 获取值 */
void *PI_API pi_sequence_get_value(PiSequence *sequence);

/* 设置插值方式 */
void PI_API pi_sequence_set_interpolate_type(PiSequence *sequence, EInterpolateType type);

/* 添加节点 */
uint PI_API pi_sequence_add_node(PiSequence *sequence, float time, void *value);

/* 删除节点 */
void PI_API pi_sequence_remove_node(PiSequence *sequence, uint index);

/* 设置节点的值 */
void PI_API pi_sequence_set_node_value(PiSequence *sequence, uint index, void *value);

/* 设置节点的时间 */
uint PI_API pi_sequence_set_node_time(PiSequence *sequence, uint index, float time);

/* 获取节点的value */
void *PI_API pi_sequence_get_node_value(PiSequence *sequence, uint index);

/* 获得节点的时间 */
float PI_API pi_sequence_get_node_time(PiSequence *sequence, uint index);

/* 获得节点的个数 */
uint PI_API pi_sequence_get_node_count(PiSequence *sequence);

/* 判断sequenc中value是否相同 */
PiBool PI_API pi_sequence_is_same(PiSequence *sequence);

PI_END_DECLS

#endif /* INCLUDE_SEQUENCE_H */
