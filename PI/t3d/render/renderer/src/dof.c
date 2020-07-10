
#include "dof.h"
#include <rendersystem.h>
#include <entity.h>

/**
 * 景深DOF(depth of field)渲染器
 */
typedef struct
{
	PiBool is_deploy;

	char *target_name;
	char *view_cam_name;
	char *color_tex_name;
	char *depth_tex_name;

	PiCamera *quad_camera;

	PiMaterial *material;
	PiMesh *quad_mesh;
	PiRenderMesh *quad_rmesh;
	PiEntity *quad_entity;

	SamplerState color_tex_sampler;
	SamplerState depth_tex_sampler;
	/* 逆矩阵， 用于逆向获取深度值 */
	PiMatrix4 proj_mat_inverse;

	/* 模糊处理 */
	float blur_template[41][2];
	/*屏幕空间焦点坐标*/
	float focalPoint[2];
	PiVector3 viewSpacePoint;
	/*焦点深度*/
	float distance;
	/*衰减系数*/
	PiVector3 regionRangeScale;

	/* 字符串常量 */
	char *U_SceneColorTex;
	char *U_SceneDepthTex;
	char *U_ProjMatrixInverse;

	char *U_FocalPoint;
	char *U_RegionRangeScale;
	char *U_BlurTemplate;
} DOFRenderer;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT((renderer)->type == ERT_DOF, "Renderer type error!");
}

