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
 * ��ֵ���У�ǰ������
 */
typedef struct PiSequence PiSequence;

PI_BEGIN_DECLS

/* ���� */
PiSequence *PI_API pi_sequence_create(ESequenceType type);

/* �ͷ� */
void PI_API pi_sequence_free(PiSequence *sequence);

/* ���ù���ģʽ��ͬһ���в����ظ����� */
void PI_API pi_sequence_set_search_method(PiSequence *sequence, SearchMethod search_method);

/* �������ģʽΪԤ������Ҫ����Ԥ����ռ� */
void PI_API pi_sequence_set_pretreatment_length(PiSequence *sequence, uint length);

/* �������ģʽΪԤ������Ҫ����Ԥ����ռ� */
void* PI_API pi_sequence_get_pretreatment_data(PiSequence *sequence, uint *num);

/* ���� */
PiBool PI_API pi_sequence_copy(PiSequence *dst, PiSequence *src);

/* ����ʱ�� */
void PI_API pi_sequence_set_time(PiSequence *sequence, float time);

/* ��ȡֵ */
void *PI_API pi_sequence_get_value(PiSequence *sequence);

/* ���ò�ֵ��ʽ */
void PI_API pi_sequence_set_interpolate_type(PiSequence *sequence, EInterpolateType type);

/* ��ӽڵ� */
uint PI_API pi_sequence_add_node(PiSequence *sequence, float time, void *value);

/* ɾ���ڵ� */
void PI_API pi_sequence_remove_node(PiSequence *sequence, uint index);

/* ���ýڵ��ֵ */
void PI_API pi_sequence_set_node_value(PiSequence *sequence, uint index, void *value);

/* ���ýڵ��ʱ�� */
uint PI_API pi_sequence_set_node_time(PiSequence *sequence, uint index, float time);

/* ��ȡ�ڵ��value */
void *PI_API pi_sequence_get_node_value(PiSequence *sequence, uint index);

/* ��ýڵ��ʱ�� */
float PI_API pi_sequence_get_node_time(PiSequence *sequence, uint index);

/* ��ýڵ�ĸ��� */
uint PI_API pi_sequence_get_node_count(PiSequence *sequence);

/* �ж�sequenc��value�Ƿ���ͬ */
PiBool PI_API pi_sequence_is_same(PiSequence *sequence);

PI_END_DECLS

#endif /* INCLUDE_SEQUENCE_H */
