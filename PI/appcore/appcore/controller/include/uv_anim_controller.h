#ifndef INCLUDE_UV_ANIM_H
#define INCLUDE_UV_ANIM_H
#include "app_controller.h"
#include "pi_lib.h"
#include "vertex_anim.h"


typedef struct UVAnimKeyFrame
{
	int32 start_time;
	uint32 sub_mesh_count;
	Transform **uv_transforms;
}UVAnimKeyFrame;


typedef enum
{
	UV_LOOP,	 /* 循环播放 */
	UV_DEFAULT, /* 播完后，恢复到默认类型 */
	UV_KEEP,	 /* 播完后，保持最后帧类型 */
	UV_FIRST,   /* 播完后，保持第一帧类型 */
} UVAnimationState;

typedef struct
{
	int32 frame_count;
	UVAnimKeyFrame *frames;
} UVAnimNode;

typedef struct
{
	int32 duration;
	uint32 node_count;
	UVAnimNode *nodes;
}PiUVAnim;

PI_BEGIN_DECLS

PiController* PI_API app_uv_anim_controller_new(uint32 node_count);
void PI_API app_uv_anim_controller_free(PiController* controller);
PiBool PI_API app_uv_anim_stop(PiController *c);
PiBool PI_API app_uv_anim_play(PiController *c, wchar *name, float speed, UVAnimationState state, float start_time);
PiBool PI_API app_uv_anim_add(PiController *c, wchar *name, PiUVAnim *anim);
PiController* PI_API app_uv_anim_controller_new(uint32 node_count);
void PI_API app_uv_anim_controller_free(PiController* controller);
void PI_API app_uv_anim_free(PiUVAnim *anim);
void PI_API app_uvanim_load(PiUVAnim *uvanim, byte* data, uint32 size);
PiUVAnim* PI_API app_uv_anim_new(byte* data, uint32 size);

PI_END_DECLS

#endif