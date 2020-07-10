#include "d3d9_texture.h"
#include "d3d9_convert.h"
#include "d3d9_renderstate.h"
#include "d3d9_rendersystem.h"

#include "renderinfo.h"
#include "renderwrap.h"

#include <d3dx9.h>

// D3D9的纹理总结：
// 1.深度模板纹理必须使用 Usage = D3DUSAGE_DEPTHSTENCIL 并且 Pool = D3DPOOL_DEFAULT,需要处理设备丢失,不支持锁定
// 2.RenderTarget 纹理必须使用 Usage = D3DUSAGE_RENDERTARGET 并且 Pool = D3DPOOL_DEFAULT,需要处理设备丢失,不支持锁定
// 3.普通纹理使用 Usage = 0 并且 Pool = D3DPOOL_MANAGED,不需要处理设备丢失,支持锁定
// 4.通过锁定纹理 (LockRect/LockBox) 可以更新或者读取纹理数据,需要应用 RowPitch 和 SlicePitch
// 5.纹理拷贝问题, D3D9的api src 必须使用 Usage = D3DPOOL_SYSTEMMEM 并且 dest 必须使用 Pool = D3DPOOL_DEFAULT,固暂不支持纹理拷贝
// 6.纹理拷贝实现, src 和 dest 都用普通纹理,创建一个临时缓冲,从 src 读取数据并写入 dest 中

