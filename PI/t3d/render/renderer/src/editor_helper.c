#include "editor_helper.h"

#include <pi_spatial.h>
#include <entity.h>
#include <rendertarget.h>
#include <rendersystem.h>

/**
 * editor helper renderer
 */

#define MAX_SHAPE_CIRCLE_NUM 10
#define SHAPE_CIRCLE_SIZE 3
#define MAX_SHAPE_RECTANGLE_NUM 10
#define SHAPE_RECTANGLE_SIZE 6
#define MAX_SHAPE_SECTOR_NUM 10
#define SHAPE_SECTOR_SIZE 6

typedef struct
{
	PiBool is_deploy;

	char *scene_color_name;
	char *scene_depth_name;
	char *output_name;
	char *scene_camera_name;

	PiMesh *mesh;
	PiRenderMesh *rmesh;
	SamplerState sampler;

	PiEntity *post_quad;
	PiMaterial *post_material;
	PiCamera *post_camera;

	PiBool enable;

	PiBool grid_enable;
	float grid_snap;
	float grid_line_width;
	float grid_factor;
	PiBool color_mode;

	PiBool brush_enable;
	float brush_pos[2];
	float brush_size;

	PiBool draw_shape_enable;
	uint32 circle_num;
	float circle_pos[MAX_SHAPE_CIRCLE_NUM][2];
	float circle_radius[MAX_SHAPE_CIRCLE_NUM];
	uint32 rectangle_num;
	float rectangle_pos[MAX_SHAPE_RECTANGLE_NUM][2];
	float rectangle_rotation_mat[MAX_SHAPE_RECTANGLE_NUM][3][3];
	float rectangle_width[MAX_SHAPE_RECTANGLE_NUM];
	float rectangle_height[MAX_SHAPE_RECTANGLE_NUM];
	uint32 sector_num;
	float sector_pos[MAX_SHAPE_CIRCLE_NUM][2];
	float sector_dir[MAX_SHAPE_CIRCLE_NUM][2];
	float sector_radius[MAX_SHAPE_CIRCLE_NUM];
	float sector_angle[MAX_SHAPE_CIRCLE_NUM];

	PiBool projection_enable;
	PiTexture *projection_texture;
	float projection_origin[2];
	float projection_size[2];
	float projection_factor;

	PiRenderTarget *texture_merge_rt;
	PiEntity *texture_merge_quad;
	PiMaterial *texture_merge_material;
	PiCamera *texture_merge_camera;
	SamplerState texture_merge_color_map_sampler;
	SamplerState texture_merge_alpha_sampler;
	PiRenderView *color_map_view;
	PiRenderView *alpha_map_view;
	PiMesh *reverse_mesh;
	PiRenderMesh *reverse_rmesh;

	/* ×Ö·û´®³£Á¿ */
	char *ENABLE;
	char *GRID_ENABLE;
	char *BRUSH_ENABLE;
	char *PROJECTION_ENABLE;
	char *COLOR_MODE;
	char *DRAW_SHAPE_ENABLE;
	char *DRAW_CIRCLE_ENABLE;
	char *DRAW_RECTANGLE_ENABLE;
	char *DRAW_SECTOR_ENABLE;

	char *U_GridSnap;
	char *U_GridLineWidth;
	char *U_GridFactor;
	char *U_ColorTex;
	char *U_DepthTex;
	char *U_ViewProjMatrixInverse;
	char *U_BrushPos;
	char *U_BrushSize;
	char *U_ProjectionTexture;
	char *U_ProjectionOrigin;
	char *U_ProjectionSize;
	char *U_ProjectionFactor;
	char *U_Scale_0;
	char *U_Scale_1;
	char *U_Scale_2;
	char *U_Scale_3;
	char *U_Color_0;
	char *U_Color_1;
	char *U_Color_2;
	char *U_Color_3;
	char *U_Alpha_0;
	char *U_Alpha_1;
	char *U_Alpha_2;
	char *U_Alpha_3;
	char *U_ShapeCircleNum;
	char *U_ShapeCirclePos;
	char *U_ShapeCircleRadius;
	char *U_ShapeRectangleNum;
	char *U_ShapeRectanglePos;
	char *U_ShapeRectangleRotationMat;
	char *U_ShapeRectangleWidth;
	char *U_ShapeRectangleHeight;
	char *U_ShapeSectorNum;
	char *U_ShapeSectorPos;
	char *U_ShapeSectorDir;

	char *U_ShapeSectorRadius;
	char *U_ShapeSectorAngle;
} EditorHelperRenderer;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT((renderer)->type == ERT_EDITOR_HELPER, "Renderer type error!");
}

