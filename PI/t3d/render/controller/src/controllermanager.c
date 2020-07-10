#include "controllermanager.h"

static ControllerApplyData* _get_apply_data(ControllerManager* cmgr)
{
	ControllerApplyData* data = pi_vector_pop(cmgr->apply_data_pool);
	if (data == NULL)
	{
		data = pi_new0(ControllerApplyData, 1);
	}
	return data;
}

static void _retrieve_apply_data(ControllerManager* cmgr, ControllerApplyData* data)
{
	pi_vector_push(cmgr->apply_data_pool, data);
}

static ControllerNode* _get_controller_node(ControllerManager* cmgr)
{
	ControllerNode* node = pi_vector_pop(cmgr->node_pool);
	if (node == NULL)
	{
		node = pi_new0(ControllerNode, 1);
		node->children = pi_vector_new();
	}
	else
	{
		//理论上上次删除节点的时候子节点应该已经清空了，这里应该不需要做。
		pi_vector_clear(&node->apply_array, FALSE);
		pi_vector_clear(node->children, FALSE);
	}
	return node;
}

static void _retrieve_controller_node(ControllerManager* cmgr, ControllerNode* node)
{
	pi_vector_push(cmgr->node_pool, node);
}

static void _remove_child(ControllerNode* parent, ControllerNode* child)
{
	uint i, size;
	ControllerNode* node;
	size = pi_vector_size(parent->children);
	for (i = 0; i < size; i++)
	{
		node = pi_vector_get(parent->children, i);
		if (node == child)
		{
			pi_vector_remove_unorder(parent->children, i);
			return;
		}
	}
}

static void _insert_child(ControllerNode* parent, ControllerNode* child)
{
	pi_vector_push(parent->children, child);
}

static uint PI_API _hash_func(PiController* controller)
{
	return (uint)controller;
}

static PiBool PI_API _hash_equel(PiController* controller1, PiController* controller2)
{
	return controller1 == controller2;
}

ControllerManager* PI_API pi_controller_manager_create(PiVector* query_list)
{
	ControllerManager* cmgr = pi_new0(ControllerManager, 1);
	cmgr->controllers = pi_hash_new(1.6, (PiHashFunc)_hash_func, (PiEqualFunc)_hash_equel);
	cmgr->node_pool = pi_vector_new();
	cmgr->root = _get_controller_node(cmgr);
	cmgr->controllersData = pi_vector_new();
	cmgr->query_list = query_list;
	cmgr->apply_data_pool = pi_vector_new();
	return cmgr;
}

void PI_API pi_controller_manager_free(ControllerManager* cmgr)
{
	pi_hash_free(cmgr->controllers);
	pi_free(cmgr->root);
	pi_vector_free(cmgr->controllersData);
	pi_vector_free(cmgr->apply_data_pool);
	pi_vector_free(cmgr->node_pool);
	pi_free(cmgr);
}

void PI_API pi_controller_manager_add(ControllerManager* cmgr, ControllerType type, PiController* controller)
{
	ControllerNode* node;
	if (!pi_hash_lookup(cmgr->controllers, controller, &node))
	{
		node = _get_controller_node(cmgr);
		node->type = type;
		node->parent = cmgr->root;
		node->data = controller;
		cmgr->is_need_update = TRUE;
		_insert_child(cmgr->root, node);
		pi_hash_insert(cmgr->controllers, controller, node);
	}
}

void PI_API pi_controller_manager_remove(ControllerManager* cmgr, PiController* controller)
{
	ControllerNode* oldNode;
	if (pi_hash_lookup(cmgr->controllers, controller, &oldNode))
	{
		uint i, size;
		ControllerNode* childNode;
		cmgr->is_need_update = TRUE;
		_remove_child(oldNode->parent, oldNode);
		size = pi_vector_size(oldNode->children);
		for (i = 0; i < size; i++)
		{
			childNode = pi_vector_get(oldNode->children, i);
			childNode->parent = cmgr->root;
			_insert_child(cmgr->root, childNode);
		}
		pi_hash_delete(cmgr->controllers, controller, NULL);
		_retrieve_controller_node(cmgr, oldNode);
	}
}

