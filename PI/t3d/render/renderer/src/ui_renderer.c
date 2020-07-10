#include "ui_renderer.h"

#include <pi_vector3.h>
#include <rendersystem.h>
#include <entity.h>
#include <material.h>
#include <component.h>
#include <picture.h>

/**
 * UIäÖÈ¾Æ÷
 */
typedef struct
{
	char *component_list_name;
	char *target_name;

	PiVector *draw_list;
	PiVector *tmp_stack;
	PiVector *flip_stack;

	uint width, height;

	PiCamera *camera;
	PiRenderTarget  *independent_view_target;
	PiPicture *independent_view_picture;

	PiBool is_deploy;
} UIRenderer;

static void _type_check(PiRenderer *renderer)
{
	PI_ASSERT(renderer->type == ERT_UI, "Renderer type error!");
}

static PiBool _init(PiRenderer *renderer, PiHash *resources)
{
	UIRenderer *impl = (UIRenderer *)renderer->impl;
	PiRenderTarget *target;

	if (!impl->is_deploy)
	{
		return FALSE;
	}

	pi_hash_lookup(resources, impl->target_name, (void **)&target);
	impl->width = target->width;
	impl->height = target->height;
	impl->draw_list = pi_vector_new();
	impl->tmp_stack = pi_vector_new();
	impl->flip_stack = pi_vector_new();

	impl->independent_view_picture = pi_picture_new();
	impl->independent_view_target = pi_rendertarget_new(TT_MRT, TRUE);
	impl->camera = pi_camera_new();

	pi_camera_set_location(impl->camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->camera, 0, (float)impl->width, 0, (float)impl->height, 0.0f, 2.0f, TRUE);

	return TRUE;
}

static void _reset_draw_rectangle(PiRenderTarget *rt, PiRectangle *reprint_rect, PiRectangle *rt_rect, float rt_scale[2], PiCamera *camera)
{
	uint width = (uint)((reprint_rect->max_x - reprint_rect->min_x) / rt_scale[0]);
	uint height = (uint)((reprint_rect->max_y - reprint_rect->min_y) / rt_scale[1]);
	pi_rendertarget_set_viewport(rt, (uint)((reprint_rect->min_x - rt_rect->min_x) / rt_scale[0]), (uint)((reprint_rect->min_y - rt_rect->min_y) / rt_scale[1]), width, height);
	pi_camera_set_frustum(camera, (float)reprint_rect->min_x, (float)reprint_rect->max_x, (float)reprint_rect->min_y, (float)reprint_rect->max_y, 0.0f, 2.0f, TRUE);
	pi_rendersystem_set_camera(camera);
}

static void _draw(PiRenderer *renderer, float tpf, PiHash *resources)
{
	uint count = 0;
	uint i, n;
	UIRenderer *impl;
	PiRenderTarget *target, *current_target = NULL;
	PiRectangle rt_rect;
	float rt_scale[2];
	_type_check(renderer);
	impl = (UIRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->target_name, (void **)&target);

	n = pi_vector_size(impl->draw_list);
	for (i = 0; i < n; ++i)
	{
		PiComponent *component = (PiComponent *)pi_vector_get(impl->draw_list, i);
		PiBool cache_mask = TRUE;
		PiBool is_independent = FALSE;
		if (!component)
		{
			component = (PiComponent *)pi_vector_get(impl->draw_list, ++i);
			current_target = impl->independent_view_target;
			pi_rectangle_copy(&rt_rect, &component->global_bounds);
			rt_scale[0] = component->global_scale[0];
			rt_scale[1] = component->global_scale[1];
			pi_rendertarget_attach(current_target, ATT_COLOR0, component->independent_view_buffer);
			pi_rendersystem_set_target(current_target);
			cache_mask = FALSE;
		}
		else if (!component->parent)
		{
			current_target = target;
			rt_rect.min_x = 0;
			rt_rect.min_y = 0;
			rt_rect.max_x = impl->width;
			rt_rect.max_y = impl->height;
			rt_scale[0] = 1;
			rt_scale[1] = 1;
			pi_camera_set_frustum(impl->camera, 0, (float)impl->width, 0, (float)impl->height, 0.0f, 2.0f, TRUE);
			pi_rendersystem_set_camera(impl->camera);
			pi_rendersystem_set_target(current_target);
		}

		is_independent = component->is_independent || !component->parent;
		if (is_independent && cache_mask)
		{
			uint width = component->global_bounds.max_x - component->global_bounds.min_x;
			uint height = component->global_bounds.max_y - component->global_bounds.min_y;
			pi_picture_set_size(impl->independent_view_picture, width, height);
			pi_picture_set_location(impl->independent_view_picture, component->global_bounds.min_x, component->global_bounds.min_y);
			pi_picture_set_texture(impl->independent_view_picture, component->independent_view_texture, FALSE);

			if (component->parent)
			{
				_reset_draw_rectangle(current_target, &component->reprint_bounds, &rt_rect, rt_scale, impl->camera);
			}
			pi_picture_set_blend_mode(impl->independent_view_picture, component->is_translucent ? EBM_ALPHA_ASSOCIATIVE_FINISH : EBM_NONE);
			pi_entity_draw(impl->independent_view_picture->entity);
			count++;
		}
		else
		{
			if (!is_independent || component->is_internal_reprint)
			{
				_reset_draw_rectangle(current_target, &component->reprint_bounds, &rt_rect, rt_scale, impl->camera);
				pi_component_draw(component);
				count++;
			}
		}
	}

	pi_rendertarget_set_viewport(target, 0, 0, impl->width, impl->height);

	for (i = 0; i < n; ++i)
	{
		PiComponent *component = (PiComponent *)pi_vector_get(impl->draw_list, i);
		if (component)
		{
			pi_component_finish_draw(component);
		}
	}
	pi_vector_clear(impl->draw_list, FALSE);

	//if (count != 1)
	//{
	//	pi_log_print(LOG_INFO, "%d DrawCalls", count);
	//}
}

