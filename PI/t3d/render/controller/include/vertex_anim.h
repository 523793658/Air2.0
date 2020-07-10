#ifndef INCLUDE_VERTEX_ANIM_H
#define INCLUDE_VERTEX_ANIM_H

#include <pi_lib.h>
#include <controller.h>

/*
 *顶点动画控制器
 */
typedef enum
{
	AmbientColor		= 0,
	DiffuseColor		= 1,
	SpecularColor		= 2,
	SpecularLevel		= 3,
	Glossiness			= 4,
	SelfIllumination	= 5,
	Opacity				= 6,
	FilterColor			= 7,
	Bump				= 8,
	Refelction			= 9,
	Refraction			= 10,
	Displacement		= 11,
	TexMapTypeNum
} TexMapType;

typedef struct
{
	PiVector3 translate;
	PiVector3 scale;
	PiQuaternion rotate;
} Transform;

typedef struct
{
	int32 start_time;
	uint32 sub_mesh_count;
	PiMesh **meshes;
	Transform **uv_transforms;
} VertexAnimKeyFrame;

typedef struct
{
	int32 frame_count;
	VertexAnimKeyFrame *frames;
} VertexAnimNode;

typedef struct
{
	int32 duration;

	uint32 node_count;
	VertexAnimNode *nodes;
} PiVertexAnim;

typedef enum
{
	VAS_LOOP,	 /* 循环播放 */
	VAS_DEFAULT, /* 播完后，恢复到默认类型 */
	VAS_KEEP,	 /* 播完后，保持最后帧类型 */
	VAS_FIRST,   /* 播完后，保持第一帧类型 */
} VertexAnimationState;

PI_BEGIN_DECLS

PiController *PI_API pi_vertex_anim_new(uint32 node_count);

void PI_API pi_vertex_anim_free(PiController *c);

PiBool PI_API pi_vertex_anim_add(PiController *c, wchar *name, PiVertexAnim *anim);

PiBool PI_API pi_vertex_anim_play(PiController *c, wchar *name, float speed, VertexAnimationState state, float start_time);

PiBool PI_API pi_vertex_anim_stop(PiController *c);

PiVertexAnim *PI_API pi_veanim_new(byte *data, uint32 size);

void PI_API pi_veanim_free(PiVertexAnim *veanim);

void PI_API pi_veanim_load(PiVertexAnim *veanim, byte *data, uint32 size);

PI_END_DECLS

#endif //INCLUDE_VERTEX_ANIM_H