extern "C" {

	extern PiRenderSystem *g_rsystem;

	static uint _get_compress_block_size(RenderFormat format)
	{
		uint size = 0;
		switch (format)
		{
		case RF_BC1:
		case RF_SIGNED_BC1:
		case RF_BC1_SRGB:
		case RF_BC4:
		case RF_SIGNED_BC4:
		case RF_BC4_SRGB:
			size = 8;
			break;
		default:
			size = 16;
			break;
		}
		return size;
	}


	PiBool d3d9_texture_init(PiTexture *texture)
	{
		D3D9Texture *impl = NULL;
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
		D3D9Context *context = d3d9_system->context;

		if (texture->array_size > 1)
		{
			pi_log_print(LOG_WARNING, "texture array isn't support");
			return FALSE;
		}

		texture->impl = impl = pi_new0(D3D9Texture, 1);

		impl->d3d9_format = d3d9_tex_format_get(texture->format);
		d3d9_tex_usage_get(texture->usage, &impl->d3d9_usage, &impl->d3d9_pool);

		HRESULT hr = S_OK;
		switch (texture->type)
		{
		case TT_2D:
			hr = IDirect3DDevice9_CreateTexture(context->device, texture->width, texture->height, texture->num_mipmap, impl->d3d9_usage, impl->d3d9_format, impl->d3d9_pool, &impl->handle.texture_2d, NULL);
			break;
		case TT_3D:
			hr = IDirect3DDevice9_CreateVolumeTexture(context->device, texture->width, texture->height, texture->depth, texture->num_mipmap, impl->d3d9_usage, impl->d3d9_format, impl->d3d9_pool, &impl->handle.texture_3d, NULL);
			break;
		case TT_CUBE:
			hr = IDirect3DDevice9_CreateCubeTexture(context->device, texture->width, texture->num_mipmap, impl->d3d9_usage, impl->d3d9_format, impl->d3d9_pool, &impl->handle.texture_cube, NULL);
			break;
		default:
			break;
		}

		PI_ASSERT(SUCCEEDED(hr), "CreateTexture failed, hr = %d", hr);
		pi_renderstate_set_default_sampler(&impl->curr_ss);
		impl->curr_ss.tex = texture;

		return TRUE;
	}

	PiBool d3d9_texture_clear(PiTexture *texture)
	{
		D3D9Texture *impl = (D3D9Texture *)texture->impl;

		if (impl != NULL)
		{
			switch (texture->type)
			{
			case TT_2D:
				IDirect3DTexture9_Release(impl->handle.texture_2d);
				break;
			case TT_3D:
				IDirect3DVolumeTexture9_Release(impl->handle.texture_3d);
				break;
			case TT_CUBE:
				IDirect3DCubeTexture9_Release(impl->handle.texture_cube);
				break;
			default:
				break;
			}
			pi_free(impl);
			texture->impl = NULL;
		}
		return TRUE;
	}

	static PiBool _is_default_texture(PiTexture *texture)
	{
		D3D9Texture *impl = (D3D9Texture *)texture->impl;
		return impl->d3d9_pool == D3DPOOL_DEFAULT;
	}

	PiBool PI_API render_texture_init(PiTexture *texture)
	{
		uint d3d9_usage;
		D3DPOOL d3d9_pool;
		d3d9_tex_usage_get(texture->usage, &d3d9_usage, &d3d9_pool);
		if (d3d9_pool != D3DPOOL_DEFAULT || render_system_check(g_rsystem) != CHECK_LOST){
			PiBool r = d3d9_texture_init(texture);
			if (r)
			{
				pi_renderinfo_add_texture_num(1);

				if (_is_default_texture(texture))
				{
					d3d9_state_add_default_texture(texture);
				}
			}
			return r;
		}
		else
		{
			uint usage;
			D3DPOOL pool;
			d3d9_tex_usage_get(texture->usage, &usage, &pool);
			if (pool == D3DPOOL_DEFAULT)
			{
				d3d9_state_add_default_texture(texture);
				pi_renderinfo_add_texture_num(1);
			}
			return TRUE;
		}
	}

	PiBool PI_API render_texture_clear(PiTexture *texture)
	{
		PiBool is_remove_from_state = _is_default_texture(texture);

		PiBool r = d3d9_texture_clear(texture);
		
		if (r)
		{
			pi_renderinfo_add_texture_num(-1);

			if (is_remove_from_state)
			{
				d3d9_state_remove_default_texture(texture);
			}			
		}
		return r;
	}

	// 注：因为GenerateMipSubLevls不起作用，所以用D3DX软件生成Mipmap
	PiBool PI_API render_texture_build_mipmap(PiTexture *texture)
	{
		D3D9Texture *impl = (D3D9Texture *)texture->impl;
		switch (texture->type)
		{
		case TT_2D:
			// IDirect3DTexture9_GenerateMipSubLevels(impl->handle.texture_2d);
			D3DXFilterTexture(impl->handle.texture_2d, NULL, D3DX_DEFAULT, D3DX_DEFAULT);
			break;
		case TT_3D:
			// IDirect3DVolumeTexture9_GenerateMipSubLevels(impl->handle.texture_3d);
			D3DXFilterVolumeTexture(impl->handle.texture_3d, NULL, D3DX_DEFAULT, D3DX_DEFAULT);
			break;
		case TT_CUBE:
			// IDirect3DCubeTexture9_GenerateMipSubLevels(impl->handle.texture_cube);
			D3DXFilterCubeTexture(impl->handle.texture_cube, NULL, D3DX_DEFAULT, D3DX_DEFAULT);
			break;
		default:
			break;
		}
		return TRUE;
	}

	void *PI_API render_texture_get_curr_sampler(PiTexture *tex)
	{
		D3D9Texture *impl = (D3D9Texture *)tex->impl;
		return &impl->curr_ss;
	}

	PiBool PI_API render_texture_2d_update(PiTexture *texture, uint array_index, uint level, uint x, uint y, uint w, uint h, uint data_size, byte *data)
	{
		RECT rect;
		D3DLOCKED_RECT locked_rect;
		D3D9Texture *impl = (D3D9Texture *)texture->impl;
		
		rect.left = x;
		rect.right = x + w;
		rect.top = y;
		rect.bottom = y + h;

		// 注： D3DPOOL_DEFAULT 的纹理 不能 lock
		PI_ASSERT(texture->usage == TU_NORMAL, "texture usage can't lock !");

		HRESULT hr = IDirect3DTexture9_LockRect(impl->handle.texture_2d, level, &locked_rect, &rect, 0);
		PI_ASSERT(SUCCEEDED(hr), "IDirect3DTexture9_LockRect can't lock, hr = %d", hr);
		
		if (pi_renderformat_is_compressed_format(texture->format))
		{// 如果是DDS压缩格式，那么外部的数据和里面的数据应该一模一样
			
			uint block_size = _get_compress_block_size(texture->format);
			uint src_pitch = (w + 3) / 4 * block_size;
			PI_ASSERT(locked_rect.Pitch == (int)src_pitch, "Pitch isn't same as computed !");

			pi_memcpy_inline(locked_rect.pBits, data, data_size);
		}
		else
		{
			byte *dst_data = (byte *)locked_rect.pBits;
			uint j, size_per_pixel = pi_renderformat_get_numbytes(texture->format);
			if (size_per_pixel * w != ((uint)locked_rect.Pitch))
			{
				for (j = 0; j < h; ++j)
				{
					pi_memcpy_inline(dst_data, data, w * size_per_pixel);
					dst_data += locked_rect.Pitch;
					data += w * size_per_pixel;
				}
			}
			else
			{
				pi_memcpy_inline(dst_data, data, data_size);
			}
			
		}
		
		IDirect3DTexture9_UnlockRect(impl->handle.texture_2d, level);
		
		return TRUE;
	}

	PiBool PI_API render_texture_3d_update(PiTexture *texture, uint level, uint x, uint y, uint z, uint w, uint h, uint d, uint data_size, byte *data)
	{
		D3DBOX box;
		D3DLOCKED_BOX locked_box;
		D3D9Texture *impl = (D3D9Texture *)texture->impl;
			
		box.Left = x;
		box.Top = y;
		box.Right = x + w;
		box.Bottom = y + h;
		box.Front = z;
		box.Back = z + d;

		// 注： D3DPOOL_DEFAULT 的纹理 不能 lock
		PI_ASSERT(texture->usage == TU_NORMAL, "texture usage can't lock !");

		HRESULT hr = IDirect3DVolumeTexture9_LockBox(impl->handle.texture_3d, level, &locked_box, &box, 0);
		PI_ASSERT(SUCCEEDED(hr), "IDirect3DVolumeTexture9_LockBox can't lock, hr = %d", hr);

		if (pi_renderformat_is_compressed_format(texture->format))
		{// 如果是DDS压缩格式，那么外部的数据和里面的数据应该一模一样

			uint block_size = _get_compress_block_size(texture->format);
			uint row_pitch = (w + 3) / 4 * block_size;
			uint slice_pitch = ((w + 3) / 4) * ((h + 3) / 4) * block_size;
			PI_ASSERT(locked_box.RowPitch == (int)row_pitch && locked_box.SlicePitch == (int)slice_pitch, "Pitch isn't same as computed !");

			pi_memcpy_inline(locked_box.pBits, data, data_size);
		}
		else
		{
			byte *dst_data = (byte *)locked_box.pBits;
			uint i, j, size_per_pixel = pi_renderformat_get_numbytes(texture->format);
			for (j = 0; j < d; ++j)
			{
				for (i = 0; i < h; ++i)
				{
					pi_memcpy_inline(dst_data, data, w * size_per_pixel);
					dst_data += locked_box.RowPitch;
					data += w * size_per_pixel;
				}
			}
		}
		
		IDirect3DVolumeTexture9_UnlockBox(impl->handle.texture_3d, level);

		return TRUE;
	}

	PiBool PI_API render_texture_cube_update(PiTexture *texture, uint level, TextureCubeFace face, uint x, uint y, uint w, uint h, uint data_size, byte *data)
	{
		RECT rect;
		D3DLOCKED_RECT locked_rect;
		D3DCUBEMAP_FACES face_type;
		D3D9Texture *impl = (D3D9Texture *)texture->impl;
		
		rect.left = x;
		rect.right = x + w;
		rect.top = y;
		rect.bottom = y + h;

		face_type = d3d9_cube_map_face_get(face);

		// 注： D3DPOOL_DEFAULT 的纹理 不能 lock
		PI_ASSERT(texture->usage == TU_NORMAL, "texture usage can't lock !");

		HRESULT hr = IDirect3DCubeTexture9_LockRect(impl->handle.texture_cube, face_type, level, &locked_rect, &rect, 0);
		PI_ASSERT(SUCCEEDED(hr), "IDirect3DCubeTexture9_LockRect can't lock, hr = %d", hr);

		if (pi_renderformat_is_compressed_format(texture->format))
		{// 如果是DDS压缩格式，那么外部的数据和里面的数据应该一模一样

			uint block_size = _get_compress_block_size(texture->format);
			uint src_pitch = (w + 3) / 4 * block_size;
			PI_ASSERT(locked_rect.Pitch == (int)src_pitch, "Pitch isn't same as computed !");

			pi_memcpy_inline(locked_rect.pBits, data, data_size);
		}
		else
		{
			byte *dst_data = (byte *)locked_rect.pBits;
			uint j, size_per_pixel = pi_renderformat_get_numbytes(texture->format);
			for (j = 0; j < h; ++j)
			{
				pi_memcpy_inline(dst_data, data, w * size_per_pixel);
				dst_data += locked_rect.Pitch;
				data += w * size_per_pixel;
			}
		}

		IDirect3DCubeTexture9_UnlockRect(impl->handle.texture_cube, face_type, level);

		return TRUE;
	}

	PiImage *PI_API render_texture_2d_get(PiTexture *texture, uint array_index, uint level, uint x, uint y, uint w, uint h)
	{
		PiImage *img = NULL;
		D3D9Texture *impl = (D3D9Texture *)texture->impl;

		uint size_per_pixel = pi_renderformat_get_numbytes(texture->format);
		byte *data = pi_new0(byte, w * h * size_per_pixel);

		if (pi_renderformat_is_compressed_format(texture->format))
		{
			PI_ASSERT(FALSE, "get texture can't work at compress texture");
			pi_free(data);
			return NULL;
		}

		IDirect3DTexture9 *dst_texture = impl->handle.texture_2d;

		if (texture->usage != TU_NORMAL)
		{// 如果是源是渲染目标的纹理，那么就要用GetRenderTargetData来获取数据

			IDirect3DSurface9 *src_surface = NULL;
			HRESULT hr = IDirect3DTexture9_GetSurfaceLevel(impl->handle.texture_2d, level, &src_surface);
			PI_ASSERT(SUCCEEDED(hr), "GetSurfaceLevel failed, hr = %d", hr);

			IDirect3DSurface9 *dst_surface = NULL;

			D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;
			D3D9Context *context = d3d9_system->context;

			PI_ASSERT(level < texture->num_mipmap, "Texture level isn't valid");

			uint w = texture->level_size[level].width;
			uint h = texture->level_size[level].height;

			// 目标必须是 SYSTEM分配池；
			// mipmap数量必须为1
			hr = IDirect3DDevice9_CreateTexture(context->device, w, h, 1, 0, impl->d3d9_format, D3DPOOL_SYSTEMMEM, &dst_texture, NULL);
			PI_ASSERT(SUCCEEDED(hr), "CreateTexture failed, hr = %d", hr);

			hr = IDirect3DTexture9_GetSurfaceLevel(dst_texture, 0, &dst_surface);
			PI_ASSERT(SUCCEEDED(hr), "GetSurfaceLevel failed, hr = %d", hr);

			hr = IDirect3DDevice9_GetRenderTargetData(context->device, src_surface, dst_surface);
			PI_ASSERT(SUCCEEDED(hr), "GetRenderTargetData failed, hr = %d", hr);

			IDirect3DSurface9_Release(src_surface);
			IDirect3DSurface9_Release(dst_surface);
		}
		
		RECT rect;
		D3DLOCKED_RECT locked_rect;

		rect.left = x;
		rect.right = x + w;
		rect.top = y;
		rect.bottom = y + h;

		HRESULT hr = IDirect3DTexture9_LockRect(dst_texture, level, &locked_rect, &rect, D3DLOCK_READONLY);
		PI_ASSERT(SUCCEEDED(hr), "IDirect3DTexture9_LockRect can't lock, hr = %d", hr);

		byte *dst_data = data;
		byte *src_data = (byte *)locked_rect.pBits;
		for (uint i = 0; i < h; ++i)
		{
			pi_memcpy_inline(dst_data, src_data, w * size_per_pixel);
			dst_data += w * size_per_pixel;
			src_data += locked_rect.Pitch;
		}

		IDirect3DTexture9_UnlockRect(dst_texture, level);
		
		if (texture->usage != TU_NORMAL)
		{
			IDirect3DTexture9_Release(dst_texture);
		}

		img = pi_render_image_new(w, h, texture->format, data);
		pi_free(data);
		return img;
	}

	PiImage *PI_API render_texture_cube_get(PiTexture *texture, uint level, TextureCubeFace face, uint x, uint y, uint w, uint h)
	{
		PI_ASSERT(FALSE, "this function isn't implemention");
		return NULL;
	}

	PiBool PI_API render_texture_2d_copy(PiTexture *dst, PiTexture *src,
	                                     uint dst_array_index, uint dst_level, uint dst_x, uint dst_y,
	                                     uint src_array_index, uint src_level, uint src_x, uint src_y, uint w, uint h)
	{
		PI_ASSERT(FALSE, "this function isn't implemention");
		return FALSE;
	}

	PiBool PI_API render_texture_3d_copy(PiTexture *dst, PiTexture *src,
	                                     uint dst_level, uint dst_x, uint dst_y, uint dst_z,
	                                     uint src_level, uint src_x, uint src_y, uint src_z, uint w, uint h, uint d)
	{
		PI_ASSERT(FALSE, "this function isn't implemention");
		return FALSE;
	}

	PiBool PI_API render_texture_cube_copy(PiTexture *dst, PiTexture *src,
	                                       TextureCubeFace dst_face, uint dst_level, uint dst_x, uint dst_y,
	                                       TextureCubeFace src_face, uint src_level, uint src_x, uint src_y, uint w, uint h)
	{
		PI_ASSERT(FALSE, "this function isn't implemention");
		return FALSE;
	}
}