/* 模糊模板值 来源 http://www.pasteall.org/10779 */
static void _blur_proc(float blur_template[41][2], float width, float height)
{
	uint i;
	float width_pix = 1.0f / width;
	float height_pix = 1.0f / height;
	float aspect_ratio = width / height;
	float temp[41][2] =
	{
		{0.0f, 0.0f},
		{0.0f, 0.4f},
		{0.15f, 0.37f},
		{0.29f, 0.29f},
		{ -0.37f, 0.15f},
		{0.4f, 0.0f},
		{0.37f, -0.15f},
		{0.29f, -0.29f},
		{ -0.15f, -0.37f},
		{0.0f, -0.4f},
		{ -0.15f, 0.37f},
		{ -0.29f, 0.29f},
		{0.37f, 0.15f},
		{ -0.4f, 0.0f},
		{ -0.37f, -0.15f},
		{ -0.29f, -0.29f},
		{0.15f, -0.37f},

		{0.15f, 0.37f},
		{ -0.37f, 0.15f},
		{0.37f, -0.15f},
		{ -0.15f, -0.37f},
		{ -0.15f, 0.37f},
		{0.37f, 0.15f},
		{ -0.37f, -0.15f},
		{0.15f, -0.37f},

		{0.29f, 0.29f},
		{0.4f, 0.0f},
		{0.29f, -0.29f},
		{0.0f, -0.4f},
		{ -0.29f, 0.29f},
		{ -0.4f, 0.0f},
		{ -0.29f, -0.29f},
		{0.0f, 0.4f},

		{0.29f, 0.29f},
		{0.4f, 0.0f},
		{0.29f, -0.29f},
		{0.0f, -0.4f},
		{ -0.29f, 0.29f},
		{ -0.4f, 0.0f},
		{ -0.29f, -0.29f},
		{0.0f, 0.4f},
	};

	for (i = 0; i < 41; i++)
	{
		temp[i][1] *= aspect_ratio;
	}

	for (i = 17; i < 25; i++)
	{
		temp[i][0] *= 0.9f;
		temp[i][1] *= 0.9f;
	}

	for (i = 25; i < 33; i++)
	{
		temp[i][0] *= 0.7f;
		temp[i][1] *= 0.7f;
	}

	for (i = 33; i < 41; i++)
	{
		temp[i][0] *= 0.4f;
		temp[i][1] *= 0.4f;
	}

	for (i = 0; i < 41; i++)
	{
		temp[i][0] *= width_pix;
		temp[i][1] *= height_pix;
	}

	pi_memcpy(blur_template, temp, sizeof(float) * 41 * 2);
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	float width, height;
	PiRenderTarget *target;
	DOFRenderer *impl;
	_type_check(renderer);
	impl = (DOFRenderer *)renderer->impl;

	if (!impl->is_deploy)
	{
		return FALSE;
	}

	pi_hash_lookup(resources, impl->target_name, (void **)&target);
	width = (float)target->width;
	height = (float)target->height;

	impl->quad_camera = pi_camera_new();
	pi_camera_set_location(impl->quad_camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->quad_camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->quad_camera, -width / 2.0f + 0.5f, width / 2.0f + 0.5f, -height / 2.0f - 0.5f, height / 2.0f - 0.5f, 0.0f, 2.0f, TRUE);



	/* 创建材质 专门用于景深处理 */
	impl->material = pi_material_new(RS_DOF_VS, RS_DOF_FS);
	pi_material_set_depth_enable(impl->material, FALSE);

	/* 屏幕网格构建 用于景深处理 */
	impl->quad_mesh = pi_mesh_create_quad(NULL, NULL, 0.0f);
	impl->quad_rmesh = pi_rendermesh_new(impl->quad_mesh, TRUE);

	impl->quad_entity = pi_entity_new();
	pi_entity_set_material(impl->quad_entity, impl->material);
	pi_entity_set_mesh(impl->quad_entity, impl->quad_rmesh);
	pi_spatial_set_local_scaling(impl->quad_entity->spatial, width, height, 1.0f);
	pi_spatial_update(impl->quad_entity->spatial);

	/* 初始化纹理采样器 */
	pi_renderstate_set_default_sampler(&impl->color_tex_sampler);
	pi_sampler_set_addr_mode(&impl->color_tex_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->color_tex_sampler, TFO_MIN_MAG_POINT);

	pi_renderstate_set_default_sampler(&impl->depth_tex_sampler);
	pi_sampler_set_addr_mode(&impl->depth_tex_sampler, TAM_CLAMP, TAM_CLAMP, TAM_CLAMP);
	pi_sampler_set_filter(&impl->depth_tex_sampler, TFO_MIN_MAG_POINT);

	/* 模糊处理 */
	_blur_proc(impl->blur_template, width, height);
	pi_material_set_uniform(impl->material, impl->U_BlurTemplate, UT_VEC2, 41, impl->blur_template, FALSE);


	/* 设置逆投影矩阵 */
	pi_material_set_uniform(impl->material, impl->U_ProjMatrixInverse, UT_MATRIX4, 1, &impl->proj_mat_inverse, FALSE);
	pi_material_set_uniform(impl->material, impl->U_SceneColorTex, UT_SAMPLER_2D, 1, &impl->color_tex_sampler, FALSE);
	pi_material_set_uniform(impl->material, impl->U_SceneDepthTex, UT_SAMPLER_2D, 1, &impl->depth_tex_sampler, FALSE);
	pi_material_set_uniform(impl->material, impl->U_RegionRangeScale, UT_VEC3, 1, &impl->regionRangeScale, FALSE);
	return TRUE;
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	PiRenderTarget *target;
	DOFRenderer *impl;
	_type_check(renderer);
	impl = (DOFRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->target_name, (void **)&target);

	pi_rendersystem_set_target(target);

	pi_rendersystem_set_camera(impl->quad_camera);

	pi_entity_draw(impl->quad_entity);
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	PiCamera *view_camera;
	PiTexture *color_tex, *depth_tex;
	DOFRenderer *impl;
	_type_check(renderer);
	impl = (DOFRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->view_cam_name, (void **)&view_camera);
	float scale = impl->distance / pi_camera_get_frustum_near(view_camera);
	impl->viewSpacePoint.x = impl->focalPoint[0] * (pi_camera_get_frustum_right(view_camera) - pi_camera_get_frustum_left(view_camera)) / 2.0f * scale;
	impl->viewSpacePoint.y = impl->focalPoint[1] * (pi_camera_get_frustum_top(view_camera) - pi_camera_get_frustum_bottom(view_camera)) / 2.0f * scale;
	impl->viewSpacePoint.z = impl->distance;
	pi_mat4_inverse(&impl->proj_mat_inverse, pi_camera_get_projection_matrix(view_camera));
	pi_material_set_uniform(impl->material, impl->U_FocalPoint, UT_VEC3, 1, &impl->viewSpacePoint, FALSE);
	/* 颜色纹理 */
	pi_hash_lookup(resources, impl->color_tex_name, (void **)&color_tex);
	pi_sampler_set_texture(&impl->color_tex_sampler, color_tex);

	/* 深度纹理 */
	pi_hash_lookup(resources, impl->depth_tex_name, (void **)&depth_tex);
	pi_sampler_set_texture(&impl->depth_tex_sampler, depth_tex);
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	float w = (float)width;
	float h = (float)height;
	DOFRenderer *impl;
	_type_check(renderer);
	impl = (DOFRenderer *)renderer->impl;

	pi_spatial_set_local_scaling(impl->quad_entity->spatial, w, h, 1.0f);
	pi_spatial_update(impl->quad_entity->spatial);

	pi_camera_set_frustum(impl->quad_camera, -w / 2.0f + 0.5f, w / 2.0f + 0.5f, -h / 2.0f - 0.5f, h / 2.0f - 0.5f, 0.0f, 2.0f, TRUE);

	_blur_proc(impl->blur_template, w, h);
	pi_material_set_uniform(impl->material, impl->U_BlurTemplate, UT_VEC2, 41, impl->blur_template, FALSE);
}

