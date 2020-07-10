#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

#include <string>

#include "pi_lib.h"
#include "game_service/BaseClass/Task.h"
#include "V8Task.h"



class HttpRequestTask : public pi::Task
{
public:
    HttpRequestTask();
    virtual ~HttpRequestTask();

    void SetParam(const std::wstring& url, const CallBackType& cb);

    virtual bool Run();
    virtual void Callback();

private:
    std::wstring url_;
    CallBackType cb_;
    std::wstring result_;
};

#endif