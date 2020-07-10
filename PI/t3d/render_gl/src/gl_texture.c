#include <gl_texture.h>
#include <gl_convert.h>
#include <gl_renderstate.h>
#include <gl_rendersystem.h>
#include <renderinfo.h>

#include <renderwrap.h>
#include <gl_interface.h>

static void _init_sampler_state(GLTexture *impl, SamplerState *ss)
{
	uint val;
	uint min_filter, mag_filter;

	val = gl_tex_addr_get(ss->addr_mode_u);
	gl2_TexParameteri(impl->gl_target, GL2_TEXTURE_WRAP_S, val);

	val = gl_tex_addr_get(ss->addr_mode_v);
	gl2_TexParameteri(impl->gl_target, GL2_TEXTURE_WRAP_T, val);

	if (impl->gl_target == GL3_TEXTURE_3D && gl_Self_IsTexture3D())
	{
		val = gl_tex_addr_get(ss->addr_mode_w);
		gl2_TexParameteri(impl->gl_target, GL3_TEXTURE_WRAP_R, val);
	}

	if (gl_Self_GetInterfaceType() == RIT_OPENGL)
		gl2_TexParameterfv(impl->gl_target, GL_TEXTURE_BORDER_COLOR, ss->border_clr.rgba);

	gl_tex_filter_get(ss->filter, &min_filter, &mag_filter);
	gl2_TexParameteri(impl->gl_target, GL2_TEXTURE_MAG_FILTER, mag_filter);
	gl2_TexParameteri(impl->gl_target, GL2_TEXTURE_MIN_FILTER, min_filter);

	// {
	// 	sint depth_stencil_texture_mode = GL2_STENCIL_INDEX;
	// 	gl2_TexParameteriv(impl->gl_target, GL_DEPTH_STENCIL_TEXTURE_MODE, &depth_stencil_texture_mode);
	// }

	if (gl_Self_IsTextureFilterAnisotropic())
	{
		if (ss->filter & TFOE_ANISOTROPIC)
		{
			gl2_TexParameteri(impl->gl_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, ss->max_anisotropy);
		}
		else
		{
			gl2_TexParameteri(impl->gl_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
		}
	}

	if(gl_Self_IsTextureLOD())
	{
		gl2_TexParameterf(impl->gl_target, GL3_TEXTURE_MIN_LOD, ss->min_lod);
		gl2_TexParameterf(impl->gl_target, GL3_TEXTURE_MAX_LOD, ss->max_lod);

		val = gl_compare_func_get(ss->cmp_func);
		if (ss->cmp_func != CF_ALWAYSFAIL)
		{
			gl2_TexParameteri(impl->gl_target, GL3_TEXTURE_COMPARE_MODE, GL3_COMPARE_REF_TO_TEXTURE);
		}
		else
		{
			gl2_TexParameteri(impl->gl_target, GL3_TEXTURE_COMPARE_MODE, GL2_NONE);
		}
		gl2_TexParameteri(impl->gl_target, GL3_TEXTURE_COMPARE_FUNC, val);
	}

	if(gl_Self_GetInterfaceType() == RIT_OPENGL)
	{
		gl2_TexParameterf(impl->gl_target, GL_TEXTURE_LOD_BIAS, ss->mip_map_lod_bias);
	}	
}

static PiBool _init_2d_texture(PiTexture *texture, GLTexture *impl)
{
	uint level;

	for (level = 0; level < texture->num_mipmap; ++level)
	{
		uint width = texture->level_size[level].width;
		uint height = texture->level_size[level].height;

		if (texture->is_compressed_format)
		{
			uint data_size = ((width + 3) / 4) * ((height + 3) / 4) * texture->compress_block_size;
			
			if (texture->array_size > 1)
			{
				gl3_CompressedTexImage3D(impl->gl_target, level, impl->gl_internal_fmt, width, height, texture->array_size, 0, data_size, NULL);
			}
			else
			{
				gl2_CompressedTexImage2D(impl->gl_target, level, impl->gl_internal_fmt, width, height, 0, data_size, NULL);
			}
		}
		else
		{
			if (texture->array_size > 1)
			{
				gl3_TexImage3D(impl->gl_target, level, impl->gl_internal_fmt, width, height, texture->array_size, 0, impl->gl_fmt, impl->gl_type, NULL);
			}
			else
			{
				gl2_TexImage2D(impl->gl_target, level, impl->gl_internal_fmt, width, height, 0, impl->gl_fmt, impl->gl_type, NULL);
			}
		}
	}
	return TRUE;
}

static PiBool _init_3d_texture(PiTexture *texture, GLTexture *impl)
{
	uint level;
	
	for (level = 0; level < texture->num_mipmap; ++level)
	{
		uint width = texture->level_size[level].width;
		uint height = texture->level_size[level].height;
		uint depth = texture->level_size[level].depth;

		if (texture->is_compressed_format)
		{
			uint data_size = ((width + 3) / 4) * ((height + 3) / 4) * depth * texture->compress_block_size;
			gl3_CompressedTexImage3D(impl->gl_target, level, impl->gl_internal_fmt, width, height, depth, 0, data_size, NULL);
		}
		else
		{
			gl3_TexImage3D(impl->gl_target, level, impl->gl_internal_fmt, width, height, depth, 0, impl->gl_fmt, impl->gl_type, NULL);
		}
	}
	return TRUE;
}

static PiBool _init_cube_texture(PiTexture *texture, GLTexture *impl)
{
	uint face, level;

	for(face = 0; face < 6; ++ face)
	{
		for (level = 0; level < texture->num_mipmap; ++level)
		{
			uint width = texture->level_size[level].width;

			if (texture->is_compressed_format)
			{
				uint data_size = ((width + 3) / 4) * ((width + 3) / 4) * texture->compress_block_size;

				width = (width + 3) / 4 * 4;
				
				gl2_CompressedTexImage2D(GL2_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, impl->gl_internal_fmt, width, width, 0, data_size, NULL);
			}
			else
			{
				gl2_TexImage2D(GL2_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, impl->gl_internal_fmt, width, width, 0, impl->gl_fmt, impl->gl_type, NULL);
			}
		}
	}
	return TRUE;
}

static void _init_texture_state(PiTexture *texture)
{
	GLTexture *impl = texture->impl;

	glstate_bind_texture(texture);

	gl2_PixelStorei(GL2_PACK_ALIGNMENT, 1);
	gl2_PixelStorei(GL2_UNPACK_ALIGNMENT, 1);

	pi_renderstate_set_default_sampler(&impl->curr_ss);
	impl->curr_ss.tex = texture;
	_init_sampler_state(impl, &impl->curr_ss);
}

PiBool PI_API render_texture_init(PiTexture *texture)
{
	PiBool r = FALSE;
	GLTexture *impl = NULL;
	
	pi_renderinfo_add_texture_num(1);
	if ((texture->array_size > 1) && (!gl_Self_IsTextureArray()))
	{
		pi_log_print(LOG_ERROR, "texture array isn't support");
		return FALSE;
	}

	texture->impl = impl = pi_new0(GLTexture, 1);

	impl->gl_target = gl_tex_target_get(texture->type, texture->array_size);

	gl2_GenTextures(1, &impl->gl_id);
	
	gl_tex_format_get(texture->format, &impl->gl_internal_fmt, &impl->gl_fmt, &impl->gl_type);

	_init_texture_state(texture);
	
	if(gl_Self_IsVersion3())
	{
		gl2_TexParameteri(impl->gl_target, GL3_TEXTURE_MAX_LEVEL, texture->num_mipmap - 1);
	}	
	
	switch (texture->type)
	{
	case TT_2D:
		r = _init_2d_texture(texture, impl);
		break;
	case TT_3D:
		r = _init_3d_texture(texture, impl);
		break;
	case TT_CUBE:
		r = _init_cube_texture(texture, impl);
		break;
	default:
		break;
	}
	return r;
}

PiBool PI_API render_texture_clear(PiTexture *texture)
{
	GLTexture *impl = texture->impl;
	
	pi_renderinfo_add_texture_num(-1);
	glstate_remove_texture(texture);

	if(impl != NULL)
	{
		gl2_DeleteTextures(1, &impl->gl_id);
	}
	
	pi_free(impl);
	return TRUE;
}

PiBool PI_API render_texture_build_mipmap(PiTexture *texture)
{
	GLTexture *impl = texture->impl;
	glstate_bind_texture(texture);
	gl2_GenerateMipmap(impl->gl_target);
	return TRUE;
}

PiBool PI_API render_texture_2d_update(PiTexture *texture, uint array_index, uint level, uint x, uint y, uint w, uint h, uint data_size, byte *data)
{
	GLTexture *impl = texture->impl;
	glstate_bind_texture(texture);
	if (texture->is_compressed_format)
	{
		uint data_size = ((w + 3) / 4) * ((h + 3) / 4) * texture->compress_block_size;

		if (texture->array_size > 1)
		{
			gl3_CompressedTexSubImage3D(impl->gl_target, level, x, y, array_index, w, h, 1, impl->gl_internal_fmt, data_size, data);
		}
		else
		{
			gl2_CompressedTexSubImage2D(impl->gl_target, level, x, y, w, h, impl->gl_internal_fmt, data_size, data);
		}
	}
	else
	{
		if (texture->array_size > 1)
		{
			gl3_TexSubImage3D(impl->gl_target, level, x, y, array_index, w, h, 1, impl->gl_fmt, impl->gl_type, data);
		}
		else
		{
			gl2_TexSubImage2D(impl->gl_target, level, x, y, w, h, impl->gl_fmt, impl->gl_type, data);
		}
	}

	return TRUE;
}

PiBool PI_API render_texture_3d_update(PiTexture *texture, uint level, uint x, uint y, uint z, uint w, uint h, uint d, uint data_size, byte *data)
{
	GLTexture *impl = texture->impl;
	glstate_bind_texture(texture);
	if (texture->is_compressed_format)
	{
		uint data_size = d * ((w + 3) / 4) * ((h + 3) / 4) * texture->compress_block_size;
		gl3_CompressedTexSubImage3D(impl->gl_target, level, x, y, z, w, h, d, impl->gl_internal_fmt, data_size, data);
	}
	else
	{
		gl3_TexSubImage3D(impl->gl_target, level, x, y, z, w, h, d, impl->gl_fmt, impl->gl_type, data);
	}
	return TRUE;
}

PiBool PI_API render_texture_cube_update(PiTexture *texture, uint level, TextureCubeFace face, uint x, uint y, uint w, uint h, uint data_size, byte *data)
{
	GLTexture *impl = texture->impl;
	glstate_bind_texture(texture);
	
	face -= CF_POSITIVE_X;
	face += GL2_TEXTURE_CUBE_MAP_POSITIVE_X;

	if (texture->is_compressed_format)
	{
		uint data_size = ((w + 3) / 4) * ((h + 3) / 4) * texture->compress_block_size;
		gl2_CompressedTexSubImage2D(face, level, x, y, w, h, impl->gl_internal_fmt, data_size, data);
	}
	else
	{
		gl2_TexSubImage2D(face, level, x, y, w, h, impl->gl_fmt, impl->gl_type, data);
	}

	return TRUE;
}

PiImage* PI_API render_texture_2d_get(PiTexture *texture, uint array_index, uint level, uint x, uint y, uint w, uint h)
{
	byte *data = NULL;
	PiImage *img = NULL;
	GLTexture *impl = texture->impl;

	uint texel_size = pi_renderformat_get_numbytes(texture->format);
	uint rpitch = texel_size * texture->level_size[level].width;

	glstate_bind_texture(texture);

	data = pi_malloc(rpitch * texture->level_size[level].height);

	if (texture->is_compressed_format)
	{
		gl_GetCompressedTexImage(impl->gl_target, level, data);
	}
	else
	{
		PiRenderSystem *system = pi_rendersystem_get_instance();
		GLRenderSystem *system_impl = system->impl;

		uint old_fbo = glstate_bind_fbo(0);
		gl2_BindFramebuffer(GL2_FRAMEBUFFER, system_impl->blit_src_fbo);

		if (texture->array_size > 1)
		{
			gl3_FramebufferTextureLayer(GL2_FRAMEBUFFER, GL2_COLOR_ATTACHMENT0, impl->gl_id, level, array_index);
		}
		else
		{
			if(pi_renderformat_is_depth_format(texture->format))
			{
				gl2_FramebufferTexture2D(GL2_FRAMEBUFFER, GL2_DEPTH_ATTACHMENT, impl->gl_target, impl->gl_id, level);
				gl2_FramebufferTexture2D(GL2_FRAMEBUFFER, GL2_COLOR_ATTACHMENT0, impl->gl_target, 0, 0);
			}
			else
			{
				gl2_FramebufferTexture2D(GL2_FRAMEBUFFER, GL2_COLOR_ATTACHMENT0, impl->gl_target, impl->gl_id, level);
				gl2_FramebufferTexture2D(GL2_FRAMEBUFFER, GL2_DEPTH_ATTACHMENT, impl->gl_target, 0, 0);
			}
		}

		gl2_ReadPixels(x, y, w, h, impl->gl_fmt, impl->gl_type, data);
		gl2_BindFramebuffer(GL2_FRAMEBUFFER, 0);
		glstate_bind_fbo(old_fbo);
	}
	
	img = pi_render_image_new(texture->level_size[level].width, texture->level_size[level].height, texture->format, data);
	pi_free(data);
	return img;
}

PiImage* PI_API render_texture_cube_get(PiTexture *texture, uint level, TextureCubeFace face, uint x, uint y, uint w, uint h)
{
	byte *data = NULL;
	PiImage *img = NULL;
	GLTexture *impl = texture->impl;

	uint texel_size = pi_renderformat_get_numbytes(texture->format);
	uint rpitch = texel_size * texture->level_size[level].width;

	glstate_bind_texture(texture);

	data = pi_malloc(rpitch * texture->level_size[level].height);
	glstate_bind_texture(texture);

	if (texture->is_compressed_format)
	{
		gl_GetCompressedTexImage(GL2_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, NULL);
	}
	else
	{
		gl_GetTexImage(GL2_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, impl->gl_fmt, impl->gl_type, NULL);
	}
	
	img = pi_render_image_new(texture->level_size[level].width, texture->level_size[level].height, texture->format, data);
	pi_free(data);
	return img;
}

PiBool PI_API render_texture_2d_copy(PiTexture *dst, PiTexture *src,
	uint dst_array_index, uint dst_level, uint dst_x, uint dst_y,
	uint src_array_index, uint src_level, uint src_x, uint src_y, uint w, uint h)
{
	GLTexture *src_impl = src->impl;
	GLTexture *dst_impl = dst->impl;
	
	PiRenderSystem *system = pi_rendersystem_get_instance();
	GLRenderSystem *system_impl = system->impl;

	if(gl_Self_IsCopyImage())
	{
		gl_CopyImageSubData(
			src_impl->gl_id, src_impl->gl_target, src_level, src_x, src_y, src_array_index,
			dst_impl->gl_id, dst_impl->gl_target, dst_level, dst_x, dst_y, dst_array_index, w, h, 1);
	}
	else
	{
		if (gl_Self_IsFramebufferBlit()
			&& (gl_Self_IsTextureRG() || 4 == pi_renderformat_get_numcomponents(dst->format)) )
		{
			uint old_fbo = glstate_bind_fbo(0);

			gl2_BindFramebuffer(GL3_READ_FRAMEBUFFER, system_impl->blit_src_fbo);
			if (src->array_size > 1)
			{
				gl3_FramebufferTextureLayer(GL3_READ_FRAMEBUFFER, GL2_COLOR_ATTACHMENT0, src_impl->gl_id, src_level, src_array_index);
			}
			else
			{
				if(pi_renderformat_is_depth_format(src->format))
				{
					gl2_FramebufferTexture2D(GL3_READ_FRAMEBUFFER, GL2_DEPTH_ATTACHMENT, src_impl->gl_target, src_impl->gl_id, src_level);
					gl2_FramebufferTexture2D(GL3_READ_FRAMEBUFFER, GL2_COLOR_ATTACHMENT0, src_impl->gl_target, 0, 0);
				}
				else
				{
					gl2_FramebufferTexture2D(GL3_READ_FRAMEBUFFER, GL2_COLOR_ATTACHMENT0, src_impl->gl_target, src_impl->gl_id, src_level);
					gl2_FramebufferTexture2D(GL3_READ_FRAMEBUFFER, GL2_DEPTH_ATTACHMENT, src_impl->gl_target, 0, 0);
				}
			}
			
			gl2_BindFramebuffer(GL3_DRAW_FRAMEBUFFER, system_impl->blit_dst_fbo);
			if (dst->array_size > 1)
			{
				gl3_FramebufferTextureLayer(GL3_DRAW_FRAMEBUFFER, GL2_COLOR_ATTACHMENT0, dst_impl->gl_id, dst_level, dst_array_index);
			}
			else
			{
				if(pi_renderformat_is_depth_format(dst->format))
				{
					gl2_FramebufferTexture2D(GL3_DRAW_FRAMEBUFFER, GL2_DEPTH_ATTACHMENT, dst_impl->gl_target, dst_impl->gl_id, src_level);
					gl2_FramebufferTexture2D(GL3_DRAW_FRAMEBUFFER, GL2_COLOR_ATTACHMENT0, dst_impl->gl_target, 0, 0);
				}
				else
				{
					gl2_FramebufferTexture2D(GL3_DRAW_FRAMEBUFFER, GL2_COLOR_ATTACHMENT0, dst_impl->gl_target, dst_impl->gl_id, src_level);
					gl2_FramebufferTexture2D(GL3_DRAW_FRAMEBUFFER, GL2_DEPTH_ATTACHMENT, dst_impl->gl_target, 0, 0);
				}
			}

			gl3_BlitFramebuffer(src_x, src_y, src_x + w, src_y + h, dst_x, dst_y, dst_x + w, dst_y + h, GL2_COLOR_BUFFER_BIT, GL2_NEAREST);

			gl2_BindFramebuffer(GL3_READ_FRAMEBUFFER, 0);
			gl2_BindFramebuffer(GL3_DRAW_FRAMEBUFFER, 0);
			glstate_bind_fbo(old_fbo);
		}
		else
		{
			uint old_fbo = glstate_bind_fbo(0);
			gl2_BindFramebuffer(GL2_FRAMEBUFFER, system_impl->blit_src_fbo);
			if (src->array_size > 1)
			{
				gl3_FramebufferTextureLayer(GL2_FRAMEBUFFER, GL2_COLOR_ATTACHMENT0, src_impl->gl_id, src_level, src_array_index);
			}
			else
			{
				gl2_FramebufferTexture2D(GL2_FRAMEBUFFER, GL2_COLOR_ATTACHMENT0, GL2_TEXTURE_2D, src_impl->gl_id, src_level);
			}
			
			glstate_bind_texture(dst);
			gl2_CopyTexSubImage2D(dst_impl->gl_target, dst_level, dst_x, dst_y, src_x, src_y, w, h);
			gl2_BindFramebuffer(GL2_FRAMEBUFFER, 0);
			glstate_bind_fbo(old_fbo);
		}
	}
	return TRUE;
}

PiBool PI_API render_texture_3d_copy(PiTexture *dst, PiTexture *src,
	uint dst_level, uint dst_x, uint dst_y, uint dst_z, 
	uint src_level, uint src_x, uint src_y, uint src_z, uint w, uint h, uint d)
{
	GLTexture *src_impl = src->impl;
	GLTexture *dst_impl = dst->impl;

	PiRenderSystem *system = pi_rendersystem_get_instance();
	GLRenderSystem *system_impl = system->impl;

	if (gl_Self_IsCopyImage())
	{
		gl_CopyImageSubData(
			src_impl->gl_id, src_impl->gl_target, src_level,
			src_x, src_y, src_z,
			dst_impl->gl_id, dst_impl->gl_target, dst_level,
			dst_x, dst_y, dst_z, w, h, d);
	}
	else
	{
		if (gl_Self_IsFramebufferBlit()
			&& (gl_Self_IsTextureRG() || 4 == pi_renderformat_get_numcomponents(dst->format)) )
		{
			uint32 depth;
			uint old_fbo = glstate_bind_fbo(0);
			for (depth = 0; depth < d; ++depth)
			{
				gl2_BindFramebuffer(GL3_READ_FRAMEBUFFER, system_impl->blit_src_fbo);
				gl_FramebufferTexture3D(GL3_READ_FRAMEBUFFER, GL2_COLOR_ATTACHMENT0, src_impl->gl_target, src_impl->gl_id, src_level, src_z + depth);

				gl2_BindFramebuffer(GL3_DRAW_FRAMEBUFFER, system_impl->blit_dst_fbo);
				gl_FramebufferTexture3D(GL3_DRAW_FRAMEBUFFER, GL2_COLOR_ATTACHMENT0, dst_impl->gl_target, dst_impl->gl_id, dst_level, dst_z + depth);

				gl3_BlitFramebuffer(src_x, src_y, src_x + w, src_y + h, dst_x, dst_y, dst_x + w, dst_y + h, GL2_COLOR_BUFFER_BIT, GL2_NEAREST);
			}
			gl2_BindFramebuffer(GL3_READ_FRAMEBUFFER, 0);
			gl2_BindFramebuffer(GL3_DRAW_FRAMEBUFFER, 0);
			glstate_bind_fbo(old_fbo);
		}
		else
		{
			uint i;
			uint old_fbo = glstate_bind_fbo(0);
			gl2_BindFramebuffer(GL2_FRAMEBUFFER, system_impl->blit_src_fbo);
			
			glstate_bind_texture(dst);

			for(i = 0; i < d; ++i)
			{
				gl_FramebufferTexture3D(GL2_FRAMEBUFFER, GL2_COLOR_ATTACHMENT0, src_impl->gl_target, src_impl->gl_id, src_level, src_z + i);
				gl3_CopyTexSubImage3D(dst_impl->gl_target, dst_level, dst_x, dst_y, dst_z, src_x, src_y, w, h);
			}
			gl2_BindFramebuffer(GL2_FRAMEBUFFER, 0);
			glstate_bind_fbo(old_fbo);
		}
	}
	return TRUE;
}

PiBool PI_API render_texture_cube_copy(PiTexture *dst, PiTexture *src,
	TextureCubeFace dst_face, uint dst_level, uint dst_x, uint dst_y, 
	TextureCubeFace src_face, uint src_level, uint src_x, uint src_y, uint w, uint h)
{
	GLTexture *src_impl = src->impl;
	GLTexture *dst_impl = dst->impl;

	PiRenderSystem *system = pi_rendersystem_get_instance();
	GLRenderSystem *system_impl = system->impl;

	if (gl_Self_IsCopyImage())
	{
		gl_CopyImageSubData(
			src_impl->gl_id, src_impl->gl_target, src_level,
			src_x, src_y, src_face - CF_POSITIVE_X,
			dst_impl->gl_id, dst_impl->gl_target, dst_level,
			dst_x, dst_y, dst_face - CF_POSITIVE_X, w, h, 1);
	}
	else
	{
		if (gl_Self_IsFramebufferBlit()
			&& (gl_Self_IsTextureRG() || 4 == pi_renderformat_get_numcomponents(dst->format)) )
		{
			uint old_fbo = glstate_bind_fbo(0);

			gl2_BindFramebuffer(GL3_READ_FRAMEBUFFER, system_impl->blit_src_fbo);
			gl2_FramebufferTexture2D(GL3_READ_FRAMEBUFFER, GL2_COLOR_ATTACHMENT0, GL2_TEXTURE_CUBE_MAP_POSITIVE_X + src_face - CF_POSITIVE_X, src_impl->gl_id, src_level);
			
			gl2_BindFramebuffer(GL3_DRAW_FRAMEBUFFER, system_impl->blit_dst_fbo);
			gl2_FramebufferTexture2D(GL3_DRAW_FRAMEBUFFER, GL2_COLOR_ATTACHMENT0, GL2_TEXTURE_CUBE_MAP_POSITIVE_X + dst_face - CF_POSITIVE_X, dst_impl->gl_id, dst_level);
			
			gl3_BlitFramebuffer(src_x, src_y, src_x + w, src_y + h, dst_x, dst_y, dst_x + w, dst_y + h, GL2_COLOR_BUFFER_BIT, GL2_NEAREST);
			gl2_BindFramebuffer(GL3_READ_FRAMEBUFFER, 0);
			gl2_BindFramebuffer(GL3_DRAW_FRAMEBUFFER, 0);
			glstate_bind_fbo(old_fbo);
		}
		else
		{
			uint i;
			uint old_fbo = glstate_bind_fbo(0);
			gl2_BindFramebuffer(GL2_FRAMEBUFFER, system_impl->blit_src_fbo);

			glstate_bind_texture(dst);
			
			for(i = 0; i < 6; ++i)
			{
				uint face = i + GL2_TEXTURE_CUBE_MAP_POSITIVE_X;
				gl2_FramebufferTexture2D(GL2_FRAMEBUFFER, GL2_COLOR_ATTACHMENT0, face, src_impl->gl_id, src_level);
				gl2_CopyTexSubImage2D(face, dst_level, dst_x, dst_y, src_x, src_y, w, h);
			}			
			gl2_BindFramebuffer(GL2_FRAMEBUFFER, 0);
			glstate_bind_fbo(old_fbo);
		}
	}
	return TRUE;
}

void* PI_API render_texture_get_curr_sampler(PiTexture *tex)
{
	GLTexture *impl = tex->impl;
	return &impl->curr_ss;
}

PiBool gl_texture_set_sampler(SamplerState *ss)
{
	PiTexture *tex;
	GLTexture *impl;
	SamplerState *curr_ss;
	if (ss->tex == NULL)
	{
		return FALSE;
	}

	tex = ss->tex;
	impl = tex->impl;
	curr_ss = &impl->curr_ss;
	glstate_bind_texture(tex);

	if(curr_ss->addr_mode_u != ss->addr_mode_u)
	{
		uint u = gl_tex_addr_get(ss->addr_mode_u);
		curr_ss->addr_mode_u = ss->addr_mode_u;
		gl2_TexParameteri(impl->gl_target, GL2_TEXTURE_WRAP_S, u);
	}

	if(curr_ss->addr_mode_v != ss->addr_mode_v)
	{
		uint v = gl_tex_addr_get(ss->addr_mode_v);
		curr_ss->addr_mode_v = ss->addr_mode_v;
		gl2_TexParameteri(impl->gl_target, GL2_TEXTURE_WRAP_T, v);
	}

	if (impl->gl_target == GL3_TEXTURE_3D && gl_Self_IsTexture3D())
	{
		if(curr_ss->addr_mode_w != ss->addr_mode_w)
		{
			uint w = gl_tex_addr_get(ss->addr_mode_w);
			curr_ss->addr_mode_w = ss->addr_mode_w;
			gl2_TexParameteri(impl->gl_target, GL3_TEXTURE_WRAP_R, w);
		}		
	}

	if(!color_is_equal(&curr_ss->border_clr, &ss->border_clr))
	{
		color_copy(&curr_ss->border_clr, &ss->border_clr);

		if(gl_Self_GetInterfaceType() == RIT_OPENGL)
		{
			gl2_TexParameterfv(impl->gl_target, GL_TEXTURE_BORDER_COLOR, ss->border_clr.rgba);
		}
	}

	if(curr_ss->filter != ss->filter)
	{
		uint min_filter, mag_filter;

		curr_ss->filter = ss->filter;
		gl_tex_filter_get(ss->filter, &min_filter, &mag_filter);

		gl2_TexParameteri(impl->gl_target, GL2_TEXTURE_MAG_FILTER, mag_filter);
		gl2_TexParameteri(impl->gl_target, GL2_TEXTURE_MIN_FILTER, min_filter);

		if (gl_Self_IsTextureFilterAnisotropic())
		{
			if (ss->filter & TFOE_ANISOTROPIC)
			{
				gl2_TexParameteri(impl->gl_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, ss->max_anisotropy);
			}
			else
			{
				gl2_TexParameteri(impl->gl_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
			}
		}
	}

	if(gl_Self_IsTextureLOD())
	{
		if(curr_ss->min_lod != ss->min_lod)
		{
			curr_ss->min_lod = ss->min_lod;
			gl2_TexParameterf(impl->gl_target, GL3_TEXTURE_MIN_LOD, ss->min_lod);
		}

		if(curr_ss->max_lod != ss->max_lod)
		{
			curr_ss->max_lod = ss->max_lod;
			gl2_TexParameterf(impl->gl_target, GL3_TEXTURE_MAX_LOD, ss->max_lod);
		}
	}
	
	if(gl_Self_IsVersion3())
	{
		if(curr_ss->cmp_func != ss->cmp_func)
		{
			uint val = gl_compare_func_get(ss->cmp_func);
			curr_ss->cmp_func = ss->cmp_func;
			if (ss->cmp_func != CF_ALWAYSPASS)
			{
				gl2_TexParameteri(impl->gl_target, GL3_TEXTURE_COMPARE_MODE, GL3_COMPARE_REF_TO_TEXTURE);
			}
			else
			{
				gl2_TexParameteri(impl->gl_target, GL3_TEXTURE_COMPARE_MODE, GL2_NONE);
			}
			gl2_TexParameteri(impl->gl_target, GL3_TEXTURE_COMPARE_FUNC, val);
		}
	}
	
	if (gl_Self_GetInterfaceType() == RIT_OPENGL)
	{
		if(curr_ss->mip_map_lod_bias != ss->mip_map_lod_bias)
		{
			curr_ss->mip_map_lod_bias = ss->mip_map_lod_bias;
			gl2_TexParameterf(impl->gl_target, GL_TEXTURE_LOD_BIAS, ss->mip_map_lod_bias);
		}
	}
	return TRUE;
}