PiRenderer *PI_API pi_dof_new_with_name(char* name)
{
	PiRenderer *renderer;
	DOFRenderer *impl = pi_new0(DOFRenderer, 1);

	impl->U_SceneColorTex = pi_conststr("u_SceneColorTex");
	impl->U_SceneDepthTex = pi_conststr("u_SceneDepthTex");
	impl->U_ProjMatrixInverse = pi_conststr("u_ProjMatrixInverse");

	impl->U_FocalPoint = pi_conststr("u_FocalPoint");
	impl->U_RegionRangeScale = pi_conststr("u_RegionRangeScale");
	impl->U_BlurTemplate = pi_conststr("u_BlurTemplate");

	impl->focalPoint[0] = 0.0f;
	impl->focalPoint[1] = 0.0f;
	impl->distance = 20.0f;
	pi_vec3_set(&impl->regionRangeScale, 33.0f, 75.0f, 9.8f);

	renderer = pi_renderer_create(ERT_DOF, name, _init, _resize, _update, _draw, impl);
	return renderer;
}

PiRenderer *PI_API pi_dof_new()
{
	return pi_dof_new_with_name("depth of field");
}

void PI_API pi_dof_deploy(PiRenderer *renderer, char *target_name, char *view_cam_name, char *color_tex_name, char *depth_tex_name)
{
	DOFRenderer *impl;
	_type_check(renderer);
	impl = (DOFRenderer *)renderer->impl;

	pi_free(impl->target_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->color_tex_name);
	pi_free(impl->depth_tex_name);

	impl->target_name = pi_str_dup(target_name);
	impl->view_cam_name = pi_str_dup(view_cam_name);
	impl->color_tex_name = pi_str_dup(color_tex_name);
	impl->depth_tex_name = pi_str_dup(depth_tex_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_dof_set_point(PiRenderer* renderer, float x, float y)
{
	DOFRenderer *impl;
	_type_check(renderer);
	impl = (DOFRenderer *)renderer->impl;
	impl->focalPoint[0] = x;
	impl->focalPoint[1] = y;
}

void PI_API pi_dof_free(PiRenderer *renderer)
{
	DOFRenderer *impl;
	_type_check(renderer);
	impl = (DOFRenderer *)renderer->impl;

	pi_camera_free(impl->quad_camera);

	pi_entity_free(impl->quad_entity);
	pi_rendermesh_free(impl->quad_rmesh);
	pi_mesh_free(impl->quad_mesh);
	pi_material_free(impl->material);

	pi_free(impl->target_name);
	pi_free(impl->view_cam_name);
	pi_free(impl->color_tex_name);
	pi_free(impl->depth_tex_name);

	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}

void PI_API pi_dof_set_focal_distance(PiRenderer *renderer, float distance)
{
	DOFRenderer *impl;
	_type_check(renderer);
	impl = (DOFRenderer *)renderer->impl;

	impl->distance = distance;
}

void PI_API pi_dof_set_focal_region(PiRenderer *renderer, float region)
{
	DOFRenderer *impl;
	_type_check(renderer);
	impl = (DOFRenderer *)renderer->impl;
	impl->regionRangeScale.x = region;
	if (impl->material){
		pi_material_set_uniform(impl->material, impl->U_RegionRangeScale, UT_VEC3, 1, &impl->regionRangeScale, FALSE);
	}
}

void PI_API pi_dof_set_focal_range(PiRenderer *renderer, float range)
{
	DOFRenderer *impl;
	_type_check(renderer);
	impl = (DOFRenderer *)renderer->impl;
	impl->regionRangeScale.y = range;
	if (impl->material){
		pi_material_set_uniform(impl->material, impl->U_RegionRangeScale, UT_VEC3, 1, &impl->regionRangeScale, FALSE);
	}
}

void PI_API pi_dof_set_focal_scale(PiRenderer *renderer, float scale)
{
	DOFRenderer *impl;
	_type_check(renderer);
	impl = (DOFRenderer *)renderer->impl;
	impl->regionRangeScale.z = scale;
	if (impl->material){
		pi_material_set_uniform(impl->material, impl->U_RegionRangeScale, UT_VEC3, 1, &impl->regionRangeScale, FALSE);
	}
}
