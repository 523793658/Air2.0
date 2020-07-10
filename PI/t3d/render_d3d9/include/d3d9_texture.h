#ifndef INCLUDE_D3D9_TEXTURE_H
#define INCLUDE_D3D9_TEXTURE_H

#include "renderstate.h"
#include <d3d9.h>


typedef struct
{
	D3DFORMAT d3d9_format;		/* 显卡的内部格式 */
	uint d3d9_usage;
	D3DPOOL d3d9_pool;

	SamplerState curr_ss;

	union
	{		
		IDirect3DTexture9 *texture_2d;
		IDirect3DVolumeTexture9 *texture_3d;
		IDirect3DCubeTexture9 *texture_cube;
	} handle;
} D3D9Texture;

PI_BEGIN_DECLS

PiBool d3d9_texture_init(PiTexture *texture);

PiBool d3d9_texture_clear(PiTexture *texture);

PI_END_DECLS

#endif /* INCLUDE_D3D9_TEXTURE_H */
