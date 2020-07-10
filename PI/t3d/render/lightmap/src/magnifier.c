#include "pi_lib.h"
#include "renderpipeline_ext.h"
#include "fbo_copy.h"
#include "entity.h"
#include "magnifier.h"
#include "rendersystem.h"

Magnifier* PI_API pi_lightmap_magnifier_create()
{
	Magnifier* magnifier = pi_new0(Magnifier, 1);
	uint i, size;
	size = 512;
	for (i = 0; i < 3; i++)
	{
		magnifier->temp_texture[i] = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, size, size, TRUE);
		magnifier->temp_view[i] = pi_renderview_new_tex2d(RVT_COLOR, magnifier->temp_texture[i], 0, 0, TRUE);
		magnifier->temp_target[i] = pi_rendertarget_new(TT_MRT, TRUE);
		pi_rendertarget_attach(magnifier->temp_target[i], ATT_COLOR0, magnifier->temp_view[i]);
		size /= 2;
	}
	return magnifier;
}


void PI_API pi_lightmap_magnifier_work(Magnifier* magnifier, PiTexture* texture, PiRenderTarget* target)
{
	uint size, index, tempSize, i;
	PiRenderPipelineExt* pipeline;
	PiVector* renderer_list = pi_vector_new();
	PiRenderView* input_view;
	PiRenderTarget* input_target;
	PiRenderer* fbo;
	char inputName[256];
	char outputName[256];

	input_view = pi_renderview_new_tex2d(RVT_COLOR, texture, 0, 0, TRUE);
	input_target = pi_rendertarget_new(TT_MRT, TRUE);
	pi_rendertarget_attach(input_target, ATT_COLOR0, input_view);

	pipeline = pi_renderpipeline_ext_new();

	pi_renderpipeline_ext_add_global_resource(pipeline, "target0", input_target);

	size = max(texture->width, texture->height);

	tempSize = 512;
	index = 0;
	while (tempSize > size)
	{
		fbo = pi_fbo_copy_new();
		pi_fbo_copy_set_filter(fbo, TFO_CMP_MIN_MAG_MIP_LINEAR);
		pi_vector_push(renderer_list, fbo);
		pi_log_2_buffer(inputName, 256, "target%d", index);
		pi_log_2_buffer(outputName, 256, "target%d", index+1);
		pi_renderpipeline_ext_add_global_resource(pipeline, outputName, magnifier->temp_target[index]);
		pi_fbo_copy_deploy(fbo, inputName, outputName);
		pi_renderpipeline_ext_add(pipeline, fbo);
		tempSize /= 2;
		index++;
	}
	fbo = pi_fbo_copy_new();
	pi_vector_push(renderer_list, fbo);
	pi_fbo_copy_set_filter(fbo, TFO_CMP_MIN_MAG_MIP_LINEAR);
	pi_log_2_buffer(inputName, 256, "target%d", index);
	pi_log_2_buffer(outputName, 256, "target%d", index + 1);
	pi_fbo_copy_deploy(fbo, inputName, outputName);
	pi_renderpipeline_ext_add_global_resource(pipeline, outputName, target);
	pi_renderpipeline_ext_add(pipeline, fbo);


	pi_renderpipeline_ext_draw(pipeline, 1.0f);


	size = pi_vector_size(renderer_list);
	for (i = 0; i < size; i++)
	{
		PiRenderer* renderer = pi_vector_get(renderer_list, i);
		pi_fbo_copy_free(renderer);
	}
	pi_rendersystem_set_target(NULL);
	pi_renderview_free(input_view);
	pi_rendertarget_free(input_target);
}