void PI_API pi_controller_manager_add_child(ControllerManager* cmgr, PiController* controller, PiController* child)
{
	ControllerNode *parentNode, *childNode;
	pi_hash_lookup(cmgr->controllers, controller, &parentNode);
	pi_hash_lookup(cmgr->controllers, child, &childNode);
	cmgr->is_need_update = TRUE;
	_remove_child(childNode->parent, childNode);
	childNode->parent = parentNode;
	_insert_child(parentNode, childNode);
}

void PI_API pi_controller_manager_remove_child(ControllerManager* cmgr, PiController* controller, PiController* child)
{
	ControllerNode *parentNode, *childNode;
	pi_hash_lookup(cmgr->controllers, controller, &parentNode);
	pi_hash_lookup(cmgr->controllers, child, &childNode);
	_remove_child(parentNode, childNode);
	childNode->parent = cmgr->root;
	_insert_child(cmgr->root, childNode);
	cmgr->is_need_update = TRUE;
}

void PI_API pi_controller_manager_remove_all_children(ControllerManager* cmgr, PiController* controller)
{
	ControllerNode* node;
	if (pi_hash_lookup(cmgr->controllers, controller, &node))
	{
		ControllerNode* childNode;
		uint i, size;
		size = pi_vector_size(node->children);
		for (i = 0; i < size; i++)
		{
			childNode = pi_vector_get(node->children, i);
			childNode->parent = cmgr->root;
			_insert_child(cmgr->root, childNode);
		}
		pi_vector_clear(node->children, FALSE);
	}
	cmgr->is_need_update = TRUE;
}

void PI_API pi_controller_manager_add_apply(ControllerManager* cmgr, PiController* controller, ControllerApplyType type, void* obj)
{
	ControllerNode* node;
	if (pi_hash_lookup(cmgr->controllers, controller, &node))
	{
		ControllerApplyData* data = _get_apply_data(cmgr);
		data->type = type;
		data->apply_obj = obj;
		pi_vector_push(&node->apply_array, data);
	}
}

void PI_API pi_controller_manager_remove_apply(ControllerManager* cmgr, PiController* controller, void* obj)
{
	ControllerNode* node;
	if (pi_hash_lookup(cmgr->controllers, controller, &node))
	{
		uint i, size;
		ControllerApplyData* data;
		size = pi_vector_size(&node->apply_array);
		for (i = 0; i < size; i++)
		{
			data = pi_vector_get(&node->apply_array, i);
			if (data->apply_obj == obj)
			{
				_retrieve_apply_data(cmgr, data);
				pi_vector_remove_unorder(&node->apply_array, i);
				cmgr->is_need_update = TRUE;
				return;
			}
		}
	}
}

static void _update(ControllerManager* cmgr, ControllerNode* node)
{
	uint i, size;
	pi_vector_push(cmgr->controllersData, node);
	size = pi_vector_size(node->children);
	for (i = 0; i < size; i++)
	{
		_update(cmgr, pi_vector_get(node->children, i));
	}
}

PiVector* PI_API pi_controller_manager_get(ControllerManager* cmgr)
{
	if (cmgr->is_need_update)
	{
		uint i, size;
		ControllerNode* child;
		pi_vector_clear(cmgr->controllersData, FALSE);
		cmgr->is_need_update = FALSE;
		size = pi_vector_size(cmgr->root->children);
		for (i = 0; i < size; i++)
		{
			child = pi_vector_get(cmgr->root->children, i);
			_update(cmgr, child);
		}
	}
	return cmgr->controllersData;
}

void PI_API pi_controller_manager_get_controller_list(ControllerManager* cmgr)
{
	//暂不实现
}

void PI_API pi_controller_manager_set_update_flag(ControllerManager* cmgr, PiController* controller, PiBool flag)
{
	//暂不实现
}