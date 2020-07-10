#ifndef _ParseAudio_H_
#define _ParseAudio_H_
#include <string>

#include "pi_lib.h"
#include "game_service/BaseClass/Task.h"
#include "V8Task.h"
#include "util/include/parse.h"


class ParseAudioTask : public pi::Task
{
public:
	ParseAudioTask();
	virtual ~ParseAudioTask();

	void SetParam(const wchar* path, int format, const CallBackType& cb);
	virtual bool Run();
	virtual void Callback();

private:
	CallBackType cb_;
	wchar* path;
	int format;
	AudioResult* result_;
};





#endif