static void _set_brush_data_to_material(EditorHelperRenderer *impl)
{
	if(impl->brush_enable)
	{
		pi_material_set_def(impl->post_material, impl->BRUSH_ENABLE, TRUE);
		pi_material_set_uniform(impl->post_material, impl->U_BrushPos, UT_VEC2, 1, &impl->brush_pos, TRUE);
		pi_material_set_uniform(impl->post_material, impl->U_BrushSize, UT_FLOAT, 1, &impl->brush_size, TRUE);
	}
	else
	{
		pi_material_set_def(impl->post_material, impl->BRUSH_ENABLE, FALSE);
	}
}

static void _set_projection_data_to_material(EditorHelperRenderer *impl)
{
	if(impl->projection_enable)
	{
		pi_material_set_def(impl->post_material, impl->PROJECTION_ENABLE, TRUE);
		pi_sampler_set_texture(&impl->sampler, impl->projection_texture);
		pi_material_set_uniform(impl->post_material, impl->U_ProjectionTexture, UT_SAMPLER_2D, 1, &impl->sampler, TRUE);
		pi_material_set_uniform(impl->post_material, impl->U_ProjectionOrigin, UT_VEC2, 1, &impl->projection_origin, TRUE);
		pi_material_set_uniform(impl->post_material, impl->U_ProjectionSize, UT_VEC2, 1, &impl->projection_size, TRUE);
		pi_material_set_uniform(impl->post_material, impl->U_ProjectionFactor, UT_FLOAT, 1, &impl->projection_factor, TRUE);
	}
	else
	{
		pi_material_set_def(impl->post_material, impl->PROJECTION_ENABLE, FALSE);
	}
}

