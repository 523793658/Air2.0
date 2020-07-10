#include "d3d9_rendersystem.h"
#include "d3d9_renderview.h"
#include "d3d9_texture.h"
#include "renderinfo.h"
#include "renderwrap.h"

extern "C" {

	extern PiRenderSystem *g_rsystem;

	PiBool PI_API render_target_init(PiRenderTarget *target)
	{
		return TRUE;
	}

	PiBool PI_API render_target_clear(PiRenderTarget *target)
	{
		return TRUE;
	}

	PiBool PI_API render_target_copy(PiRenderTarget *dst, PiRenderTarget *src, TargetBufferMask mask, TargetFilterType filter)
	{
		// todo:使用 StretchRect 把多个surface依次拷贝，不过存在一些限制
		return TRUE;
	}

	PiBool PI_API render_target_set_viewport(PiRenderTarget *target, uint32 left, uint32 bottom, uint32 width, uint32 height)
	{
		D3D9RenderSystem *d3d9_system = (D3D9RenderSystem *)g_rsystem->impl;

		target->left = left;
		target->bottom = bottom;
		target->width = width;
		target->height = height;

		/* 如果该目标已经绑在渲染环境中，还需要设置到OpenGL */
		if (target == d3d9_system->state.target)
		{
			d3d9_state_set_viewport(left, bottom, width, height);
		}
		return TRUE;
	}

	PiBool PI_API render_target_bind(PiRenderTarget *target)
	{
		return TRUE;
	}

	/* 无特殊实现 */
	PiBool PI_API render_target_unbind(PiRenderTarget *target)
	{
		return TRUE;
	}

}
