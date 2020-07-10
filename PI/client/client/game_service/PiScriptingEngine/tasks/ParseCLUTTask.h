#ifndef _PARSE_CLUT_TASK_H_
#define _PARSE_CLUT_TASK_H_

#include <string>

#include "pi_lib.h"
#include "game_service/BaseClass/Task.h"
#include "V8Task.h"
#include "util/include/parse.h"


class ParseCLUTTask : public pi::Task
{
public:
	ParseCLUTTask();
	virtual ~ParseCLUTTask();

	void SetParam(byte* data, uint size, const CallBackType& cb);
	virtual bool Run();
	virtual void Callback();

private:
	CallBackType cb_;

	byte* data_;
	uint size_;

	PiColorLookUpTable* result_;
};

#endif