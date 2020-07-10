#include "octree.h"

#define _CHD_NUM 8
#define _MAX_OBJ_NUM 5
#define _MAX_OT_LEVEL 8

#define _LOOSE_K 1.3f

#define _OT_OBJ_SIZE sizeof(_S3dOTObj)
#define _OT_NODE_SIZE sizeof(_S3dOTNode)

#define _OT_OBJ(handle) ((_S3dOTObj*)(pi_ha_ptr(ot->objAlloc, handle)))
#define _OT_NODE(handle) ((_S3dOTNode*)(pi_ha_ptr(ot->nodeAlloc, handle)))

static PiBool _LOOSE_ENABLE = TRUE;

typedef struct
{
	PiAABBBox aabb;	// 本节点的世界坐标系下空间范围
	_S3dOTNodeHandle parent;	// 父节点
	_S3dOTNodeHandle children[_CHD_NUM];	// 子节点
	//_S3dOTLinkHandle objects; // 节点列表
	S3dOTObjHandle objects; // 节点列表,注意:第一个obj为引用占位
	int32 obj_num; // 本节点直接持有的物体数量
	int32 all_obj_num;	// 包括自己和其所有子节点所持有的物体数量
	PiBool splitted;	//此节点是否执行过分裂
} _S3dOTNode;

typedef struct
{
	PiAABBBox aabb;		// 物件于世界坐标系下aabb信息
	_S3dOTNodeHandle owner;	// 持有该物体的八叉树节点
	uint32 mask;	//掩码
	void *user_data; // 物件附加数据
	S3dOTObjHandle prior;
	S3dOTObjHandle next;
} _S3dOTObj;

static void _loose(PiAABBBox *dst, PiAABBBox *src)
{
	float d_k = _LOOSE_K - 1.0f;
	float dx, dy, dz;

	dx = (src->maxPt.x - src->minPt.x) * d_k / 2.0f;
	dy = (src->maxPt.y - src->minPt.y) * d_k / 2.0f;
	dz = (src->maxPt.z - src->minPt.z) * d_k / 2.0f;
	dst->maxPt.x = src->maxPt.x + dx;
	dst->minPt.x = src->minPt.x - dx;
	dst->maxPt.y = src->maxPt.y + dy;
	dst->minPt.y = src->minPt.y - dy;
	dst->maxPt.z = src->maxPt.z + dz;
	dst->minPt.z = src->minPt.z - dz;
}

static void _loose_inverse(PiAABBBox *dst, PiAABBBox *src)
{
	float d_k = _LOOSE_K;
	float dx, dy, dz;

	dx = src->maxPt.x - src->minPt.x;
	dy = src->maxPt.y - src->minPt.y;
	dz = src->maxPt.z - src->minPt.z;
	dx = (dx - (dx / d_k)) / 2.0f;
	dy = (dy - (dy / d_k)) / 2.0f;
	dz = (dz - (dz / d_k)) / 2.0f;

	dst->maxPt.x = src->maxPt.x - dx;
	dst->minPt.x = src->minPt.x + dx;
	dst->maxPt.y = src->maxPt.y - dy;
	dst->minPt.y = src->minPt.y + dy;
	dst->maxPt.z = src->maxPt.z - dz;
	dst->minPt.z = src->minPt.z + dz;
}

static S3dOTObjHandle _node_get_objs(S3dOctree *ot, _S3dOTNodeHandle node)
{
	_S3dOTNode *pNode = _OT_NODE(node);
	_S3dOTObj *objs = _OT_OBJ(pNode->objects);
	return objs->next;
}

