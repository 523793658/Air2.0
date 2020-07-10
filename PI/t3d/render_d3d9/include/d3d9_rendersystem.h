#ifndef INCLUDE_D3D9_RENDERSYSTEM_H
#define INCLUDE_D3D9_RENDERSYSTEM_H

/** 
 * ��Ⱦ��ں͵���
 */

#include "d3d9_context.h"
#include "d3d9_renderstate.h"


typedef struct
{
	D3D9Context *context;    /* ��Ⱦ���� */

	D3D9RenderState state;   /* ��Ⱦ״̬ */
} D3D9RenderSystem;

#endif /* INCLUDE_D3D9_RENDERSYSTEM_H */