static void _update(PiRenderer *renderer, float tpf, PiHash *resources)
{
	PiVector *component_list;
	UIRenderer *impl;
	uint i, n;

	impl = (UIRenderer *)renderer->impl;

	pi_hash_lookup(resources, impl->component_list_name, (void **)&component_list);
	n = pi_vector_size(component_list);
	pi_component_sort_layer(component_list);

	for (i = 0; i < n; ++i)
	{
		PiComponent *component = (PiComponent *)pi_vector_get(component_list, i);
		if (component->parent == NULL)
		{
			pi_component_update(component, tpf);
			pi_component_get_reprint_components(component, impl->draw_list, impl->tmp_stack, impl->flip_stack);
		}
	}
}

static void _resize(PiRenderer *renderer, uint width, uint height)
{
	UIRenderer *impl = (UIRenderer *)renderer->impl;

	impl->width = width;
	impl->height = height;

	pi_camera_set_location(impl->camera, 0.0f, 0.0f, 1.0f);
	pi_camera_set_direction(impl->camera, 0.0f, 0.0f, -1.0f);
	pi_camera_set_frustum(impl->camera, 0, (float)impl->width, 0, (float)impl->height, 0.0f, 2.0f, TRUE);
}

PiRenderer *PI_API pi_ui_renderer_new()
{
	PiRenderer *renderer;
	UIRenderer *impl = pi_new0(UIRenderer, 1);

	renderer = pi_renderer_create(ERT_UI, "UI", _init, _resize, _update, _draw, impl);
	return renderer;
}

void PI_API pi_ui_renderer_deploy(PiRenderer *renderer, char *target_name, char *component_list_name)
{
	UIRenderer *impl;
	_type_check(renderer);
	impl = (UIRenderer *)renderer->impl;

	pi_free(impl->target_name);
	pi_free(impl->component_list_name);
	impl->target_name = pi_str_dup(target_name);
	impl->component_list_name = pi_str_dup(component_list_name);

	impl->is_deploy = TRUE;
}

void PI_API pi_ui_renderer_free(PiRenderer *renderer)
{
	UIRenderer *impl;
	_type_check(renderer);
	impl = (UIRenderer *)renderer->impl;

	pi_free(impl->target_name);
	pi_free(impl->component_list_name);
	pi_vector_free(impl->draw_list);
	pi_vector_free(impl->tmp_stack);
	pi_vector_free(impl->flip_stack);
	pi_rendertarget_free(impl->independent_view_target);
	pi_picture_free(impl->independent_view_picture);
	pi_camera_free(impl->camera);

	pi_free(renderer->impl);
	pi_renderer_destroy(renderer);
}
