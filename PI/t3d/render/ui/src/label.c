
#include "label.h"
#include "text.h"

typedef struct
{
	PiText *text;					/* 基本的text */
	float text_color[4];			/* 本体text的颜色 */
	float opacity;					/* 本体text的不透明度 */
	PiBool is_text_update;			/* 是否更新本体text */

	PiBool stroke_enable;			/* 描边开关 */
	PiText *stroke_text;			/* 描边的text */
	float stroke_size;				/* 描边的大小 */
	float stroke_color[4];			/* 描边的颜色 */
	PiBool is_stroke_update;		/* 是否更新描边的text */

	PiBool shadow_enable;			/* 阴影开关 */
	sint shadow_offset[2];			/* 阴影的偏移 */
	float shadow_color[4];			/* 阴影的颜色 */

	PiBool reprint;					/* 是否重绘 */
	sint text_offset[2];			/* text的原点相对控件原点的偏移 */
} PiTextLabel;

static void _component_draw(PiComponent *component, PiTextLabel *label)
{
	PiEntity *text_entity;

	sint text_positon[2];

	text_positon[0] = component->global_bounds.min_x + label->text_offset[0];
	text_positon[1] = component->global_bounds.min_y + label->text_offset[1];

	if (label->shadow_enable)
	{
		PiEntity *shadow_entity;
		pi_text_set_color(label->text, label->shadow_color);
		shadow_entity = pi_text_entity_get(label->text);
		if (shadow_entity != NULL)
		{
			sint shadow_position[2];
			shadow_position[0] = text_positon[0] + label->shadow_offset[0];
			shadow_position[1] = text_positon[1] - label->shadow_offset[1];
			pi_spatial_set_local_translation(shadow_entity->spatial, (float)shadow_position[0], (float)shadow_position[1], 0.0f);
			pi_spatial_update(shadow_entity->spatial);
			pi_entity_draw(shadow_entity);
		}
		pi_text_set_color(label->text, label->text_color);
	}

	if (label->stroke_enable)
	{
		PiEntity *stroke_entity = pi_text_entity_get(label->stroke_text);
		if (stroke_entity != NULL)
		{
			pi_spatial_set_local_translation(stroke_entity->spatial, (float)text_positon[0], (float)text_positon[1], 0.0f);
			pi_spatial_update(stroke_entity->spatial);
			pi_entity_draw(stroke_entity);
		}
	}

	text_entity = pi_text_entity_get(label->text);
	if (text_entity)
	{
		pi_spatial_set_local_translation(text_entity->spatial, (float)text_positon[0], (float)text_positon[1], 0.0f);
		pi_spatial_update(text_entity->spatial);
		pi_entity_draw(text_entity);
	}
}

PiComponent *PI_API pi_label_new(PiFontManager *font_manager)
{
	PiTextLabel *label;
	PiComponent *component;

	if (font_manager == NULL)
	{
		return NULL;
	}

	label = pi_new0(PiTextLabel, 1);
	component = pi_component_new(EWT_LABEL, _component_draw, NULL, label);

	pi_component_set_translucent(component, TRUE);

	label->opacity = 1.0f;

	label->text_color[3] = 1.0f;

	label->stroke_color[3] = 1.0f;

	label->shadow_color[3] = 1.0f;

	label->text = pi_text_new(font_manager);
	pi_text_set_blend_mode(label->text, TBM_ALPHA_ASSOCIATIVE);

	label->stroke_text = pi_text_new(font_manager);
	pi_text_set_blend_mode(label->stroke_text, TBM_ALPHA_ASSOCIATIVE);
	pi_text_set_outline_type(label->stroke_text, OT_LINE);

	return component;
}

void PI_API pi_label_delete(PiComponent *component)
{
	PiTextLabel *label = (PiTextLabel *)component->impl;

	pi_component_delete(component);

	pi_text_delete(label->text);
	pi_text_delete(label->stroke_text);

	pi_free(label);
}

void PI_API pi_label_set_font_family(PiComponent *component, PiFontFamilyType font_family_type)
{
	PiTextLabel *label = component->impl;

	pi_text_set_font_family(label->text, font_family_type);
	pi_text_set_font_family(label->stroke_text, font_family_type);

	label->is_text_update = TRUE;
	label->is_stroke_update = TRUE;
}

