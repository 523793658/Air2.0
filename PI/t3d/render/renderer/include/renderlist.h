#ifndef INCLUDE_RENDERLIST_H
#define INCLUDE_RENDERLIST_H

#include <pi_lib.h>
#include <entity.h>

/**
 * ��Ⱦ����
 */

typedef enum 
{
	SORT_NONE = 0,	/* ������ */
	SORT_SOLID = 1,	/* ��͸������ */
	SORT_TRANS = 2	/* ��͸������ */
}SortType;

PI_BEGIN_DECLS

/* ��ʼ����Ⱦ���У�block��add��reset�����ݶ�Ӧ���ֽ��� */
PiVector* PI_API pi_renderlist_new();

/* ������Ⱦ���� */
PiBool PI_API pi_renderlist_free(PiVector *list);

/* ȡĬ�ϵ������� */
PiCompareFunc PI_API pi_renderlist_get_sort_func(SortType type);

/* ���� */
PiBool PI_API pi_renderlist_sort(PiVector *list, PiCompareFunc func, void *user_data);

PI_END_DECLS

#endif /* INCLUDE_RENDERLIST_H */