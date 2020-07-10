#ifndef SPATIAL_SYNC_H
#define SPATIAL_SYNC_H

#include <pi_spatial.h>

//��������
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

//����ͬ������
spatialSync *PI_API pi_create_sync(PiSpatial *dstSpatital);

//�ͷ�ͬ������
void PI_API pi_free_sync(spatialSync *spatialSync);

//ͬ���������ͬ������
void PI_API pi_add_sync_data(spatialSync *spatialSync, PiSpatial *src, int type, PiMatrix4 *local_matrix);

//ͬ������ɾ��ͬ������
void PI_API pi_remove_sync_data(spatialSync *spatialSync, PiSpatial *src);

#endif /* SPATIAL_SYNC_H */