static void _node_get_inside_obj(S3dOctree *ot, _S3dOTNodeHandle node, uint mask, PiVector *list, ResultOperation operation, void *user_data)
{
	_S3dOTNodeHandle child_node;
	_S3dOTNode *pNode = _OT_NODE(node);
	_S3dOTNode *pChildNode;
	S3dOTObjHandle obj = _node_get_objs(ot, node);
	_S3dOTObj *pObj;
	void *result;
	int i;

	while (obj != 0)
	{
		pObj = _OT_OBJ(obj);

		if ((pObj->mask & mask))
		{
			if (operation != NULL)
			{
				result = operation(obj, user_data);
			}
			else
			{
				result = (void *)(intptr_t)obj;
			}
			pi_vector_push(list, result);
		}

		obj = pObj->next;
	}

	if (pNode->splitted)
	{
		for (i = 0; i < _CHD_NUM; i++)
		{
			child_node = pNode->children[i];
			if (child_node != 0)
			{
				pChildNode = _OT_NODE(child_node);
				if (pChildNode->all_obj_num > 0)
				{
					_node_get_inside_obj(ot, child_node, mask, list, operation, user_data);
				}
			}
		}
	}
}

static void _node_update_counter(S3dOctree *ot, _S3dOTNodeHandle node, int32 value)
{
	_S3dOTNodeHandle parent;
	_S3dOTNode *pNode = _OT_NODE(node);
	_S3dOTNode *pParentNode = _OT_NODE(node);
	pNode->obj_num += value;
	pNode->all_obj_num += value;
	parent = pNode->parent;
	while (parent != 0)
	{
		pParentNode = _OT_NODE(parent);
		pParentNode->all_obj_num += value;
		parent = pParentNode->parent;
	}
}

static int32 _get_child_index(S3dOctree *ot, _S3dOTNodeHandle node, PiAABBBox *box)
{
	int32 nx, ny, nz;
	float cx, cy, cz;
	_S3dOTNode *pNode = _OT_NODE(node);

	cx = (pNode->aabb.minPt.x + pNode->aabb.maxPt.x) / 2;
	cy = (pNode->aabb.minPt.y + pNode->aabb.maxPt.y) / 2;
	cz = (pNode->aabb.minPt.z + pNode->aabb.maxPt.z) / 2;

	if (box->maxPt.x <= cx)
	{
		nx = 0;
	}
	else if (box->minPt.x >= cx)
	{
		nx = 1;
	}
	else
	{
		return -1;
	}

	if (box->maxPt.y <= cy)
	{
		ny = 0;
	}
	else if (box->minPt.y >= cy)
	{
		ny = 1;
	}
	else
	{
		return -1;
	}

	if (box->maxPt.z <= cz)
	{
		nz = 0;
	}
	else if (box->minPt.z >= cz)
	{
		nz = 1;
	}
	else
	{
		return -1;
	}

	return (nz << 2 | ny << 1 | nx);
}

