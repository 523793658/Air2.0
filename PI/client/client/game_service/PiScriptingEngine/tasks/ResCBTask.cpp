#include "ResCBTask.h"
#include "game_service/PiScriptingEngine/pi_vcall/remote_object_freer.h"

ResCBTask::ResCBTask()
{
	pt_ = NULL;
}

ResCBTask::~ResCBTask()
{

}


bool ResCBTask::Run()
{
	return true;
}

void ResCBTask::Callback()
{
	if (pt_ != NULL)
	{
		reinterpret_cast<RemoteObjectFreer *>(pt_)->Release();
		pt_ = NULL;
	}
}

void ResCBTask::SetResPoint(void *point)
{
	pt_ = point;
}

