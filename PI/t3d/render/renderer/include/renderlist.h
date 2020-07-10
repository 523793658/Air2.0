#ifndef INCLUDE_RENDERLIST_H
#define INCLUDE_RENDERLIST_H

#include <pi_lib.h>
#include <entity.h>

/**
 * 渲染队列
 */

typedef enum 
{
	SORT_NONE = 0,	/* 不排序 */
	SORT_SOLID = 1,	/* 不透明排序 */
	SORT_TRANS = 2	/* 半透明排序 */
}SortType;

PI_BEGIN_DECLS

/* 初始化渲染队列，block是add和reset的数据对应的字节数 */
PiVector* PI_API pi_renderlist_new();

/* 清理渲染队列 */
PiBool PI_API pi_renderlist_free(PiVector *list);

/* 取默认的排序器 */
PiCompareFunc PI_API pi_renderlist_get_sort_func(SortType type);

/* 排序 */
PiBool PI_API pi_renderlist_sort(PiVector *list, PiCompareFunc func, void *user_data);

PI_END_DECLS

#endif /* INCLUDE_RENDERLIST_H */