static void _get_child_box(S3dOctree *ot, _S3dOTNodeHandle node, int32 child_index, PiAABBBox *box)
{
	PiVector3 *min, *max, *child_min, *child_max;
	PiVector3 center;
	_S3dOTNode *pNode = _OT_NODE(node);

	min = &pNode->aabb.minPt;
	max = &pNode->aabb.maxPt;
	pi_vec3_set(&center, (max->x + min->x) / 2, (max->y + min->y) / 2, (max->z + min->z) / 2);
	child_min = &box->minPt;
	child_max = &box->maxPt;

	switch (child_index)
	{
	case 0: // left-top-back
		pi_vec3_set(child_min, min->x, min->y, min->z);
		pi_vec3_set(child_max, center.x, center.y, center.z);
		PI_ASSERT(_get_child_index(ot, node, box) == 0, "conflict in aabb-box translation");
		break;
	case 1: // right-top-back
		pi_vec3_set(child_min, center.x, min->y, min->z);
		pi_vec3_set(child_max, max->x, center.y, center.z);
		PI_ASSERT(_get_child_index(ot, node, box) == 1, "conflict in aabb-box translation");
		break;
	case 2: // left-top-front
		pi_vec3_set(child_min, min->x, center.y, min->z);
		pi_vec3_set(child_max, center.x, max->y, center.z);
		PI_ASSERT(_get_child_index(ot, node, box) == 2, "conflict in aabb-box translation");
		break;
	case 3: // right-top-front
		pi_vec3_set(child_min, center.x, center.y, min->z);
		pi_vec3_set(child_max, max->x, max->y, center.z);
		PI_ASSERT(_get_child_index(ot, node, box) == 3, "conflict in aabb-box translation");
		break;
	case 4: // left-bottom-back
		pi_vec3_set(child_min, min->x, min->y, center.z);
		pi_vec3_set(child_max, center.x, center.y, max->z);
		PI_ASSERT(_get_child_index(ot, node, box) == 4, "conflict in aabb-box translation");
		break;
	case 5: // right-bottom-back
		pi_vec3_set(child_min, center.x, min->y, center.z);
		pi_vec3_set(child_max, max->x, center.y, max->z);
		PI_ASSERT(_get_child_index(ot, node, box) == 5, "conflict in aabb-box translation");
		break;
	case 6: // left-bottom-front
		pi_vec3_set(child_min, min->x, center.y, center.z);
		pi_vec3_set(child_max, center.x, max->y, max->z);
		PI_ASSERT(_get_child_index(ot, node, box) == 6, "conflict in aabb-box translation");
		break;
	case 7: // right-bottom-front
		pi_vec3_set(child_min, center.x, center.y, center.z);
		pi_vec3_set(child_max, max->x, max->y, max->z);
		PI_ASSERT(_get_child_index(ot, node, box) == 7, "conflict in aabb-box translation");
		break;
	}
}

static int32 _get_child_index_loose(S3dOctree *ot, _S3dOTNodeHandle node, PiAABBBox *box)
{
	_S3dOTNode *node_ptr = _OT_NODE(node);
	float d_k = _LOOSE_K;
	float dx, dy, dz, dbx, dby, dbz;
	int32 nx, ny, nz;
	PiAABBBox original_aabb;
	PiVector3 center;
	PiVector3 center_box;
	PiBool f_n, f_p;

	dx = node_ptr->aabb.maxPt.x - node_ptr->aabb.minPt.x;
	dy = node_ptr->aabb.maxPt.y - node_ptr->aabb.minPt.y;
	dz = node_ptr->aabb.maxPt.z - node_ptr->aabb.minPt.z;
	dx = (dx - (dx / d_k)) / 4.0f;
	dy = (dy - (dy / d_k)) / 4.0f;
	dz = (dz - (dz / d_k)) / 4.0f;

	dbx = box->maxPt.x - box->minPt.x;
	dby = box->maxPt.y - box->minPt.y;
	dbz = box->maxPt.z - box->minPt.z;

	original_aabb.maxPt.x = node_ptr->aabb.maxPt.x - dx * 2.0f;
	original_aabb.minPt.x = node_ptr->aabb.minPt.x + dx * 2.0f;
	original_aabb.maxPt.y = node_ptr->aabb.maxPt.y - dy * 2.0f;
	original_aabb.minPt.y = node_ptr->aabb.minPt.y + dy * 2.0f;
	original_aabb.maxPt.z = node_ptr->aabb.maxPt.z - dz * 2.0f;
	original_aabb.minPt.z = node_ptr->aabb.minPt.z + dz * 2.0f;

	pi_aabb_get_center(&node_ptr->aabb, &center);
	pi_aabb_get_center(box, &center_box);

	f_n = FALSE;
	f_p = FALSE;
	if (box->maxPt.x <= (center.x + dx) && box->minPt.x > (original_aabb.minPt.x - dx))
	{
		f_n = TRUE;
	}
	if (box->minPt.x >= (center.x - dx) && box->maxPt.x < (original_aabb.maxPt.x + dx))
	{
		f_p = TRUE;
	}
	if (f_n && f_p)
	{
		nx = center_box.x < center.x ? 0 : 1;
	}
	else if (f_n || f_p)
	{
		nx = f_n ? 0 : 1;
	}
	else
	{
		return dbx > dx ? -1 : -2;
	}

	f_n = FALSE;
	f_p = FALSE;
	if (box->maxPt.y <= (center.y + dy) && box->minPt.y > (original_aabb.minPt.y - dy))
	{
		f_n = TRUE;
	}
	if (box->minPt.y >= (center.y - dy) && box->maxPt.y < (original_aabb.maxPt.y + dy))
	{
		f_p = TRUE;
	}
	if (f_n && f_p)
	{
		ny = center_box.y < center.y ? 0 : 1;
	}
	else if (f_n || f_p)
	{
		ny = f_n ? 0 : 1;
	}
	else
	{
		return dby > dy ? -1 : -2;
	}

	f_n = FALSE;
	f_p = FALSE;
	if (box->maxPt.z <= (center.z + dz) && box->minPt.z > (original_aabb.minPt.z - dz))
	{
		f_n = TRUE;
	}
	if (box->minPt.z >= (center.z - dz) && box->maxPt.z < (original_aabb.maxPt.z + dz))
	{
		f_p = TRUE;
	}
	if (f_n && f_p)
	{
		nz = center_box.z < center.z ? 0 : 1;
	}
	else if (f_n || f_p)
	{
		nz = f_n ? 0 : 1;
	}
	else
	{
		return dbz > dz ? -1 : -2;
	}

	return (nz << 2 | ny << 1 | nx);
}

