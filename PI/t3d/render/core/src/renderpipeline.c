#include <renderpipeline.h>
#include <renderutil.h>

typedef struct
{
	uint num_frames;
	void* user_data;		/* �û����� */
	PiVector list;			/* ��Ⱦ�����б�Ԫ��ΪRenderBatch, Ҫ�����ε����ȼ����� */
	FullState state;		/* ȫ״̬ */
}RenderPipelineImpl;

void PI_API pi_renderpipeline_init(PiRenderPipeline *pipeline)
{
	RenderPipelineImpl *impl = (RenderPipelineImpl *)pipeline;
	pi_memset(impl, 0, sizeof(RenderPipelineImpl));
	pi_vector_init(&impl->list);
	pi_renderutil_init_fullstate(impl->state.state);
}

void PI_API pi_renderpipeline_clear(PiRenderPipeline *pipeline)
{
	RenderPipelineImpl *impl = (RenderPipelineImpl *)pipeline;
	pi_vector_clear(&impl->list, TRUE);
}

void PI_API pi_renderpipeline_set_state(PiRenderPipeline *pipeline, RenderStateType key, uint32 value)
{
	RenderPipelineImpl *impl = (RenderPipelineImpl *)pipeline;
	uint len = pi_vector_size(&impl->list);
	
	PI_USE_PARAM(len);
	
	PI_ASSERT(len == 0, "Invalid Operation, must invoke before any batch added.");
	
	impl->state.state[key] = value;
	
	/* �����ϵ�״̬��Ҫ��ʱ���� */
	pi_renderstate_set(key, value);
}

/** 
 * ����Ⱦ��ˮ���ϼ���ָ����Ⱦ����
 * ����RenderBatchģ���ڲ�ʹ��
 * ˳����룬��ͬ���ȼ����鵽�����
 */
void renderpipeline_add(PiRenderPipeline *pipeline, PiRenderBatch *batch)
{
	RenderPipelineImpl *impl = (RenderPipelineImpl *)pipeline;
	uint i = 0, len = pi_vector_size(&impl->list);
	uint priority = pi_renderbatch_get_priority(batch);
	for(i = 0; i < len; ++i)
	{
		PiRenderBatch *bs = (PiRenderBatch *)pi_vector_get(&impl->list, i);
		uint p = pi_renderbatch_get_priority(bs);
		if(priority > p)
			break;
	}
	pi_vector_insert(&impl->list, i, batch);
}

static PiBool PI_API _batch_remove(void *user_data, const void *data)
{
	return user_data == data;
}

/** 
 * ����Ⱦ��ˮ����ɾ��ָ����Ⱦ����
 * ����RenderBatchģ���ڲ�ʹ��
 */
void renderpipeline_remove(PiRenderPipeline *pipeline, PiRenderBatch *batch)
{
	RenderPipelineImpl *impl = (RenderPipelineImpl *)pipeline;
	pi_vector_remove_if(&impl->list, _batch_remove, batch);
}

void PI_API pi_renderpipeline_draw(PiRenderPipeline *pipeline)
{
	RenderPipelineImpl *impl = (RenderPipelineImpl *)pipeline;
	uint i, len = pi_vector_size(&impl->list);
	for(i = 0; i < len; ++i)
	{
		PiRenderBatch *batch = (PiRenderBatch *)pi_vector_get(&impl->list, i);
		pi_renderbatch_draw(batch);
	}
}

FullState* PI_API pi_renderpipeline_get_full_state(PiRenderPipeline *pipeline)
{
	RenderPipelineImpl *impl = (RenderPipelineImpl *)pipeline;
	return &impl->state;
}

void PI_API pi_renderpipeline_set_userdata(PiRenderPipeline *pipeline, void* data)
{
	RenderPipelineImpl *impl = (RenderPipelineImpl *)pipeline;
	impl->user_data = data;
}

void* PI_API pi_renderpipeline_get_userdata(PiRenderPipeline *pipeline)
{
	RenderPipelineImpl *impl = (RenderPipelineImpl *)pipeline;
	return impl->user_data;
}

uint PI_API pi_renderpipeline_get_frame(PiRenderPipeline *pipeline)
{
	RenderPipelineImpl *impl = (RenderPipelineImpl *)pipeline;
	return impl->num_frames;
}