static void _set_grid_data_to_material(EditorHelperRenderer *impl)
{
	pi_material_set_uniform(impl->post_material, impl->U_GridSnap, UT_FLOAT, 1, &impl->grid_snap, TRUE);
	pi_material_set_uniform(impl->post_material, impl->U_GridLineWidth, UT_FLOAT, 1, &impl->grid_line_width, TRUE);
	pi_material_set_uniform(impl->post_material, impl->U_GridFactor, UT_FLOAT, 1, &impl->grid_factor, TRUE);
	pi_material_set_def(impl->post_material, impl->COLOR_MODE, impl->color_mode);
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	PiRenderTarget *target;
	uint32 width, height;
	EditorHelperRenderer *impl = (EditorHelperRenderer *)renderer->impl;

	impl->ENABLE = pi_conststr("ENABLE");
	impl->GRID_ENABLE = pi_conststr("GRID_ENABLE");
	impl->BRUSH_ENABLE = pi_conststr("BRUSH_ENABLE");
	impl->PROJECTION_ENABLE = pi_conststr("PROJECTION_ENABLE");
	impl->COLOR_MODE = pi_conststr("INVERT");
	impl->DRAW_SHAPE_ENABLE = pi_conststr("DRAW_SHAPE_ENABLE");
	impl->DRAW_CIRCLE_ENABLE = pi_conststr("DRAW_CIRCLE_ENABLE");
	impl->DRAW_RECTANGLE_ENABLE = pi_conststr("DRAW_RECTANGLE_ENABLE");
	impl->DRAW_SECTOR_ENABLE = pi_conststr("DRAW_SECTOR_ENABLE");
	impl->U_GridSnap = pi_conststr("u_GridSnap");
	impl->U_GridLineWidth = pi_conststr("u_GridLineWidth");
	impl->U_GridFactor = pi_conststr("u_GridFactor");
	impl->U_ColorTex = pi_conststr("u_ColorTex");
	impl->U_DepthTex = pi_conststr("u_DepthTex");
	impl->U_ViewProjMatrixInverse = pi_conststr("u_ViewProjMatrixInverse");
	impl->U_BrushPos = pi_conststr("u_BrushPos");
	impl->U_BrushSize = pi_conststr("u_BrushSize");
	impl->U_ProjectionTexture = pi_conststr("u_ProjectionTexture");
	impl->U_ProjectionOrigin = pi_conststr("u_ProjectionOrigin");
	impl->U_ProjectionSize = pi_conststr("u_ProjectionSize");
	impl->U_ProjectionFactor = pi_conststr("u_ProjectionFactor");
	impl->U_Scale_0 = pi_conststr("u_Scale_0");
	impl->U_Scale_1 = pi_conststr("u_Scale_1");
	impl->U_Scale_2 = pi_conststr("u_Scale_2");
	impl->U_Scale_3 = pi_conststr("u_Scale_3");
	impl->U_Color_0 = pi_conststr("u_Color_0");
	impl->U_Color_1 = pi_conststr("u_Color_1");
	impl->U_Color_2 = pi_conststr("u_Color_2");
	impl->U_Color_3 = pi_conststr("u_Color_3");
	impl->U_Alpha_0 = pi_conststr("u_Alpha_0");
	impl->U_Alpha_1 = pi_conststr("u_Alpha_1");
	impl->U_Alpha_2 = pi_conststr("u_Alpha_2");
	impl->U_Alpha_3 = pi_conststr("u_Alpha_3");
	impl->U_ShapeCircleNum = pi_conststr("u_ShapeCircleNum");
	impl->U_ShapeCirclePos = pi_conststr("u_ShapeCirclePos");
	impl->U_ShapeCircleRadius = pi_conststr("u_ShapeCircleRadius");
	impl->U_ShapeRectangleNum = pi_conststr("u_ShapeRectangleNum");
	impl->U_ShapeRectanglePos = pi_conststr("u_ShapeRectanglePos");
	impl->U_ShapeRectangleRotationMat = pi_conststr("u_ShapeRectangleRotationMat");
	impl->U_ShapeRectangleWidth = pi_conststr("u_ShapeRectangleWidth");
	impl->U_ShapeRectangleHeight = pi_conststr("u_ShapeRectangleHeight");
	impl->U_ShapeSectorNum = pi_conststr("u_ShapeSectorNum");
	impl->U_ShapeSectorPos = pi_conststr("u_ShapeSectorPos");
	impl->U_ShapeSectorDir = pi_conststr("u_ShapeSectorDir");
	impl->U_ShapeSectorRadius = pi_conststr("u_ShapeSectorRadius");
	impl->U_ShapeSectorAngle = pi_conststr("u_ShapeSectorAngle");

	if(!impl->is_deploy)
	{
		return FALSE;
	}

	impl->post_quad = pi_entity_new();

	pi_hash_lookup(resources, impl->output_name, (void **)&target);
	width = target->width;
	height = target->height;

	impl->post_material = pi_material_new(RS_EDITOR_HELPER_VS, RS_EDITOR_HELPER_FS);
	
	pi_material_set_depth_enable(impl->post_material, FALSE);
	pi_material_set_depthwrite_enable(impl->post_material, FALSE);

	pi_renderstate_set_default_sampler(&impl->sampler);
	pi_sampler_set_filter(&impl->sampler, TFO_MIN_MAG_POINT);
	pi_sampler_set_addr_mode(&impl->sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);

	pi_entity_set_material(impl->post_quad, impl->post_material);

	impl->mesh = pi_mesh_create_quad(NULL, NULL, 0.0f);
	impl->rmesh = pi_rendermesh_new(impl->mesh, TRUE);
	pi_entity_set_mesh(impl->post_quad, impl->rmesh);

	pi_spatial_set_local_scaling(impl->post_quad->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->post_quad->spatial);

	impl->post_camera = pi_camera_new();
	pi_camera_set_location(impl->post_camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->post_camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->post_camera, -(float)width / 2.0f + 0.5f, width / 2.0f + 0.5f, -(float)height / 2.0f - 0.5f, (float)height / 2.0f - 0.5f, 0.0f, 2.0f, TRUE);

	if(impl->enable)
	{
		pi_material_set_def(impl->post_material, impl->ENABLE, TRUE);

		_set_brush_data_to_material(impl);

		_set_projection_data_to_material(impl);

		if(impl->grid_enable)
		{
			pi_material_set_def(impl->post_material, impl->GRID_ENABLE, TRUE);
			_set_grid_data_to_material(impl);
		}
	}

	{
		float tcoord[4 * 2] =
		{
			0.0f, 1.0f,
			1.0f, 1.0f,
			1.0f, 0.0f,
			0.0f, 0.0f
		};
		impl->reverse_mesh = pi_mesh_create_quad(NULL, tcoord, 0.0f);
		impl->reverse_rmesh = pi_rendermesh_new(impl->reverse_mesh, TRUE);
	}

	impl->texture_merge_quad = pi_entity_new();
	pi_entity_set_mesh(impl->texture_merge_quad, impl->reverse_rmesh);

	impl->texture_merge_material = pi_material_new(RS_EDITOR_HELPER_VS, RS_TEXTURE_MERGE_FS);
	
	pi_entity_set_material(impl->texture_merge_quad, impl->texture_merge_material);

	impl->texture_merge_camera = pi_camera_new();
	pi_camera_set_location(impl->texture_merge_camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->texture_merge_camera, 0.0f, 0.0f, -1.0f);

	impl->texture_merge_rt = pi_rendertarget_new(TT_MRT, TRUE);

	pi_renderstate_set_default_sampler(&impl->texture_merge_color_map_sampler);
	pi_sampler_set_filter(&impl->texture_merge_color_map_sampler, TFO_MIN_MAG_MIP_LINEAR);
	pi_sampler_set_addr_mode(&impl->texture_merge_color_map_sampler, TAM_WRAP, TAM_WRAP, TAM_WRAP);

	pi_renderstate_set_default_sampler(&impl->texture_merge_alpha_sampler);
	pi_sampler_set_addr_mode(&impl->texture_merge_alpha_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);

	return TRUE;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	EditorHelperRenderer *impl = (EditorHelperRenderer *)renderer->impl;
	PiRenderTarget *target;
	PI_USE_PARAM(tpf);
	_type_check(renderer);
	pi_hash_lookup(resources, impl->output_name, (void **)&target);
	pi_rendersystem_set_target(target);

	pi_rendersystem_set_camera(impl->post_camera);

	pi_entity_draw(impl->post_quad);
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	PiTexture *color_tex;
	PiTexture *depth_tex;

	EditorHelperRenderer *impl;
	PiCamera *scene_camera;
	PiMatrix4 view_proj_mat_inverse;
	PI_USE_PARAM(tpf);
	impl = (EditorHelperRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->scene_color_name, (void **)&color_tex);
	pi_hash_lookup(resources, impl->scene_depth_name, (void **)&depth_tex);
	pi_renderstate_set_default_sampler(&impl->sampler);

	pi_sampler_set_texture(&impl->sampler, color_tex);
	pi_material_set_uniform(impl->post_material, impl->U_ColorTex, UT_SAMPLER_2D, 1, &impl->sampler, TRUE);
	pi_sampler_set_texture(&impl->sampler, depth_tex);
	pi_material_set_uniform(impl->post_material, impl->U_DepthTex, UT_SAMPLER_2D, 1, &impl->sampler, TRUE);

	if(impl->enable)
	{
		pi_hash_lookup(resources, impl->scene_camera_name, (void **)&scene_camera);
		pi_mat4_inverse(&view_proj_mat_inverse, pi_camera_get_view_projection_matrix(scene_camera));
		pi_material_set_uniform(impl->post_material, impl->U_ViewProjMatrixInverse, UT_MATRIX4, 1, &view_proj_mat_inverse, TRUE);
	}
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	EditorHelperRenderer *impl = (EditorHelperRenderer *)renderer->impl;
	pi_camera_set_frustum(impl->post_camera, -(float)width / 2.0f + 0.5f, width / 2.0f + 0.5f, -(float)height / 2.0f - 0.5f, (float)height / 2.0f - 0.5f, 0.0f, 2.0f, TRUE);

	pi_spatial_set_local_scaling(impl->post_quad->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->post_quad->spatial);
}