static void _get_child_box_loose(S3dOctree *ot, _S3dOTNodeHandle node, int32 child_index, PiAABBBox *box)
{
	_S3dOTNode *pNode = _OT_NODE(node);

	_loose_inverse(&pNode->aabb, &pNode->aabb);
	_get_child_box(ot, node, child_index, box);
	_loose(&pNode->aabb, &pNode->aabb);
}

static _S3dOTNodeHandle _node_create(S3dOctree *ot, _S3dOTNodeHandle parent, float min_x, float min_y, float min_z, float max_x, float max_y, float max_z)
{
	_S3dOTNodeHandle node = pi_ha_create0(ot->nodeAlloc);
	_S3dOTNode *pNode = _OT_NODE(node);
	pi_vec3_set(&pNode->aabb.minPt, min_x, min_y, min_z);
	pi_vec3_set(&pNode->aabb.maxPt, max_x, max_y, max_z);
	pNode->objects = ot_obj_create(ot, NULL, 0, 0, 0, 0, 0, 0);
	pNode->parent = parent;

	if (_LOOSE_ENABLE)
	{
		_loose(&pNode->aabb, &pNode->aabb);
	}

	return node;
}

static uint _node_destroy(S3dOctree *ot, _S3dOTNodeHandle node, S3dOTObjHandle *obj_buffer)
{
	//销毁八叉树节点及其子节点,移除并返回所有被销毁节点上仍持有的物件
	uint i, count = 0;
	S3dOTObjHandle obj_head = 0, obj_tail = 0, res_obj = 0;
	_S3dOTObj *obj_ptr = NULL;
	_S3dOTNode *pNode = _OT_NODE(node);

	for (i = 0; i < _CHD_NUM; i++)
	{
		_S3dOTNodeHandle child = pNode->children[i];
		if (child != 0)
		{
			count += _node_destroy(ot, child, &res_obj);
			pNode = _OT_NODE(node);	//注意：_node_destroy可能导致栈指针无效
			if (res_obj != 0)
			{
				if (obj_head == 0)
				{
					obj_head = obj_tail = res_obj;
					obj_ptr = _OT_OBJ(obj_tail);
					while (obj_ptr->next != 0)
					{
						obj_tail = obj_ptr->next;
						obj_ptr = _OT_OBJ(obj_tail);
					}
				}
				else
				{
					obj_ptr->next = res_obj;
					obj_ptr = _OT_OBJ(res_obj);
					obj_ptr->prior = obj_tail;
					obj_tail = res_obj;
					while (obj_ptr->next != 0)
					{
						obj_tail = obj_ptr->next;
						obj_ptr = _OT_OBJ(obj_tail);
					}
				}
			}
		}
	}

	res_obj = _node_get_objs(ot, node);
	if (res_obj != 0)
	{
		obj_ptr = _OT_OBJ(pNode->objects);
		obj_ptr->next = 0;
		obj_ptr = _OT_OBJ(res_obj);
		obj_ptr->prior = 0;
		if (obj_head == 0)
		{
			obj_head = res_obj;
		}
		else
		{
			obj_ptr->prior = obj_tail;
			obj_ptr = _OT_OBJ(obj_tail);
			obj_ptr->next = res_obj;
		}
		PI_ASSERT(pNode->all_obj_num == pNode->obj_num, "Node destroy error!");
		count += pNode->obj_num;
	}
	_node_update_counter(ot, node, -pNode->obj_num);
	ot_obj_destroy(ot, pNode->objects);
	pi_ha_delete(ot->nodeAlloc, node);
	*obj_buffer = obj_head;

	return count;
}

