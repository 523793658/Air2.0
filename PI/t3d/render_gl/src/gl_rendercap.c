#include <gl_rendercap.h>
#include <gl_rendersystem.h>

#include <gl_interface.h>

void PI_API gl_rendercap_clear(PiRenderCap *cap)
{
	pi_vector_clear(&cap->vertex_formats, TRUE);
	pi_vector_clear(&cap->target_formats, TRUE);
	pi_vector_clear(&cap->texture_formats, TRUE);
}

void PI_API gl_rendercap_init(PiRenderCap *cap)
{
	sint temp;
	const char *vendor = (const char *)gl2_GetString(GL2_VENDOR);

	pi_memset_inline(cap, 0, sizeof(PiRenderCap));

	/* VS支持的最大纹理单元 */
	gl2_GetIntegerv(GL2_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &temp);
	cap->max_vertex_texture_units = (uint8)temp;
	
	/* 有VS纹理采样，至少是GL3.0 */
	if (cap->max_vertex_texture_units != 0)
	{
		if (gl_Self_IsGpuShader4())
		{
			cap->max_shader_model = 4;
		}
		else
		{
			cap->max_shader_model = 3;
		}
	}
	else
	{
			cap->max_shader_model = 2;
	}

	/* 纹理最大宽，高 */
	gl2_GetIntegerv(GL2_MAX_TEXTURE_SIZE, &temp);
	cap->max_texture_height = cap->max_texture_width = temp;
	
	/* 3D纹理最大深度 */
	if(gl_Self_IsTexture3D())
	{
		gl2_GetIntegerv(GL3_MAX_3D_TEXTURE_SIZE, &temp);
		cap->max_texture_depth = temp;
	}	

	/* cube纹理的大小 */
	gl2_GetIntegerv(GL2_MAX_CUBE_MAP_TEXTURE_SIZE, &temp);
	cap->max_texture_cube_size = temp;

	if (gl_Self_IsTextureArray())
	{
		/* 纹理数组的长度 */
		gl2_GetIntegerv(GL3_MAX_ARRAY_TEXTURE_LAYERS, &temp);
		cap->max_texture_array_length = temp;
	}
	else
	{
		cap->max_texture_array_length = 1;
	}

	/* 纹理单元数 */
	gl2_GetIntegerv(GL2_MAX_TEXTURE_IMAGE_UNITS, &temp);
	cap->max_pixel_texture_units = (uint8)temp;

	if (gl_Self_IsTextureFilterAnisotropic())
	{
		/* 各向异性过滤 */
		gl2_GetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &temp);
		cap->max_texture_anisotropy = (uint8)(temp);
	}
	else
	{
		cap->max_texture_anisotropy = 1;
	}

	/* 多渲染目标数量 */
	if(gl_Self_IsVersion3())
	{
		gl2_GetIntegerv(GL3_MAX_DRAW_BUFFERS, &temp);
		cap->max_simultaneous_rts = (uint8)(temp);
	}
	else
	{
		cap->max_simultaneous_rts = 1;
	}	
	
	/* VS输入变量的最大数量 */
	gl2_GetIntegerv(GL2_MAX_VERTEX_ATTRIBS, &temp);
	cap->max_vertex_streams = (uint8)(temp);

	cap->hw_instancing_support = TRUE;
	cap->instance_id_support = FALSE;
	cap->stream_output_support = FALSE;
	cap->alpha_to_coverage_support = TRUE;
	
	/* SM 3.0以下，dfdx，dfdy不支持 */
	if (cap->max_shader_model < 3)
	{
		cap->standard_derivatives_support = FALSE;
	}
	else
	{
		cap->standard_derivatives_support = TRUE;
	}

	if(pi_str_text_index(vendor, "NVIDIA") >= 0)
	{
		cap->vendor_type = VENDOR_NVIDIA;
	}
	
	if (pi_str_text_index(vendor, "ATI") >= 0)
	{
		cap->vendor_type = VENDOR_ATI;
	}
	if (pi_str_text_index(vendor, "AMD") >= 0)
	{
		cap->vendor_type = VENDOR_ATI;
	}
	if (pi_str_text_index(vendor, "Intel") >= 0)
	{
		cap->vendor_type = VENDOR_INTEL;
	}

	pi_vector_init(&cap->vertex_formats);
	pi_vector_init(&cap->target_formats);
	pi_vector_init(&cap->texture_formats);

	pi_vector_push(&cap->vertex_formats,  (void*)RF_A8);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_R8);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_GR8);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_BGR8);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_ABGR8);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_R8UI);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_GR8UI);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_BGR8UI);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_ABGR8UI);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_SIGNED_R8);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_SIGNED_GR8);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_SIGNED_BGR8);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_SIGNED_ABGR8);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_R8I);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_GR8I);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_BGR8I);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_ABGR8I);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_A2BGR10);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_R16);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_GR16);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_BGR16);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_ABGR16);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_R16UI);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_GR16UI);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_BGR16UI);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_ABGR16UI);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_SIGNED_R16);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_SIGNED_GR16);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_SIGNED_BGR16);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_SIGNED_ABGR16);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_R16I);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_GR16I);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_BGR16I);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_ABGR16I);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_R32UI);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_GR32UI);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_BGR32UI);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_ABGR32UI);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_R32I);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_GR32I);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_BGR32I);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_ABGR32I);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_R32F);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_GR32F);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_BGR32F);
	pi_vector_push(&cap->vertex_formats,  (void*)RF_ABGR32F);
	if (gl_Self_IsVertexType_2_10_10_10_rev())
	{
		pi_vector_push(&cap->vertex_formats,  (void*)RF_SIGNED_A2BGR10);
	}
	if (gl_Self_IsTextureRG())
	{
		pi_vector_push(&cap->vertex_formats,  (void*)RF_R16F);
		pi_vector_push(&cap->vertex_formats,  (void*)RF_GR16F);
		pi_vector_push(&cap->vertex_formats,  (void*)RF_BGR16F);
		pi_vector_push(&cap->vertex_formats,  (void*)RF_ABGR16F);
	}
		
	pi_vector_push(&cap->texture_formats,  (void*)RF_A8);
	pi_vector_push(&cap->texture_formats,  (void*)RF_ARGB4);
	pi_vector_push(&cap->texture_formats,  (void*)RF_R8);
	pi_vector_push(&cap->texture_formats,  (void*)RF_SIGNED_R8);
	if (gl_Self_IsTextureRG())
	{
		pi_vector_push(&cap->texture_formats,  (void*)RF_GR8);
		pi_vector_push(&cap->texture_formats,  (void*)RF_SIGNED_GR8);
		pi_vector_push(&cap->texture_formats,  (void*)RF_GR16);
		pi_vector_push(&cap->texture_formats,  (void*)RF_SIGNED_GR16);
	}
	pi_vector_push(&cap->texture_formats,  (void*)RF_BGR8);
	if (gl_Self_IsTextureSnorm())
	{
		pi_vector_push(&cap->texture_formats,  (void*)RF_SIGNED_BGR8);
		pi_vector_push(&cap->texture_formats,  (void*)RF_SIGNED_ABGR8);
	}
	pi_vector_push(&cap->texture_formats,  (void*)RF_ABGR8);
	pi_vector_push(&cap->texture_formats,  (void*)RF_A2BGR10);
	pi_vector_push(&cap->texture_formats,  (void*)RF_SIGNED_A2BGR10);
	pi_vector_push(&cap->texture_formats,  (void*)RF_R16);
	pi_vector_push(&cap->texture_formats,  (void*)RF_SIGNED_R16);
	if (gl_Self_IsTextureInteger())
	{
		pi_vector_push(&cap->texture_formats,  (void*)RF_R8UI);
		pi_vector_push(&cap->texture_formats,  (void*)RF_R8I);
		pi_vector_push(&cap->texture_formats,  (void*)RF_GR8UI);
		pi_vector_push(&cap->texture_formats,  (void*)RF_GR8I);
		pi_vector_push(&cap->texture_formats,  (void*)RF_BGR8UI);
		pi_vector_push(&cap->texture_formats,  (void*)RF_BGR8I);
		pi_vector_push(&cap->texture_formats,  (void*)RF_ABGR8UI);
		pi_vector_push(&cap->texture_formats,  (void*)RF_ABGR8I);
		pi_vector_push(&cap->texture_formats,  (void*)RF_R16UI);
		pi_vector_push(&cap->texture_formats,  (void*)RF_R16I);
		pi_vector_push(&cap->texture_formats,  (void*)RF_GR16UI);
		pi_vector_push(&cap->texture_formats,  (void*)RF_GR16I);
		pi_vector_push(&cap->texture_formats,  (void*)RF_BGR16UI);
		pi_vector_push(&cap->texture_formats,  (void*)RF_BGR16I);
		pi_vector_push(&cap->texture_formats,  (void*)RF_ABGR16UI);
		pi_vector_push(&cap->texture_formats,  (void*)RF_ABGR16I);
		pi_vector_push(&cap->texture_formats,  (void*)RF_R32UI);
		pi_vector_push(&cap->texture_formats,  (void*)RF_R32I);
		pi_vector_push(&cap->texture_formats,  (void*)RF_GR32UI);
		pi_vector_push(&cap->texture_formats,  (void*)RF_GR32I);
		pi_vector_push(&cap->texture_formats,  (void*)RF_BGR32UI);
		pi_vector_push(&cap->texture_formats,  (void*)RF_BGR32I);
		pi_vector_push(&cap->texture_formats,  (void*)RF_ABGR32UI);
		pi_vector_push(&cap->texture_formats,  (void*)RF_ABGR32I);
	}
	pi_vector_push(&cap->texture_formats,  (void*)RF_BGR16);
	pi_vector_push(&cap->texture_formats,  (void*)RF_SIGNED_BGR16);
	pi_vector_push(&cap->texture_formats,  (void*)RF_ABGR16);
	pi_vector_push(&cap->texture_formats,  (void*)RF_SIGNED_ABGR16);
	pi_vector_push(&cap->texture_formats,  (void*)RF_R16F);
	pi_vector_push(&cap->texture_formats,  (void*)RF_GR16F);
	if (gl_Self_IsPackedFloat())
	{
		pi_vector_push(&cap->texture_formats,  (void*)RF_B10G11R11F);
	}
	pi_vector_push(&cap->texture_formats,  (void*)RF_BGR16F);
	pi_vector_push(&cap->texture_formats,  (void*)RF_ABGR16F);
	pi_vector_push(&cap->texture_formats,  (void*)RF_R32F);
	pi_vector_push(&cap->texture_formats,  (void*)RF_GR32F);
	pi_vector_push(&cap->texture_formats,  (void*)RF_BGR32F);
	pi_vector_push(&cap->texture_formats,  (void*)RF_ABGR32F);
	if (gl_Self_IsTextureCompressionS3tc())
	{
		pi_vector_push(&cap->texture_formats,  (void*)RF_BC1);
		pi_vector_push(&cap->texture_formats,  (void*)RF_BC2);
		pi_vector_push(&cap->texture_formats,  (void*)RF_BC3);
	}
	if (gl_Self_IsTextureCompressionRgtc())
	{
		pi_vector_push(&cap->texture_formats,  (void*)RF_BC4);
		pi_vector_push(&cap->texture_formats,  (void*)RF_BC5);
		pi_vector_push(&cap->texture_formats,  (void*)RF_SIGNED_BC4);
		pi_vector_push(&cap->texture_formats,  (void*)RF_SIGNED_BC5);
	}
	if (gl_Self_IsTextureCompressionBptc())
	{
		pi_vector_push(&cap->texture_formats,  (void*)RF_BC6);
		pi_vector_push(&cap->texture_formats,  (void*)RF_BC7);
	}
	pi_vector_push(&cap->texture_formats,  (void*)RF_D16);
	if (gl_Self_IsPackedDepthStencil())
	{
		pi_vector_push(&cap->texture_formats,  (void*)RF_D24);
	}
	pi_vector_push(&cap->texture_formats,  (void*)RF_D32F);
	pi_vector_push(&cap->texture_formats,  (void*)RF_ARGB8_SRGB);
	pi_vector_push(&cap->texture_formats,  (void*)RF_ABGR8_SRGB);
	if (gl_Self_IsTextureCompressionS3tc())
	{
		pi_vector_push(&cap->texture_formats,  (void*)RF_BC1_SRGB);
		pi_vector_push(&cap->texture_formats,  (void*)RF_BC2_SRGB);
		pi_vector_push(&cap->texture_formats,  (void*)RF_BC3_SRGB);
	}
	if (gl_Self_IsTextureCompressionRgtc())
	{
		pi_vector_push(&cap->texture_formats,  (void*)RF_BC4_SRGB);
		pi_vector_push(&cap->texture_formats,  (void*)RF_BC5_SRGB);
	}
	
	cap->max_samples = 1;
	
	if (gl_Self_IsTextureRG())
	{
		pi_vector_push(&cap->target_formats,  (void*)RF_R8);
		pi_vector_push(&cap->target_formats,  (void*)RF_GR8);
	}
	pi_vector_push(&cap->target_formats,  (void*)RF_ABGR8);
	if (gl_Self_IsTextureSnorm())
	{
		pi_vector_push(&cap->target_formats,  (void*)RF_SIGNED_ABGR8);
	}
	pi_vector_push(&cap->target_formats,  (void*)RF_A2BGR10);
	pi_vector_push(&cap->target_formats,  (void*)RF_SIGNED_A2BGR10);
	if (gl_Self_IsTextureRG())
	{
		pi_vector_push(&cap->target_formats,  (void*)RF_ABGR8UI);
		pi_vector_push(&cap->target_formats,  (void*)RF_ABGR8I);
		pi_vector_push(&cap->target_formats,  (void*)RF_R16);
		pi_vector_push(&cap->target_formats,  (void*)RF_SIGNED_R16);
		pi_vector_push(&cap->target_formats,  (void*)RF_GR16);
		pi_vector_push(&cap->target_formats,  (void*)RF_SIGNED_GR16);
	}
	pi_vector_push(&cap->target_formats,  (void*)RF_ABGR16);
	pi_vector_push(&cap->target_formats,  (void*)RF_SIGNED_ABGR16);
	if (gl_Self_IsTextureInteger())
	{
		pi_vector_push(&cap->target_formats,  (void*)RF_R16UI);
		pi_vector_push(&cap->target_formats,  (void*)RF_R16I);
		pi_vector_push(&cap->target_formats,  (void*)RF_GR16UI);
		pi_vector_push(&cap->target_formats,  (void*)RF_GR16I);
		pi_vector_push(&cap->target_formats,  (void*)RF_ABGR16UI);
		pi_vector_push(&cap->target_formats,  (void*)RF_ABGR16I);
		pi_vector_push(&cap->target_formats,  (void*)RF_R32UI);
		pi_vector_push(&cap->target_formats,  (void*)RF_R32I);
		pi_vector_push(&cap->target_formats,  (void*)RF_GR32UI);
		pi_vector_push(&cap->target_formats,  (void*)RF_GR32I);
		pi_vector_push(&cap->target_formats,  (void*)RF_ABGR32UI);
		pi_vector_push(&cap->target_formats,  (void*)RF_ABGR32I);
	}
	if ((gl_Self_IsHalfFloatPixel() && gl_Self_IsTextureRG()))
	{
		pi_vector_push(&cap->target_formats,  (void*)RF_R16F);
		pi_vector_push(&cap->target_formats,  (void*)RF_GR16F);
		pi_vector_push(&cap->target_formats,  (void*)RF_R32F);
		pi_vector_push(&cap->target_formats,  (void*)RF_GR32F);
	}
	if (gl_Self_IsHalfFloatPixel())
	{
		pi_vector_push(&cap->target_formats,  (void*)RF_ABGR16F);
	}
	if (gl_Self_IsPackedFloat())
	{
		pi_vector_push(&cap->target_formats,  (void*)RF_B10G11R11F);
	}
	if (gl_Self_IsTextureFloat())
	{
		pi_vector_push(&cap->target_formats,  (void*)RF_ABGR32F);
	}
	pi_vector_push(&cap->target_formats,  (void*)RF_D16);
	if (gl_Self_IsPackedDepthStencil())
	{
		pi_vector_push(&cap->target_formats,  (void*)RF_D24);
	}
	pi_vector_push(&cap->target_formats,  (void*)RF_D32F);
	if (gl_Self_IsFramebufferSRGB())
	{
		pi_vector_push(&cap->target_formats,  (void*)RF_ARGB8_SRGB);
		pi_vector_push(&cap->target_formats,  (void*)RF_ABGR8_SRGB);
	}
}