PiRenderer *PI_API pi_editor_helper_new()
{
	PiRenderer *renderer;
	EditorHelperRenderer *impl = pi_new0(EditorHelperRenderer, 1);

	impl->projection_size[0] = 100.f;
	impl->projection_size[1] = 100.0f;
	impl->projection_origin[0] = 0.0f;
	impl->projection_origin[1] = 0.0f;
	impl->projection_factor = 1.0f;

	impl->grid_snap = 0.5f;
	impl->grid_line_width = 0.01f;
	impl->grid_factor = 1.0f;

	impl->brush_size = 5.0f;
	impl->brush_pos[0] = 0.0f;
	impl->brush_pos[1] = 0.0f;

	renderer = pi_renderer_create(ERT_EDITOR_HELPER, "editor_helper", _init, _resize, _update, _draw, impl);
	return renderer;
}

void PI_API pi_editor_helper_deploy(PiRenderer *renderer, char *scene_color_name, char *scene_depth_name, char *output_name, char *scene_camera_name)
{
	EditorHelperRenderer *impl;
	_type_check(renderer);
	impl = (EditorHelperRenderer *)renderer->impl;

	pi_free(impl->scene_depth_name);
	pi_free(impl->scene_color_name);
	pi_free(impl->scene_camera_name);
	pi_free(impl->output_name);

	impl->scene_color_name = pi_str_dup(scene_color_name);
	impl->scene_depth_name = pi_str_dup(scene_depth_name);
	impl->output_name = pi_str_dup(output_name);
	impl->scene_camera_name = pi_str_dup(scene_camera_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_editor_helper_free(PiRenderer *renderer)
{
	EditorHelperRenderer *impl;
	_type_check(renderer);
	impl = (EditorHelperRenderer *)renderer->impl;

	pi_mesh_free(impl->mesh);
	pi_rendermesh_free(impl->rmesh);
	pi_entity_free(impl->post_quad);
	pi_camera_free(impl->post_camera);
	pi_material_free(impl->post_material);
	pi_free(impl->scene_depth_name);
	pi_free(impl->scene_color_name);
	pi_free(impl->scene_camera_name);
	pi_free(impl->output_name);

	pi_mesh_free(impl->reverse_mesh);
	pi_rendermesh_free(impl->reverse_rmesh);
	pi_entity_free(impl->texture_merge_quad);
	pi_camera_free(impl->texture_merge_camera);
	pi_material_free(impl->texture_merge_material);

	pi_rendertarget_free(impl->texture_merge_rt);
	pi_renderview_free(impl->color_map_view);
	pi_renderview_free(impl->alpha_map_view);

	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}

static void _check_enable(PiRenderer *renderer, EditorHelperRenderer *impl)
{
	if(impl->brush_enable || impl->projection_enable || impl->grid_enable || impl->draw_shape_enable)
	{
		if(!impl->enable)
		{
			impl->enable = TRUE;

			if(renderer->is_init)
			{
				pi_material_set_def(impl->post_material, impl->ENABLE, TRUE);
			}
		}
	}
	else if(impl->enable)
	{
		impl->enable = FALSE;

		if(renderer->is_init)
		{
			pi_material_set_def(impl->post_material, impl->ENABLE, FALSE);
		}
	}
}

void PI_API pi_editor_helper_set_brush_enable(PiRenderer *renderer, PiBool brush_enable)
{
	EditorHelperRenderer *impl;
	_type_check(renderer);
	impl = (EditorHelperRenderer *)renderer->impl;

	if(impl->brush_enable ^ brush_enable)
	{
		impl->brush_enable = brush_enable;
		_check_enable(renderer, impl);

		if(renderer->is_init)
		{
			_set_brush_data_to_material(impl);
		}
	}
}

void PI_API pi_editor_helper_set_brush_pos(PiRenderer *renderer, float pos_x, float pos_y)
{
	EditorHelperRenderer *impl;
	_type_check(renderer);
	impl = (EditorHelperRenderer *)renderer->impl;

	impl->brush_pos[0] = pos_x;
	impl->brush_pos[1] = pos_y;

	if(impl->brush_enable && renderer->is_init)
	{
		pi_material_set_uniform(impl->post_material, impl->U_BrushPos, UT_VEC2, 1, &impl->brush_pos, TRUE);
	}
}

void PI_API pi_editor_helper_set_brush_size(PiRenderer *renderer, float size)
{
	EditorHelperRenderer *impl;
	_type_check(renderer);
	impl = (EditorHelperRenderer *)renderer->impl;

	impl->brush_size = size;

	if(impl->brush_enable && renderer->is_init)
	{
		pi_material_set_uniform(impl->post_material, impl->U_BrushSize, UT_FLOAT, 1, &impl->brush_size, TRUE);
	}
}

void PI_API pi_editor_helper_set_projection_enable(PiRenderer *renderer, PiBool projection_enable)
{
	EditorHelperRenderer *impl;
	_type_check(renderer);
	impl = (EditorHelperRenderer *)renderer->impl;

	if(impl->projection_enable ^ projection_enable)
	{
		impl->projection_enable = projection_enable;

		_check_enable(renderer, impl);

		if(renderer->is_init)
		{
			_set_projection_data_to_material(impl);
		}
	}
}

void PI_API pi_editor_helper_set_projection_texture(PiRenderer *renderer, PiTexture *tex)
{
	EditorHelperRenderer *impl;
	_type_check(renderer);
	impl = (EditorHelperRenderer *)renderer->impl;

	impl->projection_texture = tex;

	if(impl->projection_enable && renderer->is_init)
	{
		pi_sampler_set_texture(&impl->sampler, impl->projection_texture);
		pi_material_set_uniform(impl->post_material, impl->U_ProjectionTexture, UT_SAMPLER_2D, 1, &impl->sampler, TRUE);
	}
}

void PI_API pi_editor_helper_set_projection_data(PiRenderer *renderer, float origin_x, float origin_y,
        float length, float width, float factor)
{
	EditorHelperRenderer *impl;
	_type_check(renderer);
	impl = (EditorHelperRenderer *)renderer->impl;

	impl->projection_origin[0] = origin_x;
	impl->projection_origin[1] = origin_y;
	impl->projection_size[0] = length;
	impl->projection_size[1] = width;
	impl->projection_factor = factor;

	if(impl->projection_enable && renderer->is_init)
	{
		pi_material_set_uniform(impl->post_material, impl->U_ProjectionOrigin, UT_VEC2, 1, &impl->projection_origin, TRUE);
		pi_material_set_uniform(impl->post_material, impl->U_ProjectionSize, UT_VEC2, 1, &impl->projection_size, TRUE);
		pi_material_set_uniform(impl->post_material, impl->U_ProjectionFactor, UT_FLOAT, 1, &impl->projection_factor, TRUE);
	}
}

void PI_API pi_editor_helper_set_grid_enable(PiRenderer *renderer, PiBool grid_enable)
{
	EditorHelperRenderer *impl;
	_type_check(renderer);
	impl = (EditorHelperRenderer *)renderer->impl;

	if(impl->grid_enable ^ grid_enable)
	{
		impl->grid_enable = grid_enable;

		_check_enable(renderer, impl);

		if(renderer->is_init)
		{
			if(impl->grid_enable)
			{
				pi_material_set_def(impl->post_material, impl->GRID_ENABLE, TRUE);
				_set_grid_data_to_material(impl);
			}
			else
			{
				pi_material_set_def(impl->post_material, impl->GRID_ENABLE, FALSE);
			}
		}
	}
}

void PI_API pi_editor_helper_set_grid_data(PiRenderer *renderer, float snap, float line_width, float factor, PiBool color_mode)
{
	EditorHelperRenderer *impl;
	_type_check(renderer);
	impl = (EditorHelperRenderer *)renderer->impl;

	impl->grid_snap = snap;
	impl->grid_line_width = line_width;
	impl->grid_factor = factor;
	impl->color_mode = color_mode;

	if(impl->grid_enable && renderer->is_init)
	{
		_set_grid_data_to_material(impl);
	}
}

void PI_API pi_editor_helper_texture_merge_init(PiRenderer *renderer, PiTexture *color_output)
{
	EditorHelperRenderer *impl;
	uint width = color_output->width;
	uint height = color_output->height;
	_type_check(renderer);
	impl = (EditorHelperRenderer *)renderer->impl;
	pi_camera_set_frustum(impl->texture_merge_camera, -(float)width / 2.0f, width / 2.0f, -(float)height / 2.0f, (float)height / 2.0f, 0.0f, 2.0f, TRUE);
	
	pi_spatial_set_local_scaling(impl->texture_merge_quad->spatial, (float)width, (float)height, 1.0f);
	pi_spatial_update(impl->texture_merge_quad->spatial);

	if(impl->color_map_view != NULL)
	{
		pi_rendertarget_detach(impl->texture_merge_rt, ATT_COLOR0);
		pi_renderview_free(impl->color_map_view);
	}

	impl->color_map_view = pi_renderview_new_tex2d(RVT_COLOR, color_output, 0, 0, TRUE);
	pi_rendertarget_attach(impl->texture_merge_rt, ATT_COLOR0, impl->color_map_view);

	pi_rendertarget_set_viewport(impl->texture_merge_rt, 0, 0, width, height);

}

void PI_API pi_editor_helper_texture_merge_set(PiRenderer *renderer, float *scale, PiTexture **base_color_maps)
{
	EditorHelperRenderer *impl;
	_type_check(renderer);
	impl = (EditorHelperRenderer *)renderer->impl;

	pi_material_set_uniform(impl->texture_merge_material, impl->U_Scale_0, UT_VEC2, 1, scale, TRUE);

	pi_material_set_uniform(impl->texture_merge_material, impl->U_Scale_1, UT_VEC2, 1, &scale[2], TRUE);

	pi_material_set_uniform(impl->texture_merge_material, impl->U_Scale_2, UT_VEC2, 1, &scale[4], TRUE);

	pi_material_set_uniform(impl->texture_merge_material, impl->U_Scale_3, UT_VEC2, 1, &scale[6], TRUE);

	pi_renderstate_set_default_sampler(&impl->texture_merge_color_map_sampler);

	pi_sampler_set_texture(&impl->texture_merge_color_map_sampler, base_color_maps[0]);
	pi_material_set_uniform(impl->texture_merge_material, impl->U_Color_0, UT_SAMPLER_2D, 1, &impl->texture_merge_color_map_sampler, TRUE);

	pi_sampler_set_texture(&impl->texture_merge_color_map_sampler, base_color_maps[1]);
	pi_material_set_uniform(impl->texture_merge_material, impl->U_Color_1, UT_SAMPLER_2D, 1, &impl->texture_merge_color_map_sampler, TRUE);

	pi_sampler_set_texture(&impl->texture_merge_color_map_sampler, base_color_maps[2]);
	pi_material_set_uniform(impl->texture_merge_material, impl->U_Color_2, UT_SAMPLER_2D, 1, &impl->texture_merge_color_map_sampler, TRUE);

	pi_sampler_set_texture(&impl->texture_merge_color_map_sampler, base_color_maps[3]);
	pi_material_set_uniform(impl->texture_merge_material, impl->U_Color_3, UT_SAMPLER_2D, 1, &impl->texture_merge_color_map_sampler, TRUE);
}

void PI_API pi_editor_helper_texture_merge_run(PiRenderer *renderer, PiTexture **blending_alpha_maps)
{
	PiColor background;
	EditorHelperRenderer *impl;
	_type_check(renderer);
	impl = (EditorHelperRenderer *)renderer->impl;

	pi_renderstate_set_default_sampler(&impl->texture_merge_alpha_sampler);

	pi_sampler_set_texture(&impl->texture_merge_alpha_sampler, blending_alpha_maps[0]);
	pi_material_set_uniform(impl->texture_merge_material, impl->U_Alpha_0, UT_SAMPLER_2D, 1, &impl->texture_merge_alpha_sampler, TRUE);

	pi_sampler_set_texture(&impl->texture_merge_alpha_sampler, blending_alpha_maps[1]);
	pi_material_set_uniform(impl->texture_merge_material, impl->U_Alpha_1, UT_SAMPLER_2D, 1, &impl->texture_merge_alpha_sampler, TRUE);

	pi_sampler_set_texture(&impl->texture_merge_alpha_sampler, blending_alpha_maps[2]);
	pi_material_set_uniform(impl->texture_merge_material, impl->U_Alpha_2, UT_SAMPLER_2D, 1, &impl->texture_merge_alpha_sampler, TRUE);

	pi_sampler_set_texture(&impl->texture_merge_alpha_sampler, blending_alpha_maps[3]);
	pi_material_set_uniform(impl->texture_merge_material, impl->U_Alpha_3, UT_SAMPLER_2D, 1, &impl->texture_merge_alpha_sampler, TRUE);

	pi_rendersystem_set_camera(impl->texture_merge_camera);
	pi_rendersystem_set_target(impl->texture_merge_rt);
	color_set(&background, 0.0f, 0.0f, 0.0f, 0.0f);
	pi_rendersystem_clearview(TBM_COLOR, &background, 1.0f, 0);

	pi_entity_draw(impl->texture_merge_quad);
}

void PI_API pi_editor_helper_set_draw_shape_shapes( PiRenderer *renderer, PiVector *circleList, PiVector *rectangleList, PiVector *sectorList )
{
	EditorHelperRenderer *impl;
	uint32 i;
	_type_check(renderer);
	impl = (EditorHelperRenderer *)renderer->impl;


	impl->circle_num = pi_vector_size(circleList) / SHAPE_CIRCLE_SIZE;
	for( i = 0; i < impl->circle_num; ++i)
	{
		impl->circle_pos[i][0] = *(float *)pi_vector_get(circleList, i * SHAPE_CIRCLE_SIZE);
		impl->circle_pos[i][1] = *(float *)pi_vector_get(circleList, i * SHAPE_CIRCLE_SIZE + 1);
		impl->circle_radius[i] = *(float *)pi_vector_get(circleList, i * SHAPE_CIRCLE_SIZE + 2);
	}
	impl->rectangle_num = pi_vector_size(rectangleList) / SHAPE_RECTANGLE_SIZE;
	for( i = 0; i < impl->rectangle_num; ++i)
	{
		PiVector3 dir;
		PiVector3 dirY = {0, 1, 0};
		float angle;
		float cosAngle;
		float sinAngle;
		impl->rectangle_pos[i][0] = *(float *)pi_vector_get(rectangleList, i * SHAPE_RECTANGLE_SIZE);
		impl->rectangle_pos[i][1] = *(float *)pi_vector_get(rectangleList, i * SHAPE_RECTANGLE_SIZE + 1);
		dir.x = *(float *)pi_vector_get(rectangleList, i * SHAPE_RECTANGLE_SIZE + 2);
		dir.y = *(float *)pi_vector_get(rectangleList, i * SHAPE_RECTANGLE_SIZE + 3);
		dir.z = 0;
		angle = pi_vec3_angle(&dir, &dirY);
		cosAngle = pi_math_cos(angle);
		sinAngle = pi_math_sin(angle);		
		impl->rectangle_rotation_mat[i][0][0] = cosAngle;
		impl->rectangle_rotation_mat[i][0][1] = -sinAngle * pi_sign(dir.x);
		impl->rectangle_rotation_mat[i][0][2] = 0;
		impl->rectangle_rotation_mat[i][1][0] = sinAngle  * pi_sign(dir.x);
		impl->rectangle_rotation_mat[i][1][1] = cosAngle;
		impl->rectangle_rotation_mat[i][1][2] = 0;
		impl->rectangle_rotation_mat[i][2][0] = 0;
		impl->rectangle_rotation_mat[i][2][1] = 0;
		impl->rectangle_rotation_mat[i][2][2] = 1;


		impl->rectangle_width[i] = *(float *)pi_vector_get(rectangleList, i * SHAPE_RECTANGLE_SIZE + 4);
		impl->rectangle_height[i] = *(float *)pi_vector_get(rectangleList, i * SHAPE_RECTANGLE_SIZE + 5);
	}
	impl->sector_num = pi_vector_size(sectorList) / SHAPE_SECTOR_SIZE;
	for( i = 0; i < impl->sector_num; ++i)
	{
		impl->sector_pos[i][0] = *(float *)pi_vector_get(sectorList, i * SHAPE_SECTOR_SIZE);
		impl->sector_pos[i][1] = *(float *)pi_vector_get(sectorList, i * SHAPE_SECTOR_SIZE + 1);
		impl->sector_dir[i][0] = *(float *)pi_vector_get(sectorList, i * SHAPE_SECTOR_SIZE + 2);
		impl->sector_dir[i][1] = *(float *)pi_vector_get(sectorList, i * SHAPE_SECTOR_SIZE + 3);
		impl->sector_radius[i] = *(float *)pi_vector_get(sectorList, i * SHAPE_SECTOR_SIZE + 4);
		impl->sector_angle[i] = *(float *)pi_vector_get(sectorList, i * SHAPE_SECTOR_SIZE + 5);
	}
	if(renderer->is_init)
	{
		if(impl->circle_num > 0)
		{
			pi_material_set_def(impl->post_material, impl->DRAW_CIRCLE_ENABLE, TRUE);
			pi_material_set_uniform(impl->post_material, impl->U_ShapeCircleNum, UT_INT, 1, &impl->circle_num, FALSE);
			pi_material_set_uniform(impl->post_material, impl->U_ShapeCirclePos, UT_VEC2, impl->circle_num, impl->circle_pos, FALSE);
			pi_material_set_uniform(impl->post_material, impl->U_ShapeCircleRadius, UT_FLOAT, impl->circle_num, impl->circle_radius, FALSE);
		}
		else
		{
			pi_material_set_def(impl->post_material, impl->DRAW_CIRCLE_ENABLE, FALSE);
		}

		if(impl->rectangle_num > 0)
		{
			pi_material_set_def(impl->post_material, impl->DRAW_RECTANGLE_ENABLE, TRUE);
			pi_material_set_uniform(impl->post_material, impl->U_ShapeRectangleNum, UT_INT, 1, &impl->rectangle_num, FALSE);
			pi_material_set_uniform(impl->post_material, impl->U_ShapeRectanglePos, UT_VEC2, impl->rectangle_num, impl->rectangle_pos, FALSE);
			pi_material_set_uniform(impl->post_material, impl->U_ShapeRectangleRotationMat, UT_MATRIX3, impl->rectangle_num, impl->rectangle_rotation_mat, FALSE);
			pi_material_set_uniform(impl->post_material, impl->U_ShapeRectangleWidth, UT_FLOAT, impl->rectangle_num, impl->rectangle_width, FALSE);
			pi_material_set_uniform(impl->post_material, impl->U_ShapeRectangleHeight, UT_FLOAT, impl->rectangle_num, impl->rectangle_height, FALSE);
		}
		else
		{
			pi_material_set_def(impl->post_material, impl->DRAW_RECTANGLE_ENABLE, FALSE);
		}

		if(impl->sector_num > 0)
		{
			pi_material_set_def(impl->post_material, impl->DRAW_SECTOR_ENABLE, TRUE);
			pi_material_set_uniform(impl->post_material, impl->U_ShapeSectorNum, UT_INT, 1, &impl->sector_num, FALSE);
			pi_material_set_uniform(impl->post_material, impl->U_ShapeSectorPos, UT_VEC2, impl->sector_num, impl->sector_pos, FALSE);
			pi_material_set_uniform(impl->post_material, impl->U_ShapeSectorDir, UT_VEC2, impl->sector_num, impl->sector_dir, FALSE);
			pi_material_set_uniform(impl->post_material, impl->U_ShapeSectorRadius, UT_FLOAT, impl->sector_num, impl->sector_radius, FALSE);
			pi_material_set_uniform(impl->post_material, impl->U_ShapeSectorAngle, UT_FLOAT, impl->sector_num, impl->sector_angle, FALSE);
		}
		else
		{
			pi_material_set_def(impl->post_material, impl->DRAW_SECTOR_ENABLE, FALSE);

		}
	}
}

void PI_API pi_editor_helper_set_draw_shape_enable( PiRenderer *renderer, PiBool is_enable )
{
	EditorHelperRenderer *impl;
	_type_check(renderer);
	impl = (EditorHelperRenderer *)renderer->impl;
	impl->draw_shape_enable = is_enable;
	_check_enable(renderer, impl);
	if(renderer->is_init)
	{
		pi_material_set_def(impl->post_material, impl->DRAW_SHAPE_ENABLE, is_enable);
	}
}