static void _node_insert(S3dOctree *ot, _S3dOTNodeHandle node, S3dOTObjHandle id)
{
	//将指定的物件插入到指定的Node中
	int32 i = 0;
	_S3dOTNodeHandle child;
	PiAABBBox child_aabb;
	PiVector3 min_cell;
	_S3dOTObj *obj_ptr = _OT_OBJ(id);
	_S3dOTObj *obj_prior_ptr;
	_S3dOTObj *obj_next_ptr;
	_S3dOTObj *obj_current_ptr;
	S3dOTObjHandle obj_prior, obj_next, obj_current;
	_S3dOTNode *pRootNode = _OT_NODE(ot->root);
	_S3dOTNode *pNode = _OT_NODE(node);

	PI_ASSERT(obj_ptr->next == 0 && obj_ptr->prior == 0 && obj_ptr->owner == 0, "Object %d already inserted", id);

	pi_vec3_set(&min_cell,
	            (pRootNode->aabb.maxPt.x - pRootNode->aabb.minPt.x) / (1 << _MAX_OT_LEVEL),
	            (pRootNode->aabb.maxPt.y - pRootNode->aabb.minPt.y) / (1 << _MAX_OT_LEVEL),
	            (pRootNode->aabb.maxPt.z - pRootNode->aabb.minPt.z) / (1 << _MAX_OT_LEVEL));
	if ((!pNode->splitted && pNode->obj_num < _MAX_OBJ_NUM)
	        || (pNode->aabb.maxPt.x - pNode->aabb.minPt.x) < min_cell.x
	        || (pNode->aabb.maxPt.y - pNode->aabb.minPt.y) < min_cell.y
	        || (pNode->aabb.maxPt.z - pNode->aabb.minPt.z) < min_cell.z)
	{
		//对于未分裂过且小于物件数量上限的节点或节点到达分裂上限,直接插入
		obj_prior = pNode->objects;
		obj_prior_ptr = _OT_OBJ(obj_prior);
		obj_next = obj_prior_ptr->next;
		if (obj_next != 0)
		{
			obj_next_ptr = _OT_OBJ(obj_prior_ptr->next);
			obj_next_ptr->prior = id;
		}
		obj_prior_ptr->next = id;
		obj_ptr->prior = obj_prior;
		obj_ptr->next = obj_next;
		obj_ptr->owner = node;
		_node_update_counter(ot, node, 1);
	}
	else if (pNode->splitted)
	{
		//对于分裂过的节点,不需要再对节点上已有的物件做下放检查,直接下放插入的物件
		if (_LOOSE_ENABLE)
		{
			i = _get_child_index_loose(ot, node, &obj_ptr->aabb);
		}
		else
		{
			i = _get_child_index(ot, node, &obj_ptr->aabb);
		}
		if (i < 0)
		{
			obj_prior = pNode->objects;
			obj_prior_ptr = _OT_OBJ(obj_prior);
			obj_next = obj_prior_ptr->next;
			if (obj_next != 0)
			{
				obj_next_ptr = _OT_OBJ(obj_prior_ptr->next);
				obj_next_ptr->prior = id;
			}
			obj_prior_ptr->next = id;
			obj_ptr->prior = obj_prior;
			obj_ptr->next = obj_next;
			obj_ptr->owner = node;
			_node_update_counter(ot, node, 1);
		}
		else
		{
			child = pNode->children[i];
			if (child == 0)
			{
				if (_LOOSE_ENABLE)
				{
					_get_child_box_loose(ot, node, i, &child_aabb);
				}
				else
				{
					_get_child_box(ot, node, i, &child_aabb);
				}
				child = _node_create(ot, node, child_aabb.minPt.x, child_aabb.minPt.y, child_aabb.minPt.z, child_aabb.maxPt.x, child_aabb.maxPt.y, child_aabb.maxPt.z);
				//PI_ASSERT(cube2aabb(&_OT_NODE(child)->aabb, &obj_ptr->aabb), "LOOSE ERROR!!!!");
				pNode = _OT_NODE(node);	//注意：_node_create调用可能导致栈指针无效
				pNode->children[i] = child;
			}
			_node_insert(ot, child, id);
		}
	}
	else
	{
		//需要执行节点分裂,尝试将已有物件逐个下放
		obj_prior = pNode->objects;
		obj_prior_ptr = _OT_OBJ(obj_prior);
		while (obj_prior_ptr->next != 0)
		{
			obj_current = obj_prior_ptr->next;
			obj_current_ptr = _OT_OBJ(obj_current);
			if (_LOOSE_ENABLE)
			{
				i = _get_child_index_loose(ot, node, &obj_current_ptr->aabb);
			}
			else
			{
				i = _get_child_index(ot, node, &obj_current_ptr->aabb);
			}
			if (i >= 0)
			{
				child = pNode->children[i];
				if (child == 0)
				{
					if (_LOOSE_ENABLE)
					{
						_get_child_box_loose(ot, node, i, &child_aabb);
					}
					else
					{
						_get_child_box(ot, node, i, &child_aabb);
					}
					child = _node_create(ot, node, child_aabb.minPt.x, child_aabb.minPt.y, child_aabb.minPt.z, child_aabb.maxPt.x, child_aabb.maxPt.y, child_aabb.maxPt.z);
					obj_ptr = _OT_OBJ(id);
					obj_prior_ptr = _OT_OBJ(obj_prior);
					obj_current_ptr = _OT_OBJ(obj_current);
					pNode = _OT_NODE(node);	//注意：_node_create调用可能导致栈指针无效
					pNode->children[i] = child;
				}
				obj_next = obj_current_ptr->next;
				obj_prior_ptr->next = obj_next;
				if (obj_next != 0)
				{
					obj_next_ptr = _OT_OBJ(obj_next);
					obj_next_ptr->prior = obj_prior;
				}
				obj_current_ptr->prior = obj_current_ptr->next = 0;
				_node_update_counter(ot, node, -1);
				obj_current_ptr->owner = 0;
				_node_insert(ot, child, obj_current);
				obj_current = obj_prior;
			}
			obj_prior = obj_current;
			obj_prior_ptr = _OT_OBJ(obj_prior);
		}
		pNode->splitted = TRUE;
		//分裂完成后重新执行插入
		_node_insert(ot, node, id);
	}
}

