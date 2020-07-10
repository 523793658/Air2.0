#ifndef INCLUDE_GL_TEXTURE_H
#define INCLUDE_GL_TEXTURE_H

#include <texture.h>
#include <rendersystem.h>

typedef struct  
{
	sint bind_unit;		/* ���󶨵�����Ԫ����ʼ��Ϊ-1 */
	
	uint gl_id;			/* �����OpenGL��� */
	uint gl_target;		/* Ŀ������ͣ�1D��2D��3D, Cube, 1D Array, 2D Array */
	
	uint gl_internal_fmt;	/* �Կ����ڲ���ʽ */
	uint gl_fmt;			/* gl�ĸ�ʽ */
	uint gl_type;			/* Ԫ�ص����ͣ�byte��short, ...) */
	
	SamplerState curr_ss;
}GLTexture;

PI_BEGIN_DECLS

/** ���½ӿڹ�openglר�� */

PiBool gl_texture_set_sampler(SamplerState *ss);

PI_END_DECLS

#endif /* INCLUDE_GL_TEXTURE_H */