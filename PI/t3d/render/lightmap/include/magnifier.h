#ifndef _Magnifier_H_
#define _Magnifier_H_



typedef struct
{
	PiRenderView* temp_view[3];
	PiRenderTarget* temp_target[3];
	PiTexture* temp_texture[3];
	PiEntity* entity;
	PiMaterial* material;
	PiMesh* mesh;
	PiRenderMesh* rmesh;
	SamplerState ss_src;
	PiCamera* camera;
}Magnifier;

Magnifier* PI_API pi_lightmap_magnifier_create();

void PI_API pi_lightmap_magnifier_work(Magnifier* magnifier, PiTexture* texture, PiRenderTarget* target);
#endif