static void _node_update(S3dOctree *ot, _S3dOTNodeHandle node)
{
	int32 i, count = 0;
	S3dOTObjHandle res_obj, obj;
	_S3dOTObj *obj_ptr;
	_S3dOTNode *pNode = _OT_NODE(node);
	if (pNode->all_obj_num <= _MAX_OBJ_NUM)
	{
		for (i = 0; i < _CHD_NUM; i++)
		{
			if (pNode->children[i] != 0)
			{
				count += _node_destroy(ot, pNode->children[i], &res_obj);
				pNode = _OT_NODE(node);	//注意：_node_destroy可能导致栈指针无效
				if (res_obj != 0)
				{
					obj = res_obj;
					while (obj != 0)
					{
						obj_ptr = _OT_OBJ(obj);
						obj_ptr->owner = node;
						obj = obj_ptr->next;
					}

					obj = pNode->objects;
					obj_ptr = _OT_OBJ(obj);
					while (obj_ptr->next != 0)
					{
						obj = obj_ptr->next;
						obj_ptr = _OT_OBJ(obj);
					}
					obj_ptr->next = res_obj;
					obj_ptr = _OT_OBJ(res_obj);
					obj_ptr->prior = obj;
				}
				pNode->children[i] = 0;
			}
		}
		_node_update_counter(ot, node, count);
		pNode->splitted = FALSE;
	}
	else
	{
		for (i = 0; i < _CHD_NUM; i++)
		{
			if (pNode->children[i] != 0)
			{
				_node_update(ot, pNode->children[i]);
			}
		}
	}
}

