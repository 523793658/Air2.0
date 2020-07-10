
#include "pi_sequence.h"
#include <pi_vector3.h>

#define DEFAULT_PRETREATMENT_LENGTH 0

/* 线性插值函数 */
typedef void(*LinearFunc) (void *out, void *data0, void *data1, float weight);

/* 三次样条插值函数 */
typedef void(*SplineFunc) (void *out, void *data0, void *data1, void *slope0, void *slope1, float time1, float time2, float weight);

/* 三次样条插值函数 */
typedef uint(*SearchFunc) (void *data, void *user_data);

typedef struct
{
	float time;
	void *value;
} SequenceElement;

/* 预处理时后用到的数据 */
typedef struct
{
	void *array;
	uint arrary_length;
	float start_time;
	float end_time;
	float time_interval;
} Pretreatment;

/**
 * 插值序列
 */
struct PiSequence
{
	PiVector elements;					/* 元素数组 */
	uint length;						/* 元素的个数 */
	uint value_size;					/* 每个value的长度 */

	ESequenceType sequence_type;		/* value格式 */
	EInterpolateType interpolate_type;	/* 插值方式 */
	SearchMethod search_method;			/* 查找方式 */

	float sampling_time;				/* 插值的时间 */
	void *sampling_data;				/* 对应的值 */
	void *pretreatment_data;

	uint index;
	PiBool is_same;						/* 用来记录节点值是否一样 */

	LinearFunc linear_func;				/* 线性插值函数 */
	SplineFunc spline_func;				/* 样条插值函数 */
	SearchFunc search_func;				/* 查找函数 */

	Pretreatment pretreatment;
};

typedef struct
{
	uint size;
	PiVector *vector;
} VectorCopy;

typedef struct
{
	float time;
	uint index;
} SearchDate;

