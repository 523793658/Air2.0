#include "pi_lib.h"
#include "renderview.h"
#include "fbo_copy.h"
#include "renderpipeline_ext.h"
#include "renderer.h"
#include "border_extension_renderer.h"
#include "rendersystem.h"
#include "gaussian_blur.h"
#include "fxaa.h"

#include "post_process_manager.h"


PostProcessManager* PI_API pi_light_map_post_process_manager_create(GaussianBlurLevel level)
{
	PostProcessManager* mgr = pi_new0(PostProcessManager, 1);
	char* target0 = "temp_target";
	char* target1 = "temp_target2";
	int i;
	mgr->gaussianBlurLevel = level < GBL_UP ? level : GBL_4;
	mgr->pipeline = pi_renderpipeline_ext_new();
	mgr->temp_target = pi_rendertarget_new(TT_MRT, TRUE);
	mgr->temp_texture = pi_texture_2d_create(RF_ABGR16F, TU_COLOR, 1, 1, 1024, 1024, TRUE);
	mgr->temp_view = pi_renderview_new_tex2d(RVT_COLOR, mgr->temp_texture, 0, 0, TRUE);
	pi_rendertarget_attach(mgr->temp_target, ATT_COLOR0, mgr->temp_view);

	mgr->temp_target2 = pi_rendertarget_new(TT_MRT, TRUE);
	mgr->temp_texture2 = pi_texture_2d_create(RF_ABGR16F, TU_COLOR, 1, 1, 1024, 1024, TRUE);
	mgr->temp_view2 = pi_renderview_new_tex2d(RVT_COLOR, mgr->temp_texture2, 0, 0, TRUE);
	pi_rendertarget_attach(mgr->temp_target2, ATT_COLOR0, mgr->temp_view2);

	mgr->input_target = pi_rendertarget_new(TT_MRT, TRUE);
	mgr->output_target = pi_rendertarget_new(TT_MRT, TRUE);


	mgr->border_extension_renderer = pi_border_extension_renderer_new();
	pi_border_extension_renderer_deploy(mgr->border_extension_renderer, "input_target", "temp_target");
	pi_renderpipeline_ext_add(mgr->pipeline, mgr->border_extension_renderer);
	target0 = "temp_target";
	target1 = "temp_target2";
	for (i = 0; i < mgr->gaussianBlurLevel; i++)
	{
		char* temp;
		mgr->blur_renderers[i] = pi_gaussian_blur_renderer_new();
		pi_renderpipeline_ext_add(mgr->pipeline, mgr->blur_renderers[i]);
		pi_gaussian_blur_renderer_deploy(mgr->blur_renderers[i], target0, target1);
		temp = target0;
		target0 = target1;
		target1 = temp;
	}

	mgr->fbo_copy_renderer = pi_fbo_copy_new();
	pi_fbo_copy_deploy(mgr->fbo_copy_renderer, target0, "output_target");
	pi_fbo_copy_light_map(mgr->fbo_copy_renderer, TRUE);
	pi_renderpipeline_ext_add(mgr->pipeline, mgr->fbo_copy_renderer);

	pi_renderpipeline_ext_add_global_resource(mgr->pipeline, "temp_target", mgr->temp_target);
	pi_renderpipeline_ext_add_global_resource(mgr->pipeline, "temp_texture", mgr->temp_texture);
	pi_renderpipeline_ext_add_global_resource(mgr->pipeline, "temp_target2", mgr->temp_target2);
	pi_renderpipeline_ext_add_global_resource(mgr->pipeline, "temp_texture2", mgr->temp_texture2);
	pi_renderpipeline_ext_add_global_resource(mgr->pipeline, "input_target", mgr->input_target);
	pi_renderpipeline_ext_add_global_resource(mgr->pipeline, "output_target", mgr->output_target);

	return mgr;
}

void PI_API pi_light_map_post_process_manager_set_input(PostProcessManager* mgr, PiVector* view_list)
{
	mgr->view_list = view_list;
}

