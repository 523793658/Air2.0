#ifndef _RES_CB_TASK_H_
#define _RES_CB_TASK_H_

#include <string>

#include "pi_lib.h"
#include "game_service/BaseClass/Task.h"



class ResCBTask : public pi::Task 
{
public:
	ResCBTask();
	virtual ~ResCBTask();

	void SetResPoint(void * point);

	virtual bool Run();
	virtual void Callback();

private:
	void* pt_;
};

#endif