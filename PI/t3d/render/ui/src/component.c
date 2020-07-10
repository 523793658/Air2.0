#include "component.h"

PiCompR PI_API _layer_compare_func(void	*user_data, const PiComponent *a, const PiComponent *b)
{
	if (a->layer < b->layer)
	{
		return PI_COMP_LESS;
	}
	else if (a->layer > b->layer)
	{
		return PI_COMP_GREAT;
	}
	else
	{
		if ((uint)a < (uint)b)
		{
			return PI_COMP_LESS;
		}
		else
		{
			return (PiCompR)((uint)a > (uint)b);
		}
	}
}

static void _default_draw_func(PiComponent *component, void *impl) {}

static void _default_update_func(PiComponent *component, float tpf) {}

static PiBool PI_API _vector_remove_func(void *user_data, const void *data)
{
	return user_data == data;
}

static PiBool _is_independent(PiComponent *component)
{
	return component->is_independent || !component->parent;
}

static void _set_reprint(PiComponent *component)
{
	component->is_reprint = TRUE;
}

static void _reprint(PiComponent *component, PiRectangle *rect)
{
	if (rect)
	{
		pi_rectangle_merge(&component->reprint_bounds, &component->global_bounds);
	}
	else
	{
		pi_rectangle_copy(&component->reprint_bounds, &component->global_bounds);
	}
}

static void _set_bounds_update(PiComponent *component)
{
	component->is_bounds_update = TRUE;
	if (!_is_independent(component))
	{
		component->is_reprint = TRUE;
	}
}

