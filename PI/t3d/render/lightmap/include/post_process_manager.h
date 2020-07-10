#ifndef _Include_Border_Extension_Manager_H_
#define _Include_Border_Extension_Manager_H_
#include "renderer.h"
#include "renderpipeline_ext.h"

typedef enum
{
	GBL_0 = 0,
	GBL_1,
	GBL_2,
	GBL_3,
	GBL_4,
	GBL_UP
}GaussianBlurLevel;

typedef struct
{

	PiVector* view_list;
	PiRenderer* border_extension_renderer;

	PiRenderer* blur_renderers[4];
	PiRenderer* fbo_copy_renderer;

	PiRenderPipelineExt* pipeline;
	PiRenderTarget* temp_target;
	PiTexture* temp_texture;
	PiRenderView* temp_view;

	PiRenderTarget* temp_target2;
	PiTexture* temp_texture2;
	PiRenderView* temp_view2;

	PiRenderTarget* input_target;
	PiRenderTarget* output_target;

	GaussianBlurLevel gaussianBlurLevel;

}PostProcessManager;

PI_BEGIN_DECLS

PostProcessManager* PI_API pi_light_map_post_process_manager_create(GaussianBlurLevel level);

void PI_API pi_light_map_post_process_manager_set_input(PostProcessManager* mgr, PiVector* view_list);

PiVector* PI_API pi_light_map_post_process_mananger_begin(PostProcessManager* mgr);

void PI_API pi_light_map_post_process_manager_free(PostProcessManager* mgr);

PI_END_DECLS
#endif