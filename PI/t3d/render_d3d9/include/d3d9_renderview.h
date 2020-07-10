#ifndef INCLUDE_D3D9_RENDERVIEW_H
#define INCLUDE_D3D9_RENDERVIEW_H

#include "pi_lib.h"
#include "renderview.h"
#include <d3d9.h>

PI_BEGIN_DECLS

PiRenderView *d3d9_new_main_view(IDirect3DSurface9 *back_buffer, uint width, uint height, RenderFormat format, RenderViewType type);

void d3d9_free_main_view(PiRenderView *view);

PiBool d3d9_view_init(PiRenderView *view);

PiBool d3d9_view_clear(PiRenderView *view);

PI_END_DECLS

#endif /* INCLUDE_D3D9_RENDERVIEW_H */