static PiBool _is_same(PiVector *list, uint value_size)
{
	uint i, j, size;

	size = pi_vector_size(list);
	for (i = 0; i < size - 1; ++i)
	{
		SequenceElement *el = (SequenceElement *)pi_vector_get(list, i);

		for (j = i + 1; j < size; ++j)
		{
			SequenceElement *el2 = (SequenceElement *)pi_vector_get(list, j);
			if (pi_memcmp_inline(el, el2, value_size))
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

static void _set_time(PiSequence *sequence, SearchFunc search_func, float time)
{
	SequenceElement *data1 = NULL;
	SequenceElement *data2 = NULL;
	SequenceElement *data3 = NULL;
	SequenceElement *data4 = NULL;
	SequenceElement *el = NULL;

	uint i = 0;
	float frac;
	float time1;
	float time2;
	uint length = pi_vector_size(&sequence->elements);
	SearchDate search_data;

	search_data.time = time;
	search_data.index = sequence->index;
	sequence->sampling_time = time;

	i = search_func(&sequence->elements, &search_data);
	sequence->index =  i == length ? i - 1 : i;
	if (i == length)
	{
		el = (SequenceElement *)pi_vector_get(&sequence->elements, i - 1);
	}
	else
	{
		el = (SequenceElement *)pi_vector_get(&sequence->elements, i);
	}
	if (i == 0 || i == length)
	{
		data1 = el;
		data2 = el;
		data3 = el;
		data4 = el;
	}
	else
	{
		data1 = (SequenceElement *)pi_vector_get(&sequence->elements, i - 1);
		data2 = el;

		/* 用于样条插值时候使用 */
		data3 = (i - 1) == 0 ? data1 : (SequenceElement *)pi_vector_get(&sequence->elements, i - 2);
		data4 = i == length - 1 ? data2 : (SequenceElement *)pi_vector_get(&sequence->elements, i + 1);
	}
	if (data1->time == data2->time)
	{
		frac = 0;
	}
	else
	{
		frac = (time - data1->time) / (data2->time - data1->time);
	}
	if (sequence->interpolate_type == EIT_LINEAR)
	{
		sequence->linear_func(sequence->sampling_data, data1->value, data2->value, frac);
	}
	else if (sequence->interpolate_type == EIT_SPLINE)
	{
		time1 = data2->time - data3->time;
		time2 = data4->time - data1->time;
		sequence->spline_func(sequence->sampling_data, data1->value, data2->value, data3->value, data4->value, time1, time2, frac);
	}
}

/* 标准二分查找 */
static uint _binary_search(void *data, void *user_data)
{
	uint low = 0;
	PiVector *list = (PiVector *)data;
	float time = ((SearchDate *)user_data)->time;
	uint count = pi_vector_size(list);
	SequenceElement *el = NULL;
	while (count > 0)
	{
		uint count2 = count / 2;
		uint mid = low + count2;
		el = (SequenceElement *)pi_vector_get(list, mid);

		/* 二分查找一般不会第一次直接命中故将命中放在后面 */
		if (time > el->time)
		{
			/* 找到的大于中间元素，在后半段 */
			low = ++mid;
			count -= (count2 + 1);
		}
		else if (time < el->time)
		{
			/* 找到的小于中间元素，在前半段 */
			count = count2;
		}
		else
		{
			/* 找到了返回索引 */
			return mid;
		}
	}
	return low;
}

/* 使用缓存的二分查找 */
static uint _binary_cache_search(void *data, void *user_data)
{
	uint low = 0;
	PiVector *list = (PiVector *)data;
	float time = ((SearchDate *)user_data)->time;
	uint index = ((SearchDate *)user_data)->index;
	uint count = pi_vector_size(list);
	SequenceElement *el = NULL;
	while (count > 0)
	{
		uint count2 = index;
		uint mid = low + count2;
		el = (SequenceElement *)pi_vector_get(list, mid);

		/* 二分查找一般不会第一次直接命中故将命中放在后面 */
		if (time > el->time)
		{
			/* 找到的大于中间元素，在后半段 */
			low = ++mid;
			count -= (count2 + 1);
		}
		else if (time < el->time)
		{
			/* 找到的小于中间元素，在前半段 */
			count = count2;
		}
		else
		{
			/* 找到了返回索引 */
			return mid;
		}
	}
	return low;
}

/* 线性查找 */
static uint _line_search(void *data, void *user_data)
{
	PiVector *list = (PiVector *)data;
	float time = ((SearchDate *)user_data)->time;
	uint count = pi_vector_size(list);
	SequenceElement *el = NULL;
	uint i;
	for (i = 0; i < count; ++i)
	{
		el = (SequenceElement *)pi_vector_get(list, i);
		if (el->time >= time)
		{
			return i;
		}
	}
	return i;
}

/* 使用缓存的线性查找 */
static uint _line_cache_search(void *data, void *user_data)
{
	PiVector *list = (PiVector *)data;
	float time = ((SearchDate *)user_data)->time;
	uint index = ((SearchDate *)user_data)->index;
	uint count = pi_vector_size(list);
	SequenceElement *el = (SequenceElement *)pi_vector_get(list, index);
	uint i;
	sint j;
	/* 从指定处开始向后查找 */
	if (time > el->time)
	{
		for (i = index + 1; i < count; ++i)
		{
			el = (SequenceElement *)pi_vector_get(list, i);
			if (el->time >= time)
			{
				return i;
			}
		}
		return i;
	}
	/* 从指定处开始向前查找 */
	j = (sint)index - 1;
	for (; j >= 0; --j)
	{
		el = (SequenceElement *)pi_vector_get(list, j);
		if (el->time < time)
		{
			return (j + 1);
		}
	}
	return 0;
}

/* 浮点样条插值 */
static void _float_spline_func(void *out, void *data0, void *data1, void *slope0, void *slope1, float time1, float time2, float weight)
{
	float d0 = *(float *)data0;
	float d1 = *(float *)data1;
	float d2 = *(float *)slope0;
	float d3 = *(float *)slope1;
	float w1 = weight;
	float w2 = w1 * w1;
	float w3 = w2 * w1;
	if (data1 == data0)
	{
		*(float *)out = d0;
		return;
	}
	/* time1和time2为m所占的比例一般为6，不同的样条插值该值不同，对于Catmull—Rom固定为6 */
	time1 = 6.0f;
	time2 = 6.0f;
	d2 = d0 + (d1 - d2) / time1;
	d3 = d1 - (d3 - d0) / time2;
	*(float *)out = d0 + 3 * w1 * (d2 - d0) + 3 * w2 * (d0 - 2 * d2  + d3) + w3 * (3 * d2 - d0 - 3 * d3 + d1);
}

/* 三维向量样条插值 */
static void _vector3_spline_func(void *out, void *data0, void *data1, void *slope0, void *slope1, float time1, float time2, float weight)
{
	PiVector3 *o  = (PiVector3 *)out;
	PiVector3 *d0 = (PiVector3 *)data0;
	PiVector3 *d1 = (PiVector3 *)data1;
	PiVector3 *d2 = (PiVector3 *)slope0;
	PiVector3 *d3 = (PiVector3 *)slope1;
	_float_spline_func(&o->x, &d0->x, &d1->x, &d2->x, &d3->x, time1, time2, weight);
	_float_spline_func(&o->y, &d0->y, &d1->y, &d2->y, &d3->y, time1, time2, weight);
	_float_spline_func(&o->z, &d0->z, &d1->z, &d2->z, &d3->z, time1, time2, weight);
}

/* 用于浮点数的线性插值函数 */
static void _float_linear_func(void *out, void *data0, void *data1, float weight)
{
	*(float *)out = pi_lerp_float( *(float *)data0, *(float *)data1, weight);
}

/* 用于向量的线性插值函数 */
static void _vector3_linear_func(void *out, void *data0, void *data1, float weight)
{
	pi_vec3_lerp((PiVector3 *)out, (PiVector3 *)data0, (PiVector3 *)data1, weight);
}

static PiSelectR PI_API _foreach_vector(void *user_data, void *value)
{
	SequenceElement *el = (SequenceElement *)value;
	PI_USE_PARAM(user_data);
	pi_free(el->value);
	pi_free(value);
	return SELECT_NEXT;
}

static PiSelectR PI_API _copy_vector(void *user_data, void *value)
{
	SequenceElement *el = (SequenceElement *)value;
	SequenceElement *new_el = (SequenceElement *)pi_malloc0(sizeof(SequenceElement));
	VectorCopy *copy_struct = (VectorCopy *)user_data;
	new_el->time = el->time;
	new_el->value = pi_malloc0(copy_struct->size);
	pi_memcpy_inline(new_el->value, el->value, copy_struct->size);
	pi_vector_push(copy_struct->vector, new_el);
	return SELECT_NEXT;
}

static void _generate_pretreatment(PiSequence *sequence)
{
	float time;
	uint i = 0;
	SequenceElement *el = NULL;
	SequenceElement *el_end = NULL;
	el = (SequenceElement *)pi_vector_get(&sequence->elements, 0);
	if (sequence->length == 0)
	{
		return;
	}
	if (sequence->length == 1)
	{
		sequence->pretreatment.start_time = el->time;
		sequence->pretreatment.end_time = el->time;
		sequence->pretreatment.time_interval = 1.0f;
		for (i = 0; i < sequence->pretreatment.arrary_length; ++i)
		{
			pi_memcpy_inline((char *)sequence->pretreatment.array + i * sequence->value_size, el->value, sequence->value_size);
		}
		return;
	}

	pi_memcpy_inline(sequence->pretreatment.array, el->value, sequence->value_size);

	el_end = (SequenceElement *)pi_vector_get(&sequence->elements, sequence->length - 1);
	sequence->pretreatment.start_time = el->time;
	sequence->pretreatment.end_time = el_end->time;
	sequence->pretreatment.time_interval = (el_end->time - el->time) / (sequence->pretreatment.arrary_length - 1);
	for (i = 1; i < sequence->pretreatment.arrary_length; ++i)
	{
		time = i * sequence->pretreatment.time_interval + sequence->pretreatment.start_time;
		_set_time(sequence, _binary_search, time);
		pi_memcpy_inline((char *)sequence->pretreatment.array + i * sequence->value_size, sequence->sampling_data, sequence->value_size);
	}
}

PiSequence *PI_API pi_sequence_create(ESequenceType type)
{
	PiSequence *sequence = pi_new0(PiSequence, 1);
	sequence->sequence_type = type;

	switch (type)
	{
	case EST_FLOAT:
		sequence->value_size = sizeof(float);
		sequence->linear_func = _float_linear_func;
		sequence->spline_func = _float_spline_func;
		break;
	case EST_VECTOR_3F:
		sequence->value_size = sizeof(PiVector3);
		sequence->linear_func = _vector3_linear_func;
		sequence->spline_func = _vector3_spline_func;
		break;
	default:
		break;
	}
	/* 查找方法，默认标准二分查找 */
	sequence->search_func = _binary_search;
	sequence->sampling_data = pi_malloc0(sequence->value_size);
	sequence->interpolate_type = EIT_LINEAR;
	sequence->is_same = TRUE;
	sequence->pretreatment.arrary_length = DEFAULT_PRETREATMENT_LENGTH;
	/* 此处需要实现sequence->spline_func */
	//sequence->spline_func = NULL;
	return sequence;
}

void PI_API pi_sequence_free(PiSequence *sequence)
{
	pi_vector_foreach(&sequence->elements, _foreach_vector, NULL);
	pi_vector_clear(&sequence->elements, TRUE);
	if (sequence->search_method & PRE_TREATMENT)
	{
		pi_free(sequence->pretreatment.array);
	}
	pi_free(sequence->sampling_data);
	pi_free(sequence);
}

/* 设置查找方法 */
void PI_API pi_sequence_set_search_method(PiSequence *sequence, SearchMethod search_method)
{
	if (search_method & BINARY_SEARCH && search_method & USE_CACHE)
	{
		sequence->search_func = _binary_cache_search;
	}
	else if (search_method & BINARY_SEARCH)
	{
		sequence->search_func = _binary_search;
	}
	else if (search_method & LINE_SEARCH && search_method & USE_CACHE)
	{
		sequence->search_func = _line_cache_search;
	}
	else if (search_method & LINE_SEARCH)
	{
		sequence->search_func = _line_search;
	}
	else
	{
		/* 默认使用标准二分查找 */
		sequence->search_func = _binary_search;
	}
	sequence->search_method = search_method;
}

/* 拷贝，先释放后拷贝 */
PiBool PI_API pi_sequence_copy(PiSequence *dst, PiSequence *src)
{
	VectorCopy user_data;
	if (dst->sequence_type != src->sequence_type)
	{
		return FALSE;
	}

	/* 释放dst原数据 */
	pi_vector_foreach(&dst->elements, _foreach_vector, NULL);
	pi_vector_clear(&dst->elements, TRUE);

	if (dst->pretreatment.array)
	{
		pi_free(dst->pretreatment.array);
		dst->pretreatment.array = NULL;
	}
	pi_free(dst->sampling_data);

	/* 重新写dst */
	pi_memcpy_inline(dst, src, sizeof(PiSequence));
	if (src->search_method & PRE_TREATMENT)
	{
		dst->pretreatment.array = pi_malloc0(dst->pretreatment.arrary_length * dst->value_size);

		pi_memcpy_inline(dst->pretreatment.array, src->pretreatment.array, dst->value_size * dst->pretreatment.arrary_length);
	}
	pi_vector_init(&dst->elements);

	dst->sampling_data = pi_malloc0(dst->value_size);
	pi_memcpy_inline(dst->sampling_data, src->sampling_data, dst->value_size);

	if (dst->search_method & PRE_TREATMENT)
	{
		dst->pretreatment_data = (char *)dst->pretreatment.array + ((uintptr_t)src->pretreatment_data - (uintptr_t)src->pretreatment.array);
	}
	user_data.vector = &dst->elements;
	user_data.size = dst->value_size;
	pi_vector_foreach(&src->elements, _copy_vector, &user_data);
	return TRUE;
}

/* 一般情况下time值是变大的，如果采用缓存加二分的方法，效率有提高的 */
void PI_API pi_sequence_set_time(PiSequence *sequence, float time)
{
	if (sequence->search_method & PRE_TREATMENT)
	{
		Pretreatment *pretreatment = &sequence->pretreatment;
		uint index;
		time = CLAMP(time, pretreatment->start_time, pretreatment->end_time);
		index = (uint)((time - pretreatment->start_time) / pretreatment->time_interval);
		sequence->pretreatment_data = (char *)sequence->pretreatment.array + sequence->value_size * index;
		return;
	}
	_set_time(sequence, sequence->search_func, time);
}

void *PI_API pi_sequence_get_value(PiSequence *sequence)
{
	if (sequence->length == 0)
	{
		return NULL;
	}
	if (sequence->search_method & PRE_TREATMENT)
	{
		return sequence->pretreatment_data;
	}
	return sequence->sampling_data;
}

void PI_API pi_sequence_set_interpolate_type(PiSequence *sequence, EInterpolateType type)
{
	sequence->interpolate_type = type;
}

uint PI_API pi_sequence_add_node(PiSequence *sequence, float time, void *value)
{
	PiBool is_equal = FALSE;
	uint i = 0;
	SequenceElement *el = NULL;
	SequenceElement *insert_element = NULL;
	uint length = pi_vector_size(&sequence->elements);

	for (i = 0; i < length; ++i)
	{
		el = (SequenceElement *)pi_vector_get(&sequence->elements, i);
		if (time < el->time)
		{
			break;
		}
		if (time == el->time)
		{
			is_equal = TRUE;
			break;
		}
	}
	if (sequence->is_same && el)
	{
		if (pi_memcmp_inline(el->value, value, sequence->value_size))
		{
			sequence->is_same = FALSE;
		}
	}
	if (is_equal)
	{
		pi_memcpy_inline(el->value, value, sequence->value_size);
		return i;
	}
	insert_element = pi_new0(SequenceElement, 1);
	insert_element->time = time;
	insert_element->value = pi_malloc0(sequence->value_size);
	pi_memcpy_inline(insert_element->value, value, sequence->value_size);
	pi_vector_insert(&sequence->elements, i, insert_element);
	++sequence->length;
	if (sequence->search_method & PRE_TREATMENT)
	{
		_generate_pretreatment(sequence);
	}
	pi_sequence_set_time(sequence, sequence->sampling_time);

	return i;
}

/* 删除节点 */
void PI_API pi_sequence_remove_node(PiSequence *sequence, uint index)
{
	SequenceElement *el = (SequenceElement *)pi_vector_remove(&sequence->elements, index);
	pi_free(el->value);
	pi_free(el);
	sequence->length--;
	if (sequence->search_method & PRE_TREATMENT)
	{
		_generate_pretreatment(sequence);
	}
	pi_sequence_set_time(sequence, sequence->sampling_time);
	if (!sequence->is_same)
	{
		sequence->is_same = _is_same(&sequence->elements, sequence->value_size);
	}
}

/* 设置节点的值 */
void PI_API pi_sequence_set_node_value(PiSequence *sequence, uint index, void *value)
{
	SequenceElement *el = (SequenceElement *)pi_vector_get(&sequence->elements, index);
	pi_memcpy_inline(el->value, value, sequence->value_size);
	if (sequence->search_method & PRE_TREATMENT)
	{
		_generate_pretreatment(sequence);
	}
	pi_sequence_set_time(sequence, sequence->sampling_time);
	if (sequence->is_same)
	{
		sequence->is_same = FALSE;
	}
	else
	{
		sequence->is_same = _is_same(&sequence->elements, sequence->value_size);
	}
}

/* 设置节点的时间 */
uint PI_API pi_sequence_set_node_time(PiSequence *sequence, uint index, float time)
{
	sint i = sequence->length - 1;
	SequenceElement *el = NULL;
	SequenceElement *new_el = (SequenceElement *)pi_vector_remove(&sequence->elements, index);
	//pi_memcpy_inline(&new_el, el ,sizeof(SequenceElement));
	new_el->time = time;
	//el = (SequenceElement *)pi_vector_remove(&sequence->elements, index);
	while (i > 0)
	{
		el = (SequenceElement *)pi_vector_get(&sequence->elements, i);
		if (time < el->time)
		{
			pi_vector_insert(&sequence->elements, i, new_el);
			return i;
		}
		--i;
	}
	pi_vector_insert(&sequence->elements, 0, new_el);
	if (sequence->search_method & PRE_TREATMENT)
	{
		_generate_pretreatment(sequence);
	}
	pi_sequence_set_time(sequence, sequence->sampling_time);
	return 0;
}

/* 获取节点的value */
void *PI_API pi_sequence_get_node_value(PiSequence *sequence, uint index)
{
	SequenceElement *el = (SequenceElement *)pi_vector_get(&sequence->elements, index);
	return el->value;
}

/* 获得节点的时间 */
float PI_API pi_sequence_get_node_time(PiSequence *sequence, uint index)
{
	SequenceElement *el = (SequenceElement *)pi_vector_get(&sequence->elements, index);
	return el->time;
}

/* 获得节点的个数 */
uint PI_API pi_sequence_get_node_count(PiSequence *sequence)
{
	return sequence->length;
}

/* 设置预处理精度 */
void PI_API pi_sequence_set_pretreatment_length(PiSequence *sequence, uint length)
{
	if (sequence->search_method & PRE_TREATMENT)
	{
		if (sequence->pretreatment.array)
		{
			pi_free(sequence->pretreatment.array);
			sequence->pretreatment.array = NULL;
		}
		sequence->pretreatment.arrary_length = length;
		sequence->pretreatment.array = pi_malloc0(sequence->value_size * length);
		if (sequence->length > 0)
		{
			_generate_pretreatment(sequence);
		}
	}
}

void* PI_API pi_sequence_get_pretreatment_data(PiSequence *sequence, uint *num)
{
	if (sequence->search_method & PRE_TREATMENT)
	{
		*num = sequence->pretreatment.arrary_length;
		return sequence->pretreatment.array;
	}
	else
	{
		*num = 0;
		return NULL;
	}
}

PiBool PI_API pi_sequence_is_same(PiSequence *sequence)
{
	return sequence->is_same;
}

