
#include "selected_edge.h"
#include "rendertarget.h"
#include "rendersystem.h"

typedef struct
{
	uint width;
	uint height;
	PiCamera *camera;

	PiRenderTarget *selected_rt;

	PiVector *entity_list;
	float selected_color[4];
	float size[4];

	PiMesh *mesh;
	PiRenderMesh *rmesh;

	PiEntity *blur_quad;

	PiTexture *selected_tex;
	PiRenderView *selected_tex_view;
	PiTexture *depth;
	PiRenderView *depth_view;
	SamplerState sampler;

	PiMaterial *blur_material;

	PiBool is_depth_enable;
	PiBool is_deploy;

	/* 资源名称 */
	char *view_cam_name;
	char *src_depth_target_name;
	char *output_name;
	char *selected_entity_name;
	/* 常量字符串 */
	char *COLOR;

	char *U_objTex;
	char *U_blurTex;
	char *U_Tex;
	char *U_Scope;
	char *U_Color;
	char *U_TexSize;
} SelectedEdgeRenderer;

static void _type_check(PiRenderer	*renderer)
{
	PI_ASSERT(renderer->type == ERT_SELECTED_EDGE, "Renderer type error!");
}

static void _create_selected_renderview(SelectedEdgeRenderer *impl)
{
	impl->selected_tex = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, impl->width, impl->height, TRUE);
	impl->selected_tex_view = pi_renderview_new_tex2d(RVT_COLOR, impl->selected_tex, 0, 0, TRUE);

	pi_rendertarget_attach(impl->selected_rt, ATT_COLOR0, impl->selected_tex_view);
	pi_rendertarget_set_viewport(impl->selected_rt, 0, 0, impl->width, impl->height);

	pi_sampler_set_texture(&impl->sampler, impl->selected_tex);
	pi_material_set_uniform(impl->blur_material, impl->U_Tex, UT_SAMPLER_2D, 1, &impl->sampler, TRUE);
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	SelectedEdgeRenderer *impl = (SelectedEdgeRenderer *)renderer->impl;
	PiRenderTarget *target;

	PI_USE_PARAM(resources);

	if (!impl->is_deploy)
	{
		return FALSE;
	}

	impl->COLOR = pi_conststr("COLOR");
	impl->U_objTex = pi_conststr("u_objTex");
	impl->U_blurTex = pi_conststr("u_blurTex");
	impl->U_Tex = pi_conststr("u_Tex");
	impl->U_Color = pi_conststr("u_Color");
	impl->U_Scope = pi_conststr("u_Scope");
	impl->U_TexSize = pi_conststr("u_TexSize");

	pi_hash_lookup(resources, impl->output_name, (void **)&target);

	//TODO:此处的创建和pi_selected_edge_set_entity函数冲突,可能有内存泄露,因去掉会导致p23上层代码崩溃暂时保留
	if (!impl->entity_list)
	{
		impl->entity_list = pi_vector_new();
	}
	//init weight and height
	impl->width = target->width;
	impl->height = target->height;

	impl->size[0] = (float)target->width;
	impl->size[1] = (float)target->height;

	//create render targets
	impl->selected_rt = pi_rendertarget_new(TT_MRT, TRUE);

	impl->mesh = pi_mesh_create_quad(NULL, NULL, 0.0f);
	impl->rmesh = pi_rendermesh_new(impl->mesh, TRUE);

	//blur quad
	impl->blur_quad = pi_entity_new();
	impl->blur_material = pi_material_new(RS_SELECTED_EDGE_VS, RS_BLUR_SIMPLE_FS);
	pi_material_set_blend(impl->blur_material, TRUE);
	pi_material_set_blend_factor(impl->blur_material, BF_SRC_ALPHA, BF_INV_SRC_ALPHA, BF_ZERO, BF_ONE);
	pi_material_set_uniform_pack_flag(impl->blur_material, impl->U_Color, UT_VEC3, 1, impl->selected_color, FALSE, TRUE);
	pi_material_set_uniform_pack_flag(impl->blur_material, impl->U_TexSize, UT_VEC2, 1, impl->size, FALSE, TRUE);
	pi_entity_set_mesh(impl->blur_quad, impl->rmesh);
	pi_entity_set_material(impl->blur_quad, impl->blur_material);
	pi_spatial_set_local_scaling(impl->blur_quad->spatial, (float)impl->width, (float)impl->height, 1.0f);
	pi_spatial_update(impl->blur_quad->spatial);

	//tex
	pi_renderstate_set_default_sampler(&impl->sampler);
	pi_sampler_set_addr_mode(&impl->sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->sampler, TFO_MIN_MAG_LINEAR);

	_create_selected_renderview(impl);

	//camera
	impl->camera = pi_camera_new();
	pi_camera_set_location(impl->camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->camera, -(float)impl->width / 2.0f + 0.5f, (float)impl->width / 2.0f + 0.5f, -(float)impl->height / 2.0f - 0.5f, (float)impl->height / 2.0f - 0.5f, 0.0f, 2.0f, TRUE);

	return TRUE;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	SelectedEdgeRenderer *impl = (SelectedEdgeRenderer *)renderer->impl;
	PiRenderTarget *target;
	PiRenderTarget *depthViewTarget;
	PiColor background;
	PiCamera *view_cam;
	PiRenderView *depthView;
	uint n;
	PI_USE_PARAM(tpf);
	_type_check(renderer);

	n = pi_vector_size(impl->entity_list);

	if (n == 0)
	{
		return;
	}
	
	pi_hash_lookup(resources, impl->view_cam_name, (void **)&view_cam);
	pi_hash_lookup(resources, impl->output_name, (void **)&target);

	pi_hash_lookup(resources, impl->src_depth_target_name, (void **)&depthViewTarget);


	depthView = depthViewTarget->views[ATT_DEPTHSTENCIL];
	pi_rendertarget_detach(depthViewTarget, ATT_DEPTHSTENCIL);
	pi_rendertarget_attach(impl->selected_rt, ATT_DEPTHSTENCIL, depthView);
	pi_rendersystem_set_target(impl->selected_rt);
	color_set(&background, 0.0f, 0.0f, 0.0f, 1.0f);
	pi_rendersystem_clearview(TBM_COLOR, &background, 1.0f, 0);
	pi_rendersystem_set_camera(view_cam);
	pi_entity_draw_list(impl->entity_list);

	//对entity进行模糊
	pi_sampler_set_texture(&impl->sampler, impl->selected_tex);
	pi_material_set_uniform(impl->blur_material, impl->U_Tex, UT_SAMPLER_2D, 1, &impl->sampler, TRUE);

	pi_rendersystem_set_target(target);
	pi_rendersystem_set_camera(impl->camera);
	pi_entity_draw(impl->blur_quad);
	pi_rendertarget_detach(impl->selected_rt, ATT_DEPTHSTENCIL);
	pi_rendertarget_attach(depthViewTarget, ATT_DEPTHSTENCIL, depthView);
	return;
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	SelectedEdgeRenderer *impl = (SelectedEdgeRenderer *)renderer->impl;

	impl->width = width;
	impl->height = height;

	impl->size[0] = (float)width;
	impl->size[1] = (float)height;

	pi_rendertarget_detach(impl->selected_rt, ATT_COLOR0);
	pi_texture_free(impl->selected_tex);
	pi_renderview_free(impl->selected_tex_view);

	_create_selected_renderview(impl);
	pi_material_set_uniform_pack_flag(impl->blur_material, impl->U_TexSize, UT_VEC2, 1, impl->size, FALSE, TRUE);

	pi_camera_set_frustum(impl->camera, -(float)width / 2.0f, (float)width / 2.0f, -(float)height / 2.0f, (float)height / 2.0f, 0.0f, 2.0f, TRUE);

	pi_spatial_set_local_scaling(impl->blur_quad->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->blur_quad->spatial);
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
}