void PI_API pi_label_set_font_face_style(PiComponent *component, PiFontFaceStyle font_face_style)
{
	PiTextLabel *label = component->impl;

	pi_text_set_font_face_style(label->text, font_face_style);
	pi_text_set_font_face_style(label->stroke_text, font_face_style);

	label->is_text_update = TRUE;
	label->is_stroke_update = TRUE;
}

void PI_API pi_label_set_font_size(PiComponent *component, float point_size)
{
	PiTextLabel *label = component->impl;

	pi_text_set_point_size(label->text, point_size);
	pi_text_set_point_size(label->stroke_text, point_size);

	label->is_text_update = TRUE;
	label->is_stroke_update = TRUE;
}

void PI_API pi_label_set_bold_enable(PiComponent *component, PiBool enable)
{
	PiTextLabel *label = component->impl;

	pi_text_set_bold_enable(label->text, enable);
	pi_text_set_bold_enable(label->stroke_text, enable);

	label->is_text_update = TRUE;
	label->is_stroke_update = TRUE;
}

void PI_API pi_label_set_bold_strength(PiComponent *component, float strength)
{
	PiTextLabel *label = component->impl;

	pi_text_set_bold_strength(label->text, strength);
	pi_text_set_bold_strength(label->stroke_text, strength);

	label->is_text_update = TRUE;
	label->is_stroke_update = TRUE;
}

void PI_API pi_label_set_italic_enable(PiComponent *component, PiBool enable)
{
	PiTextLabel *label = component->impl;

	pi_text_set_italic_enable(label->text, enable);
	pi_text_set_italic_enable(label->stroke_text, enable);

	label->is_text_update = TRUE;
	label->is_stroke_update = TRUE;
}

void PI_API pi_label_set_italic_lean(PiComponent *component, float lean)
{
	PiTextLabel *label = component->impl;

	pi_text_set_italic_lean(label->text, lean);
	pi_text_set_italic_lean(label->stroke_text, lean);

	label->is_text_update = TRUE;
	label->is_stroke_update = TRUE;
}

void PI_API pi_label_set_characters(PiComponent *component, const wchar *characters)
{
	PiTextLabel *label = component->impl;

	pi_text_set_characters(label->text, characters);
	pi_text_set_characters(label->stroke_text, characters);

	label->is_text_update = TRUE;
	label->is_stroke_update = TRUE;
}

void PI_API pi_label_set_color(PiComponent *component, float r, float g, float b)
{
	PiTextLabel *label = component->impl;

	if (label->text_color[0] != r ||
		label->text_color[1] != g ||
		label->text_color[2] != b)
	{
		label->text_color[0] = r;
		label->text_color[1] = g;
		label->text_color[2] = b;

		pi_text_set_color(label->text, label->text_color);
		label->reprint = TRUE;
	}
}

static void _update_opacity(PiTextLabel *label)
{
	float opacity;
	if (label->shadow_enable || label->stroke_enable)
	{
		opacity = 1.0f;
	}
	else
	{
		opacity = label->opacity;
	}

	if (label->text_color[3] != opacity)
	{
		label->text_color[3] = opacity;
		pi_text_set_color(label->text, label->text_color);
		label->reprint = TRUE;
	}
}

void PI_API pi_label_set_opacity(PiComponent *component, float opacity)
{
	PiTextLabel *label = component->impl;

	if (label->opacity != opacity)
	{
		label->opacity = opacity;
		_update_opacity(label);
	}
}

void PI_API pi_label_set_stroke_enable(PiComponent *component, PiBool enable)
{
	PiTextLabel *label = component->impl;

	if (!!label->stroke_enable != !!enable)
	{
		label->stroke_enable = enable;
		_update_opacity(label);
		label->reprint = TRUE;
	}
}

void PI_API pi_label_set_stroke_size(PiComponent *component, float size)
{
	PiTextLabel *label = component->impl;

	if (label->stroke_size != size)
	{
		label->stroke_size = size;

		pi_text_set_outline_thickness(label->stroke_text, label->stroke_size);
		label->is_stroke_update = TRUE;
	}
}

