#ifndef _INCLUDE_LIGHTMAP_H_
#define _INCLUDE_LIGHTMAP_H_
#include "pi_lib.h"
#include "texture.h"
#include "environment.h"
#include "camera.h"
#include "pi_aabb.h"
#include "entity.h"
#include "renderview.h"
#include "rendertarget.h"
#include "rendersystem.h"
#include "post_process_manager.h"
#include "magnifier.h"
typedef PiBool(PI_API *LPD3DXUVATLASCB)(float fPercentDone, void* lpUserContext);


typedef enum _BEAST_WORK_STATE
{
	BS_WAITING,
	BS_WORKING,
	BS_FINISHED
}BestState;

typedef struct
{
	int x;
	int y;
	int width;
	int height;
}Block;

typedef struct  
{
	PiEntity* entity;
	float scale_in_map;
}BakeObj;

typedef struct {
	//lightmap������Ϣ
	uint index;			//��ͼ����
	float scale_x;		//UV����
	float scale_y;
	float offset_x;		//UVƫ��
	float offset_y;

	Block block;

	int origin_size;	//ԭʼ��С
} PackInfo;

typedef struct
{
	PiVector* available_list;
}LightMapDispenser;		//���������


typedef struct{
	PiVector* lightmap_list;				//������ͼ�б�
	PiVector* shadow_map_list;				//��Ӱ��ͼ�б�
	PiVector* shadow_map_sampler_list;
	PiTexture* temp_map;
	uint width;
	uint height;
	PiTexture* shadow_map_texture;
	SamplerState shadow_map_sampler;
}LightMapManager;

typedef struct
{
	PiRenderView* defaultView;

	PiVector* lightmap_view_list;

	PiRenderView* temp_view;
	PiRenderView* shadow_map_view;
	PiRenderView* shadow_map_color_view;
}RenderViewManager;

typedef struct
{
	PiRenderTarget* shadowTarget;
	PiRenderTarget* shadingTarget;
}RenderTargetManager;

typedef struct  
{
	PiVector* shadow_camera_view_preject_matrix_list;
}TransformManager;


typedef struct
{
	PiVector* reference_info;
}BeastResult;

typedef struct
{
	PiVector* cast_shadow_entity_list;
	PiVector* beast_entity_list;
	PiEnvironment* render_environment;
	LPD3DXUVATLASCB callback;
	BeastResult* result;
	BestState state;
	wchar* result_path;
	uint texture_num;
	float precess;
}BeastWork;

typedef struct  
{
	PiMaterial* shading_material;
}BeastMaterialManager;

typedef struct
{
	LightMapManager* map_manager;
	LightMapDispenser *dispenser;
	PiCamera* shadow_camera;
	RenderViewManager* viewManager;
	RenderTargetManager* targetManager;
	PiVector* work_list;
	TransformManager* transformManager;
	BeastMaterialManager* beastMaterialManager;
	PostProcessManager* borderExtensionManager;
	Magnifier* magnifier;
	int block_x;
	int block_y;
}LightMapDevice;

PI_BEGIN_DECLS

LightMapDevice* PI_API pi_light_map_device_create();

void PI_API pi_light_map_device_init(LightMapDevice* device, uint size, GaussianBlurLevel level);

BeastWork* PI_API pi_light_map_work_create(PiVector* shadow_list, PiVector* entity_list, PiEnvironment* environment);

void PI_API pi_light_map_add_work(LightMapDevice* device, BeastWork* work);

void PI_API pi_light_map_start(LightMapDevice* device);

void PI_API pi_light_map_work_free(BeastWork* work);

void PI_API pi_light_map_work_set_output_path(BeastWork* work, wchar* path);

void PI_API pi_light_map_device_free(LightMapDevice* device);

PiBool PI_API pi_mesh_atlas_create(wchar* mesh_path, PiBool isAuto);

PiBool PI_API pi_light_map_check_work_end(BeastWork* work);

uint PI_API pi_light_map_get_result_info(BeastWork* work, uint index, float offset[4]);

BakeObj* PI_API pi_light_map_create_bake_obj(PiEntity* entity, float scale);

void PI_API pi_light_map_bake_obj_free(BakeObj* obj);

PI_END_DECLS


#endif