#ifndef INCLUDE_GL_TEXTURE_H
#define INCLUDE_GL_TEXTURE_H

#include <texture.h>
#include <rendersystem.h>

typedef struct  
{
	sint bind_unit;		/* 被绑定的纹理单元，初始化为-1 */
	
	uint gl_id;			/* 纹理的OpenGL句柄 */
	uint gl_target;		/* 目标的类型，1D，2D，3D, Cube, 1D Array, 2D Array */
	
	uint gl_internal_fmt;	/* 显卡的内部格式 */
	uint gl_fmt;			/* gl的格式 */
	uint gl_type;			/* 元素的类型（byte，short, ...) */
	
	SamplerState curr_ss;
}GLTexture;

PI_BEGIN_DECLS

/** 以下接口供opengl专用 */

PiBool gl_texture_set_sampler(SamplerState *ss);

PI_END_DECLS

#endif /* INCLUDE_GL_TEXTURE_H */