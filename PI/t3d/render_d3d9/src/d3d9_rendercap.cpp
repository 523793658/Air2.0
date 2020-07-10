#include "d3d9_rendercap.h"
#include "d3d9_rendersystem.h"

extern "C" extern PiRenderSystem *g_rsystem;
void d3d9_rendercap_init(PiRenderCap *cap)
{
	pi_memset_inline(cap, 0, sizeof(PiRenderCap));

	D3DCAPS9 caps;
	D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
	D3D9Context *context = d3d9_system->context;
	HRESULT hr;
	D3DDISPLAYMODE display_mode;
	IDirect3D9_GetAdapterDisplayMode(context->d3d, D3DADAPTER_DEFAULT, &display_mode);
	IDirect3DDevice9_GetDeviceCaps(context->device, &caps);
	
	cap->max_vertex_texture_units = 16;

	/* D3D 9.0c 支持 sm3.0 */
	cap->max_shader_model = 3;

	/* 纹理最大宽，高 */
	cap->max_texture_height = caps.MaxTextureWidth;
	cap->max_texture_width = caps.MaxTextureHeight;

	/* 3D纹理最大深度 */
	cap->max_texture_depth = caps.MaxVolumeExtent;

	/* cube纹理的大小 */
	cap->max_texture_cube_size = caps.MaxTextureWidth;

	cap->max_texture_array_length = 1;

	/*是否支持空rendertarget*/
	hr = IDirect3D9_CheckDeviceFormat(context->d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, display_mode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, FOURCC_NULL);
	cap->null_render_target_support = hr == S_OK;
	/* sm3.0 支持 16 层纹理 */
	cap->max_pixel_texture_units = 16;

	if (caps.RasterCaps | D3DPRASTERCAPS_ANISOTROPY)
	{
		cap->max_texture_anisotropy = (uint8)caps.MaxAnisotropy;
	}
	else
	{
		cap->max_texture_anisotropy = 1;
	}

	/* 多渲染目标数量 */
	cap->max_simultaneous_rts = (uint8)caps.NumSimultaneousRTs;

	/* VS输入变量的最大数量 */
	cap->max_vertex_streams = (uint8)caps.MaxStreams;

	cap->hw_instancing_support = TRUE;
	cap->instance_id_support = FALSE;
	cap->stream_output_support = FALSE;
	cap->alpha_to_coverage_support = TRUE;

	/* SM 3.0以下，dfdx，dfdy不支持 */
	cap->standard_derivatives_support = TRUE;

	D3DADAPTER_IDENTIFIER9 identifier;
	IDirect3D9_GetAdapterIdentifier(context->d3d, D3DADAPTER_DEFAULT, 0, &identifier);
	cap->vendor_type = VENDOR_UNKNOWN;

	pi_vector_init(&cap->vertex_formats);
	pi_vector_init(&cap->target_formats);
	pi_vector_init(&cap->texture_formats);

	pi_vector_push(&cap->vertex_formats,  (void *)RF_A8);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_R8);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_GR8);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_BGR8);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_ABGR8);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_R8UI);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_GR8UI);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_BGR8UI);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_ABGR8UI);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_SIGNED_R8);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_SIGNED_GR8);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_SIGNED_BGR8);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_SIGNED_ABGR8);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_R8I);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_GR8I);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_BGR8I);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_ABGR8I);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_A2BGR10);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_R16);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_GR16);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_BGR16);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_ABGR16);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_R16UI);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_GR16UI);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_BGR16UI);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_ABGR16UI);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_SIGNED_R16);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_SIGNED_GR16);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_SIGNED_BGR16);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_SIGNED_ABGR16);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_R16I);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_GR16I);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_BGR16I);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_ABGR16I);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_R32UI);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_GR32UI);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_BGR32UI);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_ABGR32UI);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_R32I);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_GR32I);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_BGR32I);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_ABGR32I);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_R32F);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_GR32F);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_BGR32F);
	pi_vector_push(&cap->vertex_formats,  (void *)RF_ABGR32F);

	pi_vector_push(&cap->vertex_formats, (void *)RF_R16F);
	pi_vector_push(&cap->vertex_formats, (void *)RF_GR16F);
	pi_vector_push(&cap->vertex_formats, (void *)RF_ABGR16F);

	pi_vector_push(&cap->texture_formats,  (void *)RF_A8);
	pi_vector_push(&cap->texture_formats,  (void *)RF_ARGB4);

	pi_vector_push(&cap->texture_formats,  (void *)RF_A2BGR10);
	pi_vector_push(&cap->texture_formats, (void *)RF_ABGR8);

	pi_vector_push(&cap->texture_formats, (void *)RF_GR16);
	pi_vector_push(&cap->texture_formats,  (void *)RF_ABGR16);

	pi_vector_push(&cap->texture_formats,  (void *)RF_R16F);
	pi_vector_push(&cap->texture_formats,  (void *)RF_GR16F);
	pi_vector_push(&cap->texture_formats, (void *)RF_ABGR16F);

	pi_vector_push(&cap->texture_formats,  (void *)RF_R32F);
	pi_vector_push(&cap->texture_formats,  (void *)RF_GR32F);
	pi_vector_push(&cap->texture_formats,  (void *)RF_ABGR32F);

	pi_vector_push(&cap->texture_formats, (void *)RF_BC1);
	pi_vector_push(&cap->texture_formats, (void *)RF_BC2);
	pi_vector_push(&cap->texture_formats, (void *)RF_BC3);

	pi_vector_push(&cap->texture_formats,  (void *)RF_D16);
	pi_vector_push(&cap->texture_formats, (void *)RF_D24S8);
	pi_vector_push(&cap->texture_formats,  (void *)RF_D32F);

	pi_vector_push(&cap->texture_formats,  (void *)RF_ARGB8_SRGB);
	pi_vector_push(&cap->texture_formats,  (void *)RF_ABGR8_SRGB);
	pi_vector_push(&cap->texture_formats, (void *)RF_BC1_SRGB);
	pi_vector_push(&cap->texture_formats, (void *)RF_BC2_SRGB);
	pi_vector_push(&cap->texture_formats, (void *)RF_BC3_SRGB);


	hr = IDirect3D9_CheckDeviceFormat(context->d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, display_mode.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, FOURCC_INTZ);
	/* sm3.0 支持 16 层纹理 */
	if (hr == D3D_OK)
	{
		pi_vector_push(&cap->texture_formats, (void*)RF_INTZ);
	}


	cap->max_samples = 1;

	pi_vector_push(&cap->target_formats, (void *)RF_A8);
	pi_vector_push(&cap->target_formats, (void *)RF_ARGB4);

	pi_vector_push(&cap->target_formats,  (void *)RF_A2BGR10);
	pi_vector_push(&cap->target_formats, (void *)RF_ABGR8);

	pi_vector_push(&cap->target_formats, (void *)RF_GR16);
	pi_vector_push(&cap->target_formats,  (void *)RF_ABGR16);

	pi_vector_push(&cap->target_formats, (void *)RF_R16F);
	pi_vector_push(&cap->target_formats, (void *)RF_GR16F);
	pi_vector_push(&cap->target_formats, (void *)RF_ABGR16F);

	pi_vector_push(&cap->target_formats, (void *)RF_R32F);
	pi_vector_push(&cap->target_formats, (void *)RF_GR32F);
	pi_vector_push(&cap->target_formats, (void *)RF_ABGR32F);

	pi_vector_push(&cap->target_formats,  (void *)RF_D16);
	pi_vector_push(&cap->target_formats, (void *)RF_D24S8);
	pi_vector_push(&cap->target_formats, (void *)RF_D32F);

	pi_vector_push(&cap->target_formats, (void *)RF_ARGB8_SRGB);
	pi_vector_push(&cap->target_formats, (void *)RF_ABGR8_SRGB);
}

void d3d9_rendercap_clear(PiRenderCap *cap)
{
	pi_vector_clear(&cap->vertex_formats, TRUE);
	pi_vector_clear(&cap->target_formats, TRUE);
	pi_vector_clear(&cap->texture_formats, TRUE);
}
