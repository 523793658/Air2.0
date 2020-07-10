#ifndef _ControllerManager_H_
#define _ControllerManager_H_
#include "controller.h"
#include "camera.h"
#define MAX_CHILDREN_NUM 10

typedef struct
{
	void* apply_obj;
	ControllerApplyType type;
}ControllerApplyData;

typedef struct _ControllerNode
{
	ControllerType type;
	//struct _ControllerNode* children[MAX_CHILDREN_NUM];
	PiVector* children;
	struct _ControllerNode* parent;
	PiController* data;
	PiVector apply_array;
}ControllerNode;

typedef struct _ControllerManager
{
	PiHash* controllers;
	ControllerNode* root;
	PiBool is_need_update;
	PiVector* controllersData;
	PiCamera* update_camera;
	uint query_mask;
	PiVector* query_list;
	PiVector* apply_data_pool;
	PiVector* node_pool;
}ControllerManager;



PI_BEGIN_DECLS
ControllerManager* PI_API pi_controller_manager_create(PiVector* query_list);

void PI_API pi_controller_manager_free(ControllerManager* cmgr);

void PI_API pi_controller_manager_add(ControllerManager* cmgr, ControllerType type, PiController* controller);

void PI_API pi_controller_manager_remove(ControllerManager* cmgr, PiController* controller);

void PI_API pi_controller_manager_add_child(ControllerManager* cmgr, PiController* controller, PiController* child);

void PI_API pi_controller_manager_remove_child(ControllerManager* cmgr, PiController* controller, PiController* child);

void PI_API pi_controller_manager_remove_all_children(ControllerManager* cmgr, PiController* controller);

void PI_API pi_controller_manager_add_apply(ControllerManager* cmgr, PiController* controller, ControllerApplyType type, void* obj);

void PI_API pi_controller_manager_remove_apply(ControllerManager* cmgr, PiController* controller, void* obj);

PiVector* PI_API pi_controller_manager_get(ControllerManager* cmgr);

void PI_API pi_controller_manager_get_controller_list(ControllerManager* cmgr);

void PI_API pi_controller_manager_set_update_flag(ControllerManager* cmgr, PiController* controller, PiBool flag);

PI_END_DECLS
#endif