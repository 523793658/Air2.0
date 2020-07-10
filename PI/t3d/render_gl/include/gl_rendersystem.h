#ifndef INCLUDE_GL_RENDERSYSTEM_H
#define INCLUDE_GL_RENDERSYSTEM_H

#include <rendercap.h>
#include <rendersystem.h>
#include <gl_renderstate.h>

typedef struct  
{
	uint fbo;
	void* context;

	uint blit_src_fbo;
	uint blit_dst_fbo;

	GLRenderState state;
}GLRenderSystem;

#endif /* INCLUDE_GL_RENDERSYSTEM_H */