#ifndef INCLUDE_RENDERCAP_H
#define INCLUDE_RENDERCAP_H

#include <pi_lib.h>
#include <renderformat.h>
#include <math.h> 

/**
 * ��Ⱦ����
 */

typedef enum
{
	VENDOR_UNKNOWN,
	VENDOR_INTEL,
	VENDOR_AMD,
	VENDOR_NVIDIA
}VendorType;

typedef enum
{
	CT_VENDOR_TYPE, 

	CT_MAX_SHADER_MODEL,

	CT_MAX_TEXTURE_WIDTH,
	CT_MAX_TEXTURE_HEIGHT,
	CT_MAX_TEXTURE_DEPTH,
	CT_MAX_TEXTURE_CUBE_SIZE,
	CT_MAX_TEXTURE_ARRAY_LENGTH,
	CT_MAX_VERTEX_TEXTURE_UNITS,
	CT_MAX_PIXEL_TEXTURE_UNITS,
	CT_MAX_SIMULTANEOUS_RTS,
	CT_MAX_VERTEX_STREAMS,
	CT_MAX_TEXTURE_ANISOTROPY,

	CT_MAX_SAMPLERS,

	CT_HW_INSTANCING_SUPPORT,
	CT_INSTANCE_ID_SUPPORT,
	CT_STREAM_OUTPUT_SUPPORT,
	CT_A2C_SUPPORT,
	CT_STANDARD_DERIVATIVES_SUPPORT,
	CT_NULL_RENDER_TARGET_SUPPORT
}CapType;

/* ����֧�ֵĸ�ʽ���� */
typedef enum
{
	CFT_VERTEX_FORMAT,	/* �����ʽ */
	CFT_TARGET_FORMAT,	/* Ŀ���ʽ */
	CFT_TEXTURE_FORMAT  /* �����ʽ */
}CapFormatType;

typedef struct 
{
	VendorType vendor_type;

	uint8 max_shader_model;

	uint32 max_texture_width;
	uint32 max_texture_height;
	uint32 max_texture_depth;
	uint32 max_texture_cube_size;
	uint32 max_texture_array_length;
	uint8 max_vertex_texture_units;
	uint8 max_pixel_texture_units;
	uint8 max_simultaneous_rts;
	uint8 max_vertex_streams;
	uint8 max_texture_anisotropy;
	
	uint max_samples;
	
	PiBool hw_instancing_support;
	PiBool instance_id_support;
	PiBool stream_output_support;
	PiBool alpha_to_coverage_support;
	PiBool standard_derivatives_support;
	PiBool null_render_target_support;
	
	PiVector vertex_formats;
	PiVector target_formats;
	PiVector texture_formats;
}PiRenderCap;

PI_BEGIN_DECLS 

uint PI_API pi_rendercap_get_int(PiRenderCap *cap, CapType type);

PiBool PI_API pi_rendercap_get_bool(PiRenderCap *cap, CapType type);

uint PI_API pi_rendercap_get_format_length(PiRenderCap *cap, CapFormatType type);

RenderFormat PI_API pi_rendercap_get_format(PiRenderCap *cap, CapFormatType type, uint index);

PiBool PI_API pi_rendercap_is_format_support(PiRenderCap *cap, CapFormatType type, RenderFormat format);

PiBool PI_API pi_rendercap_is_texture_format_support(RenderFormat format);

PI_END_DECLS 

#endif INCLUDE_RENDERCAP_H