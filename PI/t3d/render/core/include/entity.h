#ifndef INCLUDE_ENTITY_H
#define INCLUDE_ENTITY_H

#include <pi_lib.h>
#include <pi_mesh.h>
#include <pi_spatial.h>

#include <material.h>
#include <pi_rendermesh.h>
#include <renderutil.h>

/**
 * 排序数据
 */
typedef enum
{
	EBT_PRIORITY,		/* 排序用到的优先级，越大越优先 */
	EBT_LIGHT_LIST,		/* 影响此Entity的光源列表，注意：此列表的创建/释放需要外部显式执行 */
	EBT_DISCARD_SHADOW,		/* 标明此entity受LIGHTMAP影响 */
	EBT_ACTOR,
	EBT_INSTANCE,
	EBT_NUM
} EntityBindType;
typedef struct _SoftWareSkinData
{
	PiRenderMesh* renderMesh;
	PiMesh* mesh;
	PiRenderData* renderData;
}SoftWareSkinData;


/**
 * 实体：mesh + material + 世界矩阵
 */
typedef struct
{
	PiRenderMesh *mesh;			/* 网格 */
	SoftWareSkinData *skinedData;	/*软件蒙皮数据*/
	PiMaterial *material;		/* 材质 */
	PiSpatial *spatial;			/* 空间信息 */
	PiSpatial *reference_spatial;
	int hws_required;
	void *bind_data[EBT_NUM];	/* 绑定数据 */
} PiEntity;

PI_BEGIN_DECLS

/* 创建entity */
PiEntity *PI_API pi_entity_new(void);

/* 销毁entity */
void PI_API pi_entity_free(PiEntity *entity);

/* 设置网格，指针 */
void PI_API pi_entity_set_mesh(PiEntity *entity, PiRenderMesh *mesh);

/* 设置材质，指针 */
void PI_API pi_entity_set_material(PiEntity *entity, PiMaterial *material);

/* 取世界矩阵 */
PiMatrix4 *PI_API pi_entity_get_world_matrix(PiEntity *entity);

/* 取spatial */
PiSpatial *PI_API pi_entity_get_spatial(PiEntity *entity);

/* 设置参考spatial*/
void PI_API pi_entity_set_reference_spatial(PiEntity* entity, PiSpatial* spatial);

//获取entity的世界aabb
PiAABBBox* PI_API pi_entity_get_world_aabb(PiEntity* entity);

/* 渲染entity, 包括设置material，设置uniform */
void PI_API pi_entity_draw(PiEntity *entity);

void PI_API pi_entity_draw_instance(PiEntity *entity, uint n);

/**
 * 设置实例化的entity数组
 */
void PI_API pi_entity_draw_list_back(PiVector *entity_list);

void PI_API pi_entity_draw_list(PiVector *entity_list);

void PI_API pi_entity_set_bind(PiEntity *entity, EntityBindType type, void *data);

void *PI_API pi_entity_get_bind(PiEntity *entity, EntityBindType type);

void PI_API pi_entity_add_hws_requied(PiEntity* entity, int count);

PI_END_DECLS

#endif /* INCLUDE_ENTITY_H */