PiRenderer *PI_API pi_selected_edge_new_with_name(char* name)
{
	PiRenderer *renderer;
	SelectedEdgeRenderer *impl = pi_new0(SelectedEdgeRenderer, 1);
	impl->is_depth_enable = TRUE;
	impl->selected_color[0] = 0;
	impl->selected_color[1] = 1;
	impl->selected_color[2] = 0;

	renderer = pi_renderer_create(ERT_SELECTED_EDGE, name, _init, _resize, _update, _draw, impl);
	return renderer;
}

PiRenderer *PI_API pi_selected_edge_new()
{
	return pi_selected_edge_new_with_name("selected");
}

void PI_API pi_selected_edge_deploy(PiRenderer *renderer, char *view_cam_name, char *src_depth_target_name, char *output_name)
{
	SelectedEdgeRenderer *impl;
	_type_check(renderer);
	impl = (SelectedEdgeRenderer *)renderer->impl;

	pi_free(impl->src_depth_target_name);
	pi_free(impl->output_name);
	pi_free(impl->view_cam_name);
	impl->view_cam_name = pi_str_dup(view_cam_name);
	impl->src_depth_target_name = pi_str_dup(src_depth_target_name);
	impl->output_name = pi_str_dup(output_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_selected_edge_free(PiRenderer *renderer)
{
	SelectedEdgeRenderer *impl;
	_type_check(renderer);
	impl = (SelectedEdgeRenderer *)renderer->impl;

	pi_camera_free(impl->camera);

	pi_mesh_free(impl->mesh);
	pi_rendermesh_free(impl->rmesh);

	pi_entity_free(impl->blur_quad);

	pi_texture_free(impl->selected_tex);
	pi_renderview_free(impl->selected_tex_view);
	pi_renderview_free(impl->depth_view);

	pi_material_free(impl->blur_material);

	pi_rendertarget_free(impl->selected_rt);

	pi_free(impl->src_depth_target_name);
	pi_free(impl->output_name);
	pi_free(impl->view_cam_name);

	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}

//TODO:此函数应该修改为deploy形式
void PI_API pi_selected_edge_set_entity(PiRenderer *renderer, PiVector *entityList)
{
	SelectedEdgeRenderer *impl;
	_type_check(renderer);
	impl =  (SelectedEdgeRenderer *)renderer->impl;
	impl->entity_list = entityList;
}

void PI_API pi_selected_edge_set_color(PiRenderer *renderer, float r, float g, float b)
{
	SelectedEdgeRenderer *impl;
	_type_check(renderer);
	impl =  (SelectedEdgeRenderer *)renderer->impl;
	impl->selected_color[0] = r;
	impl->selected_color[1] = g;
	impl->selected_color[2] = b;

	if (impl->blur_material)
	{
		pi_material_set_uniform_pack_flag(impl->blur_material, impl->U_Color, UT_VEC3, 1, impl->selected_color, FALSE, TRUE);
	}
}

void PI_API pi_selected_edge_enable_depth( PiRenderer *renderer, PiBool enable )
{
	SelectedEdgeRenderer *impl;
	_type_check(renderer);
	impl = (SelectedEdgeRenderer *)renderer->impl;
	impl->is_depth_enable = enable;
}
