
#ifndef INCLUDE_RENDERLAYOUT_H
#define INCLUDE_RENDERLAYOUT_H

#include <pi_lib.h>
#include <pi_renderdata.h>

/* 本模块的函数指针枚举 */
typedef enum
{
	RLF_INIT,
	RLF_CLEAR,
	RLF_SET_INDEX,
	RLF_SET_VERTEX,
	RLF_NUM
}RenderLayoutFunc;

/**
 * RenderLayout 渲染布局，全是多流结构
 */

typedef struct  
{
	void *impl;
}PiRenderLayout;

PI_BEGIN_DECLS

PiRenderLayout* PI_API pi_renderlayout_new(void);

PiBool PI_API pi_renderlayout_free(PiRenderLayout *layout);


VertexElement* PI_API pi_vertex_element_new(VertexSemantic semantic, EBufferUsage usage, EVertexType type, uint32 num_component, uint32 max_vertex);

void PI_API pi_vertex_element_init(VertexElement* element);

void PI_API pi_vertex_element_free(VertexElement* element);

void PI_API pi_vertex_element_update(VertexElement* element, uint32_t first_vertex, uint32_t num_vertex, const void* data, const uint32 srcStride);

IndexData* PI_API pi_index_data_new(uint32 num, EIndexType type, EBufferUsage usage);

void PI_API pi_index_data_update(IndexData* index_data, uint32 first_index, uint32 num, const void * data);

void PI_API pi_index_data_free(IndexData* index_data);

void PI_API pi_index_data_init(IndexData* index_data);

PI_END_DECLS

#endif /* INCLUDE_RENDERLAYOUT_H */