#include <rendercap.h>
#include "rendersystem.h"

uint PI_API pi_rendercap_get_int(PiRenderCap *cap, CapType type)
{
	uint r = 0;
	switch (type)
	{
	case CT_VENDOR_TYPE:
		r = cap->vendor_type;
		break;
	case CT_MAX_SHADER_MODEL:
		r = cap->max_shader_model;
		break;
	case CT_MAX_TEXTURE_WIDTH:
		r = cap->max_texture_width;
		break;
	case CT_MAX_TEXTURE_HEIGHT:
		r = cap->max_texture_height;
		break;
	case CT_MAX_TEXTURE_DEPTH:
		r = cap->max_texture_depth;
		break;
	case CT_MAX_TEXTURE_CUBE_SIZE:
		r = cap->max_texture_cube_size;
		break;
	case CT_MAX_TEXTURE_ARRAY_LENGTH:
		r = cap->max_texture_array_length;
		break;
	case CT_MAX_VERTEX_TEXTURE_UNITS:
		r = cap->max_vertex_texture_units;
		break;
	case CT_MAX_PIXEL_TEXTURE_UNITS:
		r = cap->max_pixel_texture_units;
		break;
	case CT_MAX_SIMULTANEOUS_RTS:
		r = cap->max_simultaneous_rts;
		break;
	case CT_MAX_VERTEX_STREAMS:
		r = cap->max_vertex_streams;
		break;
	case CT_MAX_TEXTURE_ANISOTROPY:
		r = cap->max_texture_anisotropy;
		break;
	case CT_MAX_SAMPLERS:
		r = cap->max_samples;
		break;
	default:
		break;
	}
	return r;
}

PiBool PI_API pi_rendercap_get_bool(PiRenderCap *cap, CapType type)
{
	PiBool r = FALSE;
	switch (type)
	{
	case CT_HW_INSTANCING_SUPPORT:
		r = cap->hw_instancing_support;
		break;
	case CT_INSTANCE_ID_SUPPORT:
		r = cap->instance_id_support;
		break;
	case CT_STREAM_OUTPUT_SUPPORT:
		r = cap->stream_output_support;
		break;
	case CT_A2C_SUPPORT:
		r = cap->alpha_to_coverage_support;
		break;
	case CT_STANDARD_DERIVATIVES_SUPPORT:
		r = cap->standard_derivatives_support;
		break;
	case CT_NULL_RENDER_TARGET_SUPPORT:
		r = cap->null_render_target_support;
		break;
	default:
		break;
	}
	return r;
}

uint PI_API pi_rendercap_get_format_length(PiRenderCap *cap, CapFormatType type)
{
	uint length = 0;
	PiVector *vec = NULL;
	switch (type)
	{
	case CFT_VERTEX_FORMAT:
		vec = &cap->vertex_formats;
		break;
	case CFT_TARGET_FORMAT:
		vec = &cap->target_formats;
		break;
	case CFT_TEXTURE_FORMAT:
		vec = &cap->texture_formats;
		break;
	default:
		break;
	}

	if(vec != NULL)
	{
		length = pi_vector_size(vec);
	}
	return length;
}

RenderFormat PI_API pi_rendercap_get_format(PiRenderCap *cap, CapFormatType type, uint index)
{
	RenderFormat fmt = RF_UNKNOWN;
	PiVector *vec = NULL;
	switch (type)
	{
	case CFT_VERTEX_FORMAT:
		vec = &cap->vertex_formats;
		break;
	case CFT_TARGET_FORMAT:
		vec = &cap->target_formats;
		break;
	case CFT_TEXTURE_FORMAT:
		vec = &cap->texture_formats;
		break;
	default:
		break;
	}

	if(vec != NULL)
	{
		fmt = (RenderFormat)pi_vector_get(vec, index);
	}
	return fmt;
}

PiBool PI_API pi_rendercap_is_texture_format_support(RenderFormat format)
{
	PiRenderCap* cap = pi_rendersystem_get_cap();
	return pi_rendercap_is_format_support(cap, CFT_TEXTURE_FORMAT, format);
}

PiBool PI_API pi_rendercap_is_format_support(PiRenderCap *cap, CapFormatType type, RenderFormat format)
{
	PiBool r = FALSE;
	PiVector *vec = NULL;
	switch (type)
	{
	case CFT_VERTEX_FORMAT:
		vec = &cap->vertex_formats;
		break;
	case CFT_TARGET_FORMAT:
		vec = &cap->target_formats;
		break;
	case CFT_TEXTURE_FORMAT:
		vec = &cap->texture_formats;
		break;
	default:
		break;
	}

	if(vec != NULL)
	{
		uint i, length = pi_vector_size(vec);
		for(i = 0; i < length; ++i)
		{
			RenderFormat fmt = (RenderFormat)pi_vector_get(vec, i);
			if(fmt == format)
			{
				r = TRUE;
				break;
			}
		}
	}

	return r;
}