#ifndef _PARSE_UV_ANIM_TASK_H_
#define _PARSE_UV_ANIM_TASK_H_

#include <string>

#include "pi_lib.h"
#include "game_service/BaseClass/Task.h"
#include "V8Task.h"
#include "util/include/parse.h"


class ParseUVAnimTask : public pi::Task
{
public:
	ParseUVAnimTask();
	virtual ~ParseUVAnimTask();

	void SetParam(byte* data, uint size, const CallBackType& cb);
	virtual bool Run();
	virtual void Callback();

private:
	CallBackType cb_;

	byte* data_;
	uint size_;

	PiUVAnim* result_;
};

#endif