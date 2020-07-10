#ifndef INCLUDE_CONTROLLER_H
#define INCLUDE_CONTROLLER_H

#include <pi_lib.h>
#include <entity.h>

/**
 * 控制器：处理每帧计算过程的推动和更新
 */

typedef enum
{
	CT_SKANIM,		/* 骨骼动画控制器 */
	CT_VERTEX_ANIM,	/* 顶点动画控制器 */
	CT_SKBINDING,	/* 骨骼绑定控制器 */
	CT_TAIL,		/* 拖尾网格控制器 */
	CT_BILLBOARD,	/* 布告板控制器   */
	CT_CHAIN,		/* 锁链网格控制器 */
	CT_VEGETATION_ANIM,		/* 植被动画控制器 */
	CT_VANISH,               /*物体消散控制器*/
	CT_SWING,                /*纹理飘动控制*/
	CT_SHINY,                /*光圈特效控制器*/
	CT_COUNT,
	CT_USER = 5000		/* 自定义类型 */
} ControllerType;

/* 作用类型 */
typedef enum
{
	CAT_MESH,		/* 网格 */
	CAT_ENTITY,		/* 实体 */
	CAT_SPATIAL,	/* 空间信息 */
	CAT_BINDING_LIST, /* 绑定列表 */
	CAT_PARTICLE_CLUSTER,
} ControllerApplyType;

typedef struct PiController PiController;

typedef PiBool (*ControllerUpdateFunc)(struct PiController *c, float tpf);

typedef PiBool (*ControllerApplyFunc)(struct PiController *c, ControllerApplyType type, void *obj);

/**
 * 渲染器
 */
typedef struct PiController
{
	ControllerType type;	/* 控制器类型 */
	PiBool is_alive;
	void* impl;				/* 具体的控制器实现 */
	PiBool need_update;
	float delta_time;

	ControllerUpdateFunc update_func;	/* 更新 */
	ControllerApplyFunc apply_func;		/* 作用到具体的物体上 */
} PiController;


PI_BEGIN_DECLS
void PI_API pi_controller_free(PiController *c);

PiController* PI_API pi_controller_new(ControllerType type, ControllerApplyFunc apply, ControllerUpdateFunc update, void *impl);

PiBool PI_API pi_controller_update(PiController *c, float tpf);

PiBool PI_API pi_controller_apply(PiController *c, ControllerApplyType type, void *obj);
PI_END_DECLS
#endif /* INCLUDE_CONTROLLER_H */