#ifndef SPATIAL_SYNC_H
#define SPATIAL_SYNC_H

#include <pi_spatial.h>

//作用类型
typedef enum
{
    SYNC_BOTH,
	SYNC_TRANSLATION,
	SYNC_SCALING,
	SYNC_ROTATION,
} applyType;

typedef struct useData
{
    int useType;
    PiSpatial *spatial;
    PiMatrix4 offsetMatrix;
} useData;

typedef struct sync
{
    PiSpatial *dstSpatial;
    PiVector *useDatas;
} spatialSync;

//创建同步函数
spatialSync *PI_API pi_create_sync(PiSpatial *dstSpatital);

//释放同步函数
void PI_API pi_free_sync(spatialSync *spatialSync);

//同步函数添加同步数据
void PI_API pi_add_sync_data(spatialSync *spatialSync, PiSpatial *src, int type, PiMatrix4 *local_matrix);

//同步函数删除同步数据
void PI_API pi_remove_sync_data(spatialSync *spatialSync, PiSpatial *src);

#endif /* SPATIAL_SYNC_H */