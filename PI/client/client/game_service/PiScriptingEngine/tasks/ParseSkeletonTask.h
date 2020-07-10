#ifndef _PARSE_SKELETON_TASK_H_
#define _PARSE_SKELETON_TASK_H_

#include <string>

#include "pi_lib.h"
#include "game_service/BaseClass/Task.h"
#include "V8Task.h"
#include "util/include/parse.h"


class ParseSkeletonTask : public pi::Task
{
public:
	ParseSkeletonTask();
	virtual ~ParseSkeletonTask();

	void SetParam(byte* data, uint size, const CallBackType& cb);
	virtual bool Run();
	virtual void Callback();

private:
	CallBackType cb_;

	byte* data_;
	uint size_;

	PiSkeleton* result_;
};

#endif