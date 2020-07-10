#include "panel.h"
#include "picture.h"

typedef struct
{
	PiPicture *picture;
	PiBool is_translucent;
	PiBool is_anim;
	EBlendMode blend_mode;
} Panel;

static void _component_draw(PiComponent *component, Panel *panel)
{
	uint width = component->global_bounds.max_x - component->global_bounds.min_x;
	uint height = component->global_bounds.max_y - component->global_bounds.min_y;
	pi_picture_set_size(panel->picture, width, height);
	pi_picture_set_location(panel->picture, component->global_bounds.min_x, component->global_bounds.min_y);

	//注意:独立绘制的控件root必须以不混合方式做内部绘制
	if (panel->is_translucent && pi_picture_get_blend_mode(panel->picture) == EBM_ALPHA_ASSOCIATIVE && (component->is_independent || !component->parent))
	{
		pi_picture_set_blend_mode(panel->picture, EBM_ALPHA_ASSOCIATIVE_INIT);
	}
	pi_entity_draw(panel->picture->entity);
}

static void _component_update(PiComponent *component, float tpf)
{
	Panel *panel = component->impl;

	if (panel->is_anim)
	{
		pi_component_reprint(component);
	}
}

PiComponent *PI_API pi_panel_new()
{
	Panel *panel = pi_new0(Panel, 1);
	PiComponent *component = pi_component_new(EWT_PANEL, (ComponentDrawFunc)_component_draw, _component_update, (void *)panel);
	pi_component_set_size(component, 128, 64);
	panel->picture = pi_picture_new();

	return component;
}

void PI_API pi_panel_delete(PiComponent *component)
{
	Panel *panel = component->impl;

	pi_component_delete(component);

	pi_picture_free(panel->picture);

	pi_free(panel);
}

void PI_API pi_panel_set_color(PiComponent *component, float r, float g, float b, float a)
{
	Panel *panel = component->impl;

	pi_picture_set_color(panel->picture, r, g, b, a);
	pi_component_reprint(component);
}

void PI_API pi_panel_set_texture(PiComponent *component, PiTexture *texture)
{
	Panel *panel = component->impl;

	pi_picture_set_texture(panel->picture, texture, TRUE);
	pi_component_reprint(component);
}

void PI_API pi_panel_set_translucent(PiComponent *component, PiBool b)
{
	Panel *panel = component->impl;

	panel->is_translucent = b;
	pi_picture_set_blend_mode(panel->picture, b ? EBM_ALPHA_ASSOCIATIVE : EBM_NONE);
	pi_component_set_translucent(component, b);
}

void PI_API pi_panel_set_texture_uv_anim(PiComponent *component, float u, float v)
{
	Panel *panel = component->impl;

	pi_picture_set_texture_uv_anim(panel->picture, u, v);
	pi_component_reprint(component);

	panel->is_anim = u != 0 || v != 0;
}

void PI_API pi_panel_set_texture_tile_anim(PiComponent *component, uint tile_x, uint tile_y, float frame_time, uint tile_count, PiBool is_blend)
{
	Panel *panel = component->impl;

	pi_picture_set_texture_tile_anim(panel->picture, tile_x, tile_y, frame_time, tile_count, is_blend);
	pi_component_reprint(component);

	panel->is_anim = tile_x > 1 || tile_y > 1;
}