void PI_API pi_label_set_stroke_color(PiComponent *component, float r, float g, float b)
{
	PiTextLabel *label = component->impl;

	if (label->stroke_color[0] != r ||
		label->stroke_color[1] != g ||
		label->stroke_color[2] != b)
	{
		label->stroke_color[0] = r;
		label->stroke_color[1] = g;
		label->stroke_color[2] = b;

		pi_text_set_color(label->stroke_text, label->stroke_color);

		if (label->stroke_enable)
		{
			label->reprint = TRUE;
		}
	}
}

void PI_API pi_label_set_shadow_enable(PiComponent *component, PiBool enable)
{
	PiTextLabel *label = component->impl;
	if (!!label->shadow_enable != !!enable)
	{
		label->shadow_enable = enable;
		_update_opacity(label);
		label->reprint = TRUE;
	}
}

void PI_API pi_label_set_shadow_offset(PiComponent *component, sint offset_x, sint offset_y)
{
	PiTextLabel *label = component->impl;

	if (label->shadow_offset[0] != offset_x ||
		label->shadow_offset[1] != offset_y)
	{
		label->shadow_offset[0] = offset_x;
		label->shadow_offset[1] = offset_y;

		if (label->shadow_enable)
		{
			label->reprint = TRUE;
		}
	}
}

void PI_API pi_label_set_shadow_color(PiComponent *component, float r, float g, float b)
{
	PiTextLabel *label = component->impl;

	if (label->shadow_color[0] != r ||
		label->shadow_color[1] != g ||
		label->shadow_color[2] != b)
	{
		label->shadow_color[0] = r;
		label->shadow_color[1] = g;
		label->shadow_color[2] = b;

		if (label->shadow_enable)
		{
			label->reprint = TRUE;
		}
	}
}

static void _layout_merge(PiTextLayout *dst, PiTextLayout *src)
{
	dst->left = MIN(dst->left, src->left);
	dst->right = MAX(dst->right, src->right);
	dst->top = MAX(dst->top, src->top);
	dst->bottom = MIN(dst->bottom, src->bottom);
}

static void _update_bound(PiComponent *component, PiTextLabel *label)
{
	uint width, height;
	PiTextLayout label_layout, *text_layout;

	text_layout = pi_text_layout_get(label->text);

	if (text_layout == NULL)
	{
		return;
	}

	pi_memcpy_inline(&label_layout, text_layout, sizeof(PiTextLayout));

	if (label->stroke_enable)
	{
		PiTextLayout *stroke_layout = pi_text_layout_get(label->stroke_text);
		if (stroke_layout != NULL)
		{
			_layout_merge(&label_layout, stroke_layout);
		}
	}

	if (label->shadow_enable)
	{
		PiTextLayout shadow_layout;

		shadow_layout.left = text_layout->left + label->shadow_offset[0];
		shadow_layout.right = text_layout->right + label->shadow_offset[0];
		shadow_layout.bottom = text_layout->bottom - label->shadow_offset[1];
		shadow_layout.top = text_layout->top - label->shadow_offset[1];
		_layout_merge(&label_layout, &shadow_layout);
	}

	width = (uint)(label_layout.right - label_layout.left);
	height = (uint)(label_layout.top - label_layout.bottom);

	label->text_offset[0] = -label_layout.left;
	label->text_offset[1] = -label_layout.bottom;

	pi_component_set_size(component, width, height);
}

void PI_API pi_label_update(PiComponent *component)
{
	PiTextLabel *label = component->impl;

	if (label->is_text_update)
	{
		pi_text_update(label->text);
		label->is_text_update = FALSE;
		label->reprint = TRUE;
	}

	if (label->stroke_enable)
	{
		if (label->is_stroke_update)
		{
			pi_text_update(label->stroke_text);
			label->is_stroke_update = FALSE;
			label->reprint = TRUE;
		}
	}

	if (label->reprint)
	{
		_update_bound(component, label);
		pi_component_reprint(component);
		label->reprint = FALSE;
	}
}
