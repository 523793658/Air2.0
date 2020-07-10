#ifndef _LOAD_TASK_H_
#define _LOAD_TASK_H_

#include <string>

#include "pi_lib.h"
#include "game_service/BaseClass/Task.h"
#include "V8Task.h"



class LoadTask : public pi::Task 
{
public:
	LoadTask();
	virtual ~LoadTask();

	void SetParam(const std::wstring& path, const CallBackType& cb);

	virtual bool Run();
	virtual void Callback();

private:
	std::wstring path_;
	CallBackType cb_;

	char* file_buf_; 
	int64 file_size_;
	int error_type_;
	wchar_t* error_message_;
};

#endif