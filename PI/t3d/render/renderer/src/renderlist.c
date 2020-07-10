#include <renderlist.h>

PiVector* PI_API pi_renderlist_new()
{
	return pi_vector_new();
}

PiBool PI_API pi_renderlist_free(PiVector *list)
{
	if(list != NULL)
	{
		pi_vector_free(list);
	}
	return TRUE;
}

/**
 * 不透明排序
 * 按优先级：从小到大
 * 优先级相同的，按纹理排序
 */
static PiCompR PI_API solid_sort_fun(PiCamera *camera, const PiEntity *pa, const PiEntity *pb)
{
	sint priority_a = (sint)pi_entity_get_bind((PiEntity *)pa, EBT_PRIORITY);
	sint priority_b = (sint)pi_entity_get_bind((PiEntity *)pb, EBT_PRIORITY);

	if(priority_a != priority_b)
	{
		/* 优先级，越小越优先 */
		return priority_a > priority_b ? PI_COMP_GREAT : PI_COMP_LESS;
	}
	if(pa->material->vs != pb->material->vs)
	{
		return pa->material->vs > pb->material->vs ? PI_COMP_GREAT : PI_COMP_LESS;
	}
	if(pa->material->fs != pb->material->fs)
	{
		return pa->material->fs > pb->material->fs ? PI_COMP_GREAT : PI_COMP_LESS;
	}
	if(pa->mesh != pb->mesh)
	{
		return pa->mesh > pb->mesh ? PI_COMP_GREAT : PI_COMP_LESS;
	}

	{
		float dis_sq_a, dis_sq_b;

		PiVector3 pos_a, pos_b;
		PiVector3 *cam_pos = pi_camera_get_location(camera);

		PiMatrix4 *m_a = pi_entity_get_world_matrix((PiEntity *)pa);
		PiMatrix4 *m_b = pi_entity_get_world_matrix((PiEntity *)pb);

		pi_vec3_set(&pos_a, m_a->m[0][3], m_a->m[1][3], m_a->m[2][3]);
		pi_vec3_set(&pos_b, m_b->m[0][3], m_b->m[1][3], m_b->m[2][3]);

		dis_sq_a = pi_vec3_distance_square(&pos_a, cam_pos);
		dis_sq_b = pi_vec3_distance_square(&pos_b, cam_pos);

		if(dis_sq_a != dis_sq_b)
		{
			/* 不透明物体，从前往后渲 */
			return dis_sq_a > dis_sq_b ? PI_COMP_GREAT : PI_COMP_LESS;
		}
	}

	return PI_COMP_EQUAL;
}

/**
 * 透明排序
 */
static PiCompR PI_API trans_sort_fun(PiCamera *camera, const PiEntity *pa, const PiEntity *pb)
{
	float dis_sq_a, dis_sq_b;

	PiVector3 pos_a, pos_b;
	
	sint priority_a = (sint)pi_entity_get_bind((PiEntity *)pa, EBT_PRIORITY);
	sint priority_b = (sint)pi_entity_get_bind((PiEntity *)pb, EBT_PRIORITY);

	PiVector3 *cam_pos;
	PiMatrix4 *m_a;
	PiMatrix4 *m_b;

	

	if(priority_a != priority_b)
	{
		/* 优先级，越小越优先 */
		return priority_a > priority_b ? PI_COMP_GREAT : PI_COMP_LESS;
	}
	cam_pos = pi_camera_get_location(camera);
	m_a = pi_entity_get_world_matrix((PiEntity *)pa);
	m_b = pi_entity_get_world_matrix((PiEntity *)pb);
	pi_vec3_set(&pos_a, m_a->m[0][3], m_a->m[1][3], m_a->m[2][3]);
	pi_vec3_set(&pos_b, m_b->m[0][3], m_b->m[1][3], m_b->m[2][3]);

	dis_sq_a = pi_vec3_distance_square(&pos_a, cam_pos);
	dis_sq_b = pi_vec3_distance_square(&pos_b, cam_pos);
	if(dis_sq_a != dis_sq_b)
	{
		/* 透明物体，从后往前渲 */
		return dis_sq_a > dis_sq_b ? PI_COMP_LESS : PI_COMP_GREAT;
	}
	if(pa->material->vs != pb->material->vs)
	{
		return pa->material->vs > pb->material->vs ? PI_COMP_GREAT : PI_COMP_LESS;
	}
	if(pa->material->fs != pb->material->fs)
	{
		return pa->material->fs > pb->material->fs ? PI_COMP_GREAT : PI_COMP_LESS;
	}
	if(pa->mesh != pb->mesh)
	{
		return pa->mesh > pb->mesh ? PI_COMP_GREAT : PI_COMP_LESS;
	}
	return PI_COMP_EQUAL;
}

PiCompareFunc PI_API pi_renderlist_get_sort_func(SortType type)
{
	PiCompareFunc func = NULL;

	switch (type)
	{
	case SORT_SOLID:
		func = solid_sort_fun;
		break;
	case SORT_TRANS:
		func = trans_sort_fun;
		break;
	default:
		break;
	}
	return func;
}

PiBool PI_API pi_renderlist_sort(PiVector *list, PiCompareFunc func, void *user_data)
{
	if(func != NULL)
	{
		pi_vector_sort(list, func, user_data);
	}

	return TRUE;
}