S3dOctree *ot_create(float min_x, float min_y, float min_z, float max_x, float max_y, float max_z)
{
	S3dOctree *ot = (S3dOctree *)pi_malloc0(sizeof(S3dOctree));
	ot->nodeAlloc = pi_ha_new(_OT_NODE_SIZE);
	ot->objAlloc = pi_ha_new(_OT_OBJ_SIZE);
	ot->root = _node_create(ot, 0, min_x, min_y, min_z, max_x, max_y, max_z);
	return ot;
}

void ot_destroy(S3dOctree *ot)
{

	pi_ha_free(ot->nodeAlloc);
	pi_ha_free(ot->objAlloc);

	pi_free(ot);
}

void ot_copy(S3dOctree *dst, S3dOctree *src)
{
	dst->root = src->root;
	pi_ha_copy(dst->objAlloc, src->objAlloc, TRUE);
	pi_ha_copy(dst->nodeAlloc, src->nodeAlloc, TRUE);
}

S3dOTObjHandle ot_obj_create(S3dOctree *ot, void *user_data, float min_x, float min_y, float min_z, float max_x, float max_y, float max_z)
{
	S3dOTObjHandle id = pi_ha_create0(ot->objAlloc);
	_S3dOTObj *obj = _OT_OBJ(id);
	obj->user_data = user_data;
	pi_vec3_set(&obj->aabb.minPt, min_x, min_y, min_z);
	pi_vec3_set(&obj->aabb.maxPt, max_x, max_y, max_z);
	obj->mask = 0;
	obj->next = obj->prior = 0;

	return id;
}

void ot_obj_destroy(S3dOctree *ot, S3dOTObjHandle id)
{
	ot_obj_remove(ot, id);

	pi_ha_delete(ot->objAlloc, id);
}

void ot_update(S3dOctree *ot)
{
	//TODO:可以考虑使用Cache,更新时只针对移除过物体的节点进行向上递归
	_node_update(ot, ot->root);
}

void ot_obj_insert(S3dOctree *ot, S3dOTObjHandle id)
{
	_node_insert(ot, ot->root, id);
}

void ot_obj_remove(S3dOctree *ot, S3dOTObjHandle id)
{
	//注意:移除并不会导致节点立即收缩,而是在ot_update时才会收缩节点
	_S3dOTObj *obj = _OT_OBJ(id);
	_S3dOTNodeHandle node = obj->owner;
	_S3dOTObj *prior_ptr;
	_S3dOTObj *next_ptr;
	//_S3dOTNode* pNode = _OT_NODE(node);

	if (node != 0)
	{
		PI_ASSERT(obj->prior != 0, "Object remove error!");
		prior_ptr = _OT_OBJ(obj->prior);
		prior_ptr->next = obj->next;
		if (obj->next != 0)
		{
			next_ptr = _OT_OBJ(obj->next);
			next_ptr->prior = obj->prior;
		}
		obj->next = obj->prior = 0;
		obj->owner = 0;
		_node_update_counter(ot, node, -1);
	}
}

