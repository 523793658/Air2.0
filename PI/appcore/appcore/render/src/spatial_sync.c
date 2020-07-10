#include "spatial_sync.h"

//全同步（位置、缩放、旋转）带偏移处理
static void _sync_both(struct PiSpatial* spatial, useData* user_data)
{
    PiQuaternion rotation;
	PiVector3 translation, scaling;
    PiMatrix4 matrix, *cache_matrix;

    cache_matrix = pi_spatial_get_world_transform(spatial);
    pi_mat4_mul(&matrix, cache_matrix, &user_data->offsetMatrix);
	pi_mat4_decompose(&translation, &scaling, &rotation, &matrix);
    pi_spatial_set_local_translation(user_data->spatial, translation.x, translation.y, translation.z);
    pi_spatial_set_local_scaling(user_data->spatial, scaling.x, scaling.y, scaling.z);
    pi_spatial_set_local_rotation(user_data->spatial, rotation.w, rotation.x, rotation.y, rotation.z);
    pi_spatial_update(user_data->spatial);
}

//位置同步，带位置偏移
static void _sync_translation(struct PiSpatial* spatial, useData* user_data)
{
	PiQuaternion rotation;
	PiVector3 translation, scaling;
    PiMatrix4 matrix, *cache_matrix;

    cache_matrix = pi_spatial_get_world_transform(spatial);
    pi_mat4_mul(&matrix, cache_matrix, &user_data->offsetMatrix);
	pi_mat4_decompose(&translation, &scaling, &rotation, &matrix);
    pi_spatial_set_local_translation(user_data->spatial, translation.x, translation.y, translation.z);
    pi_spatial_update(user_data->spatial);
}

//缩放同步，带缩放偏移
static void _sync_scaling(struct PiSpatial* spatial, useData* user_data)
{
	PiQuaternion rotation;
	PiVector3 translation, scaling;
    PiMatrix4 matrix, *cache_matrix;

    cache_matrix = pi_spatial_get_world_transform(spatial);
    pi_mat4_mul(&matrix, cache_matrix, &user_data->offsetMatrix);
	pi_mat4_decompose(&translation, &scaling, &rotation, &matrix);
    pi_spatial_set_local_scaling(user_data->spatial, scaling.x, scaling.y, scaling.z);
    pi_spatial_update(user_data->spatial);
}

//旋转同步，带旋转偏移
static void _sync_rotation(struct PiSpatial* spatial, useData* user_data)
{
   PiQuaternion rotation;
	PiVector3 translation, scaling;
    PiMatrix4 matrix, *cache_matrix;

    cache_matrix = pi_spatial_get_world_transform(spatial);
    pi_mat4_mul(&matrix, cache_matrix, &user_data->offsetMatrix);
	pi_mat4_decompose(&translation, &scaling, &rotation, &matrix);
    pi_spatial_set_local_rotation(user_data->spatial, rotation.w, rotation.x, rotation.y, rotation.z);
    pi_spatial_update(user_data->spatial);
}

static void _sync_fun (struct PiSpatial* spatial, useData* user_data)
{
    int type;
    type = user_data->useType;
    switch (type)
	{
        case SYNC_BOTH:
            _sync_both(spatial, user_data);
            break;

		case SYNC_TRANSLATION:
            _sync_translation(spatial, user_data);
			break;

		case SYNC_SCALING:
            _sync_scaling(spatial, user_data);
			break;
        
        case SYNC_ROTATION:
            _sync_rotation(spatial, user_data);
            break;
        
        default:
			pi_log_print(LOG_WARNING, "the apply's type isn't valid");
			return;
			break;
	}
}

static void _sync_update_fun(struct PiSpatial* spatial, void* user_data)
{
    uint i, size;
    spatialSync *data = (spatialSync *)user_data;
    size = pi_vector_size(data->useDatas);
    for (i = 0; i < size; i++)
    {
		_sync_fun(spatial, (useData *)pi_vector_get(data->useDatas, i));
    }
}

spatialSync *PI_API pi_create_sync(PiSpatial *dstSpatial)
{
    spatialSync *syncData = pi_new0(spatialSync, 1);
    syncData->dstSpatial = dstSpatial;
    syncData->useDatas = pi_vector_new();
    pi_spatial_set_update_operation(dstSpatial, _sync_update_fun, syncData, TRUE);
    return syncData;
}

void PI_API pi_free_sync(spatialSync *syncData)
{
    uint i, size;
    useData *use_data;
    size = pi_vector_size(syncData->useDatas);
    for (i = 0; i < size; i++)
    {
        use_data = (useData*)pi_vector_pop(syncData->useDatas);
        pi_free(use_data);
    }
	pi_spatial_remove_update_operation(syncData->dstSpatial, syncData);
    pi_vector_free(syncData->useDatas);
    pi_free(syncData);
}

void PI_API pi_add_sync_data(spatialSync *syncData, PiSpatial *src, int type, PiMatrix4 *local_matrix)
{
   useData *data = pi_new0(useData, 1);
   data->useType = type;
   data->spatial = src;
   if (local_matrix == NULL)
   {
       pi_mat4_set_identity(&data->offsetMatrix);
   }
   else
   {
       pi_mat4_copy(&data->offsetMatrix, local_matrix);
   }
   _sync_fun(syncData->dstSpatial, data);
   pi_vector_push(syncData->useDatas, data);
}

void PI_API pi_remove_sync_data(spatialSync *syncData, PiSpatial *src)
{
    uint i, size;
    useData *use_data;
    size = pi_vector_size(syncData->useDatas);
    for (i = 0; i < size; i++)
    {
		use_data = (useData *)pi_vector_get(syncData->useDatas, i);
        if (use_data->spatial == src)
        {
            pi_vector_remove(syncData->useDatas, i);
            pi_free(use_data);
            break;
        }
    }
}