static void _bounds_update(PiComponent *component)
{
	if (!component->is_visible)
	{
		pi_rectangle_reset(&component->global_bounds);
	}
	else
	{
		float displacement[2];
		component->global_scale[0] = component->scale[0];
		component->global_scale[1] = component->scale[1];
		pi_rectangle_copy(&component->global_bounds, &component->bounds);
		displacement[0] = (float)component->global_bounds.max_x - component->global_bounds.min_x;
		displacement[1] = (float)component->global_bounds.max_y - component->global_bounds.min_y;
		displacement[0] *= component->global_scale[0];
		displacement[1] *= component->global_scale[1];
		component->global_bounds.max_x = (sint)(component->global_bounds.min_x + displacement[0]);
		component->global_bounds.max_y = (sint)(component->global_bounds.min_y + displacement[1]);
		if (component->parent)
		{
			component->global_scale[0] *= component->parent->global_scale[0];
			component->global_scale[1] *= component->parent->global_scale[1];
			component->global_bounds.min_x = (sint)(component->parent->global_scale[0] * component->global_bounds.min_x);
			component->global_bounds.min_y = (sint)(component->parent->global_scale[1] * component->global_bounds.min_y);
			component->global_bounds.max_x = (sint)(component->parent->global_scale[0] * component->global_bounds.max_x);
			component->global_bounds.max_y = (sint)(component->parent->global_scale[1] * component->global_bounds.max_y);

			component->global_bounds.min_x += component->parent->global_bounds.min_x;
			component->global_bounds.min_y += component->parent->global_bounds.min_y;
			component->global_bounds.max_x += component->parent->global_bounds.min_x;
			component->global_bounds.max_y += component->parent->global_bounds.min_y;
		}
	}
}
static PiBool _dirty_mark_branch(PiComponent *component, PiComponent *root, PiComponent *target, PiRectangle *limit_bound, PiBool is_erase, PiBool independent_reprint, PiBool *self_meet)
{
	uint i, n;
	PiComponent *child;
	PiRectangle intersection;
	PiBool iter_children = TRUE;
	PiRectangle limit_local;

	if (!*self_meet)
	{
		*self_meet = target == component;
	}
	pi_rectangle_intersection(&limit_local, &component->global_bounds, limit_bound);
	if ((is_erase) || (!is_erase && *self_meet))
	{
		if ((component->is_reprint || independent_reprint) && component == target)
		{
			pi_rectangle_copy(&component->reprint_bounds, &component->global_bounds);

			pi_rectangle_intersection(&component->reprint_bounds, &limit_local, &component->reprint_bounds);
			{
				PiComponent *tmp = component;
				while (tmp->parent)
				{
					tmp = tmp->parent;
					if (_is_independent(tmp))
					{
						pi_rectangle_merge(&tmp->reprint_bounds, &component->reprint_bounds);
					}
				}
			}
		}
		else
		{
			if (target->is_reprint || target->is_bounds_update)
			{
				pi_rectangle_intersection(&intersection, &component->global_bounds, &target->global_bounds);
			}
			else
			{
				pi_rectangle_intersection(&intersection, &component->global_bounds, &target->reprint_bounds);
			}
			pi_rectangle_merge(&component->reprint_bounds, &intersection);
		}
	}

	if (_is_independent(component))
	{
		if ((component == root && target != component) || component->is_reprint)
		{
			if (!pi_rectangle_is_void(&component->reprint_bounds))
			{
				component->is_internal_reprint = TRUE;
			}
		}
		else
		{
			iter_children = FALSE;
		}
	}

	if (iter_children)
	{
		n = pi_vector_size(component->children);
		for (i = 0; i < n; i++)
		{
			child = (PiComponent *)pi_vector_get(component->children, i);

			if (!_dirty_mark_branch(child, root, target, &limit_local, is_erase, independent_reprint, self_meet))
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

static void _dirty_mark(PiComponent *component, PiBool is_erase, PiBool independent_reprint)
{
	PiComponent *root = component;
	PiBool meet = FALSE;
	PiRectangle limit_local;

	if (_is_independent(root) && component->parent)
	{
		root = root->parent;
	}

	while (!_is_independent(root))
	{
		root = root->parent;
	}

	if (root->parent)
	{
		pi_rectangle_copy(&limit_local, &root->parent->global_bounds);
	}
	else
	{
		pi_rectangle_set_max(&limit_local);
	}
	_dirty_mark_branch(root, root, component, &limit_local, is_erase, independent_reprint, &meet);

	if (root->parent)
	{
		_dirty_mark(root, is_erase, FALSE);
	}
}

static void _merge_parent_reprint_bound(PiComponent *component)
{
	uint i, n;
	PiComponent *child;
	PiRectangle intersection;

	n = pi_vector_size(component->children);
	for (i = 0; i < n; i++)
	{
		child = (PiComponent *)pi_vector_get(component->children, i);

		if (!_is_independent(component) || component->is_internal_reprint)
		{
			pi_rectangle_intersection(&intersection, &child->global_bounds, &component->reprint_bounds);
			pi_rectangle_merge(&child->reprint_bounds, &intersection);
		}
		_merge_parent_reprint_bound(child);
	}
}

static void _set_layer_update(PiComponent *component)
{
	if (component->parent)
	{
		component->parent->is_layer_update = TRUE;
		_set_reprint(component->parent);
	}
}

static void _sort_layer(PiComponent *component)
{
	pi_vector_sort(component->children, (PiCompareFunc)_layer_compare_func, NULL);
	component->is_layer_update = FALSE;
}

static void _impl_update(PiComponent *component, float tpf)
{
	uint i, n;
	PiComponent *child;

	component->update_func(component, tpf);
	n = pi_vector_size(component->children);
	for (i = 0; i < n; i++)
	{
		child = (PiComponent *)pi_vector_get(component->children, i);
		_impl_update(child, tpf);
	}
}

PiComponent *PI_API pi_component_new(EWidgetsType type, ComponentDrawFunc draw_func, ComponentUpdateFunc update_func, void *impl)
{
	PiComponent *component = pi_new0(PiComponent, 1);
	component->children = pi_vector_new();
	component->draw_func = draw_func ? draw_func : _default_draw_func;
	component->update_func = update_func ? update_func : _default_update_func;
	component->impl = impl;
	component->is_bounds_update = TRUE;
	component->is_reprint = TRUE;
	component->is_internal_reprint = FALSE;
	component->is_visible = TRUE;
	component->is_layer_update = FALSE;
	component->type = type;

	component->scale[0] = component->scale[1] = 1;
	pi_rectangle_reset(&component->bounds);
	pi_rectangle_reset(&component->global_bounds);
	pi_rectangle_reset(&component->reprint_bounds);

	return component;
}

void PI_API pi_component_delete(PiComponent *component)
{
	PiComponent *child = NULL;

	child = (PiComponent *)pi_vector_pop(component->children);
	while (child)
	{
		pi_component_detach_from_parent(child);
		child = (PiComponent *)pi_vector_pop(component->children);
	}
	pi_component_detach_from_parent(component);
	pi_vector_free(component->children);

	if (component->independent_view_buffer)
	{
		pi_renderview_free(component->independent_view_buffer);
		pi_texture_free(component->independent_view_texture);
	}

	pi_free(component);
}

void PI_API pi_component_detach_from_parent(PiComponent *component)
{
	if (component->parent)
	{
		pi_vector_remove_if(component->parent->children, _vector_remove_func, component);
		component->parent = NULL;
	}
}

void PI_API pi_component_attach_child(PiComponent *parent, PiComponent *child)
{
	pi_component_detach_from_parent(child);
	pi_vector_push(parent->children, child);
	child->parent = parent;
	_set_reprint(child);
	_set_layer_update(parent);
}

void PI_API pi_component_set_size(PiComponent *component, uint width, uint height)
{
	component->width = width;
	component->hight = height;
	component->bounds.max_x = component->bounds.min_x + width;
	component->bounds.max_y = component->bounds.min_y + height;
	_set_bounds_update(component);
}

void PI_API pi_component_set_location(PiComponent *component, sint x, sint y)
{
	component->bounds.min_x = x;
	component->bounds.min_y = y;
	component->bounds.max_x = component->bounds.min_x + component->width;
	component->bounds.max_y = component->bounds.min_y + component->hight;

	_set_bounds_update(component);
}

void PI_API pi_component_get_size(PiComponent *component, uint size[2])
{
	size[0] = component->width;
	size[1] = component->hight;
}

void PI_API pi_component_get_location(PiComponent *component, sint location[2])
{
	location[0] = component->bounds.min_x;
	location[1] = component->bounds.min_y;
}

void PI_API pi_component_set_scale(PiComponent *component, float x, float y)
{
	component->scale[0] = x;
	component->scale[1] = y;

	_set_bounds_update(component);
}

void PI_API pi_component_set_layer(PiComponent *component, sint layer)
{
	component->layer = layer;
	_set_layer_update(component);
}

void PI_API pi_component_set_visible(PiComponent *component, PiBool b)
{
	component->is_visible = b;
	_set_bounds_update(component);
}

void PI_API pi_component_set_independent(PiComponent *component, PiBool b)
{
	component->is_independent = b;
	_set_reprint(component);
}

void PI_API pi_component_set_translucent(PiComponent *component, PiBool b)
{
	component->is_translucent = b;
	_set_reprint(component);
}

void PI_API pi_component_draw(PiComponent *component)
{
	component->draw_func(component, component->impl);
}

void PI_API pi_component_finish_draw(PiComponent *component)
{
	component->is_internal_reprint = FALSE;
	pi_rectangle_reset(&component->reprint_bounds);
}

static void _update(PiComponent *component, PiComponent *root)
{
	uint i, n;
	PiBool is_independent = _is_independent(component);
	PiBool pre_reprint = component->parent && component->parent->is_reprint;
	PiBool independent_reprint = is_independent && component->is_bounds_update;

	if (is_independent && !component->independent_view_buffer)
	{
		component->independent_view_texture = pi_texture_2d_create(RF_ABGR8, TU_COLOR, 1, 1, component->width, component->hight, TRUE);
		component->independent_view_buffer = pi_renderview_new_tex2d(RVT_COLOR, component->independent_view_texture, 0, 0, TRUE);
	}
	if (component->is_layer_update)
	{
		_sort_layer(component);
	}
	if ((component->is_reprint || component->is_bounds_update) && !pre_reprint)
	{
		_dirty_mark(component, TRUE, FALSE);
	}
	if (component->parent)
	{
		if (component->parent->is_bounds_update)
		{
			component->is_bounds_update = TRUE;
		}
		if (component->parent->is_reprint && !is_independent)
		{
			component->is_reprint = TRUE;
		}
	}
	if (component->is_bounds_update)
	{
		_bounds_update(component);
	}
	if ((component->is_reprint || independent_reprint) && !pre_reprint)
	{
		_dirty_mark(component, FALSE, independent_reprint);
	}
	if (pre_reprint && component->is_reprint)
	{
		pi_rectangle_copy(&component->reprint_bounds, &component->global_bounds);
		if (is_independent)
		{
			component->is_internal_reprint = TRUE;
		}
	}

	n = pi_vector_size(component->children);
	for (i = 0; i < n; i++)
	{
		PiComponent *child = (PiComponent *)pi_vector_get(component->children, i);
		_update(child, is_independent ? component : root);
	}

	if (!component->parent)
	{
		_merge_parent_reprint_bound(component);
	}

	component->is_bounds_update = FALSE;
	component->is_reprint = FALSE;
}

void PI_API pi_component_update(PiComponent *component, float tpf)
{
	_impl_update(component, tpf);
	_update(component, component);
}

void PI_API pi_component_get_reprint_components(PiComponent *component, PiVector *result_buffer, PiVector *tmp_stack, PiVector *flip_stack)
{
	uint i, n;
	PiBool is_dirty = !pi_rectangle_is_void(&component->reprint_bounds);
	PiBool independent = _is_independent(component);

	if (is_dirty || independent)
	{
		pi_vector_push(tmp_stack, component);
	}
	n = pi_vector_size(component->children);
	for (i = 0; i < n; i++)
	{
		pi_component_get_reprint_components((PiComponent *)pi_vector_get(component->children, i), result_buffer, tmp_stack, flip_stack);
	}

	if (independent)
	{
		PiComponent *tmp = NULL;
		do
		{
			tmp = (PiComponent *)pi_vector_pop(tmp_stack);
			pi_vector_push(flip_stack, tmp);
		} while (tmp != component);

		if (pi_vector_size(flip_stack) != 1 || tmp->is_internal_reprint)
		{
			pi_vector_push(result_buffer, NULL);
			while (pi_vector_size(flip_stack) != 0)
			{
				pi_vector_push(result_buffer, pi_vector_pop(flip_stack));
			}
		}
		else
		{
			pi_vector_pop(flip_stack);
		}
		if (component->parent && is_dirty)
		{
			pi_vector_push(tmp_stack, component);
		}
	}
	if (!component->parent)
	{
		pi_vector_push(result_buffer, component);
	}
}

void PI_API pi_component_reprint(PiComponent *component)
{
	_set_reprint(component);
}

void PI_API pi_component_sort_layer(PiVector *components)
{
	pi_vector_sort(components, (PiCompareFunc)_layer_compare_func, NULL);
}