void ot_obj_update(S3dOctree *ot, S3dOTObjHandle id, float min_x, float min_y, float min_z, float max_x, float max_y, float max_z)
{
	_S3dOTObj *obj = _OT_OBJ(id);
	_S3dOTNode *node_ptr;
	PiAABBBox tmp_aabb;
	pi_vec3_set(&obj->aabb.minPt, min_x, min_y, min_z);
	pi_vec3_set(&obj->aabb.maxPt, max_x, max_y, max_z);

	if (obj->owner != 0)
	{
		node_ptr = _OT_NODE(obj->owner);
		pi_aabb_copy(&tmp_aabb, &node_ptr->aabb);
		if (_LOOSE_ENABLE)
		{
			_loose_inverse(&tmp_aabb, &tmp_aabb);
		}
		if (!pi_aabb_is_2in1(&tmp_aabb, &obj->aabb))
		{
			// TODO:对于场景中大部分物件为线性移动,可以考虑向上回溯算法以提升效率
			ot_obj_remove(ot, id);
			ot_obj_insert(ot, id);
		}
	}
	//TODO：对于AABB尺寸发生变化的可能需要从该节点上重新执行插入来下放该物件
}

void *ot_obj_get_user_data(S3dOctree *ot, S3dOTObjHandle id)
{
	_S3dOTObj *obj = _OT_OBJ(id);
	return obj->user_data;
}

uint ot_obj_get_mask(S3dOctree *ot, S3dOTObjHandle id)
{
	_S3dOTObj *obj = _OT_OBJ(id);
	return obj->mask;
}

void ot_obj_set_mask(S3dOctree *ot, S3dOTObjHandle id, uint mask)
{
	_S3dOTObj *obj = _OT_OBJ(id);
	obj->mask = mask;
}


PiAABBBox *ot_obj_get_aabb(S3dOctree *ot, S3dOTObjHandle id)
{
	_S3dOTObj *obj = _OT_OBJ(id);
	return &obj->aabb;
}

uint ot_query(S3dOctree *ot, NodeFilter node_filter, ObjFilter obj_filter, uint mask, PiVector *list, void *query_obj, ResultOperation operation, void *user_data)
{
	uint32 i;
	S3dOTObjHandle obj;
	void *result;

	_S3dOTNodeHandle s[256];
	_S3dOTNodeHandle node;
	uint32 si = 0;

	_S3dOTNode *pNode;
	_S3dOTNode *pChildNode;
	_S3dOTObj *pObj;

	EIntersectState intersect_state;
	s[si] = ot->root;
	++si;
	while (si > 0)
	{
		--si;
		node = s[si];

		pNode = _OT_NODE(node);
		obj = _node_get_objs(ot, node);

		while (obj != 0)
		{
			pObj = _OT_OBJ(obj);

			if ((pObj->mask & mask) && obj_filter(obj, query_obj, user_data))
			{
				if (operation != NULL)
				{
					result = operation(obj, user_data);
				}
				else
				{
					result = (void *)(intptr_t)obj;
				}
				pi_vector_push(list, result);
			}

			obj = pObj->next;
		}

		if (pNode->splitted)
		{
			for (i = 0; i < _CHD_NUM; i++)
			{
				node = pNode->children[i];
				if (node != 0)
				{
					pChildNode = _OT_NODE(node);
					if (pChildNode->all_obj_num > 0)
					{
						intersect_state = node_filter(&pChildNode->aabb, query_obj, user_data);
						if (intersect_state == EIS_INTERSECTS)
							//if(TRUE)
						{
							s[si] = node;
							++si;
						}
						else if (intersect_state == EIS_INSIDE)
						{
							_node_get_inside_obj(ot, node, mask, list, operation, user_data);
						}
					}
				}
			}
		}
	}
	return pi_vector_size(list);
}