PiVector* PI_API pi_light_map_post_process_mananger_begin(PostProcessManager* mgr)
{
	uint i, size;
	PiRenderView* inputView;
	PiTexture* texture;
	PiTexture* outputTexture;
	PiRenderView* outputView;
	PiVector* outputList = pi_vector_new();
	size = pi_vector_size(mgr->view_list);
	for (i = 0; i < size; i++)
	{
		inputView = pi_vector_get(mgr->view_list, i);
		texture = inputView->data.tex_2d.tex;
		outputTexture = pi_texture_2d_create(RF_A8, TU_COLOR, 1, 1, texture->width, texture->height, TRUE);
		outputView = pi_renderview_new_tex2d(RVT_COLOR, outputTexture, 0, 0, TRUE);


		pi_rendertarget_attach(mgr->input_target, ATT_COLOR0, inputView);

		pi_rendertarget_attach(mgr->output_target, ATT_COLOR0, outputView);

		if (texture->width != mgr->temp_texture->width || texture->height != mgr->temp_texture->height)
		{
			pi_renderview_free(mgr->temp_view);
			pi_texture_free(mgr->temp_texture);

			pi_renderview_free(mgr->temp_view2);
			pi_texture_free(mgr->temp_texture2);

			mgr->temp_texture = pi_texture_2d_create(RF_ABGR16F, TU_COLOR, 1, 1, texture->width, texture->height, TRUE);
			pi_renderpipeline_ext_remove_global_resource(mgr->pipeline, "temp_texture");
			pi_renderpipeline_ext_add_global_resource(mgr->pipeline, "temp_texture", mgr->temp_texture);
			mgr->temp_view = pi_renderview_new_tex2d(RVT_COLOR, mgr->temp_texture, 0, 0, TRUE);
			pi_rendertarget_attach(mgr->temp_target, ATT_COLOR0, mgr->temp_view);
			pi_rendertarget_set_viewport(mgr->temp_target, 0, 0, texture->width, texture->height);


			mgr->temp_texture2 = pi_texture_2d_create(RF_ABGR16F, TU_COLOR, 1, 1, texture->width, texture->height, TRUE);
			pi_renderpipeline_ext_remove_global_resource(mgr->pipeline, "temp_texture2");
			pi_renderpipeline_ext_add_global_resource(mgr->pipeline, "temp_texture2", mgr->temp_texture2);
			mgr->temp_view2 = pi_renderview_new_tex2d(RVT_COLOR, mgr->temp_texture2, 0, 0, TRUE);
			pi_rendertarget_attach(mgr->temp_target2, ATT_COLOR0, mgr->temp_view2);
			pi_rendertarget_set_viewport(mgr->temp_target2, 0, 0, texture->width, texture->height);
		}


		pi_renderpipeline_ext_draw(mgr->pipeline, 0.1f);
		pi_rendersystem_swapbuffer();
		pi_vector_push(outputList, outputTexture);
		pi_renderview_free(outputView);
	}
	return outputList;
}

void PI_API pi_light_map_post_process_manager_free(PostProcessManager* mgr)
{
	int i;
	pi_renderpipeline_ext_remove_global_resource(mgr->pipeline, "temp_target");
	pi_renderpipeline_ext_remove_global_resource(mgr->pipeline, "temp_texture");
	pi_renderpipeline_ext_remove_global_resource(mgr->pipeline, "temp_target2");
	pi_renderpipeline_ext_remove_global_resource(mgr->pipeline, "temp_texture2");
	pi_renderpipeline_ext_remove_global_resource(mgr->pipeline, "input_target");
	pi_renderpipeline_ext_remove_global_resource(mgr->pipeline, "output_target");
	pi_border_extension_renderer_free(mgr->border_extension_renderer);
	pi_rendertarget_free(mgr->output_target);
	pi_rendertarget_free(mgr->input_target);

	for (i = 0; i < mgr->gaussianBlurLevel; i++)
	{
		pi_gaussian_blur_renderer_free(mgr->blur_renderers[i]);
	}

	pi_renderview_free(mgr->temp_view2);
	pi_texture_free(mgr->temp_texture2);
	pi_rendertarget_free(mgr->temp_target2);

	pi_renderview_free(mgr->temp_view);
	pi_texture_free(mgr->temp_texture);
	pi_rendertarget_free(mgr->temp_target);
	pi_renderpipeline_ext_free(mgr->pipeline);
	pi_free(mgr);
}