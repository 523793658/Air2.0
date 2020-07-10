#include <renderlayout.h>
#include <renderwrap.h>

PiRenderLayout* PI_API pi_renderlayout_new(void)
{
	PiRenderLayout *layout = pi_new0(PiRenderLayout, 1);
	render_layout_init(layout);
	return layout;
}

PiBool PI_API pi_renderlayout_free(PiRenderLayout *layout)
{
	if(layout != NULL)
	{
		render_layout_clear(layout);
		pi_free(layout);
	}	
	return TRUE;
}

IndexData* PI_API pi_index_data_new(uint32 num, EIndexType type, EBufferUsage usage)
{
	IndexData* indexData = pi_new0(IndexData, 1);
	indexData->num = num;
	indexData->type = type;
	indexData->usage = usage;
	return indexData;
}

void PI_API pi_index_data_init(IndexData* index_data)
{
	index_data_init(index_data);
}

void PI_API pi_index_data_free(IndexData* index_data)
{
	if (index_data->impl)
	{
		index_data_free(index_data);
	}
	pi_free(index_data);
}

void PI_API pi_index_data_update(IndexData* index_data, uint32 first_index, uint32 num, const void * data)
{
	if (index_data->impl)
	{
		index_data_update(index_data, first_index, num, data);
	}
}



VertexElement* PI_API pi_vertex_element_new(VertexSemantic semantic, EBufferUsage usage, EVertexType type, uint32 num_component, uint32 max_vertex)
{
	VertexElement* element = pi_new0(VertexElement, 1);

	element->semantic = semantic;
	element->type = type;
	element->usage = usage;
	element->size = max_vertex * pi_get_vertex_type_size(type, num_component);
	element->num = num_component;
	return element;
}
void PI_API pi_vertex_element_init(VertexElement* element)
{
	vertex_element_init(element);
}

void PI_API pi_vertex_element_free(VertexElement* element)
{
	if (element->is_init)
	{
		vertex_element_free(element->impl);
	}
	pi_free(element);
}

void PI_API pi_vertex_element_update(VertexElement* element, uint32_t first_vertex, uint32_t num_vertex, const void* data, const uint32 srcStride)
{
	if (element->is_init)
	{
		vertex_element_update(element, first_vertex, num_vertex, data, srcStride);
	}
}