#ifndef _PARSE_IMAGE_TASK_H_
#define _PARSE_IMAGE_TASK_H_

#include <string>

#include "pi_lib.h"
#include "game_service/BaseClass/Task.h"
#include "V8Task.h"
#include "util/include/parse.h"


class ParseImageTask : public pi::Task
{
public:
	ParseImageTask();
	virtual ~ParseImageTask();

	void SetParam(byte* data, uint size, PiBool isDecompress, const CallBackType& cb);
	virtual bool Run();
	virtual void Callback();

private:
	CallBackType cb_;

	byte* data_;
	uint size_;
	PiBool isDecompress_;

	ImageResult* result_;
};

#endif