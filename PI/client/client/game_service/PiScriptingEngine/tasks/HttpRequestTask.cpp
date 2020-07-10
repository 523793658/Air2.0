#include "HttpRequestTask.h"
#include "game_tools/GameTools.h"

#include "game_service/Manager/PiGameServiceManager.h"
#include "game_service/PiScriptingEngine/backtrace.h"
#include "game_service/V8/V8Service.h"

#include "curl.h"
#include <string>

using v8::Context;
using v8::Isolate;
using v8::Value;
using v8::HandleScope;
using v8::Integer;
using v8::String;
using v8::Local;
using v8::Function;
using v8::Object;
using v8::NewStringType;

HttpRequestTask::HttpRequestTask()
{

}

HttpRequestTask::~HttpRequestTask()
{

}
static long writer(void *data, int size, int nmemb, std::string &content)
{
    long sizes = size * nmemb;
    std::string temp(reinterpret_cast<const char*>(data), sizes);
    content += temp;
    return sizes;
}

void HttpRequestTask::SetParam(const std::wstring& url, const CallBackType& cb)
{
    url_ = url;
    cb_ = cb;
}

bool HttpRequestTask::Run()
{
    CURL *curl;             //����CURL���͵�ָ��  
    CURLcode code;           //����CURLcode���͵ı��������淵��״̬��  

    std::string url(url_.begin(), url_.end());
    std::string content;

    curl = curl_easy_init();        //��ʼ��һ��CURL���͵�ָ��  

    if (curl != NULL)
    {
        // ����curlѡ��. ����CURLOPT_URL�����û�ָ��url. argv[1]�д�ŵ������д���������ַ  
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);

        //����curl_easy_perform ִ�����ǵ�����.��������صĲ���.  
        code = curl_easy_perform(curl);


        curl_easy_cleanup(curl);
    }


    result_ = std::wstring(pi_str_to_wstr(content.c_str(), PI_CP_UTF8
        ));

    return true;
}

void HttpRequestTask::Callback()
{
    PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
    V8Service *mpV8Service = (V8Service *)tpManager->GetServiceByName(L"V8");

    Isolate* isolate = mpV8Service->GetIsolate();
    Local<Context> context = mpV8Service->GetContext();
    Context::Scope scope(context);
    HandleScope handle_scope(isolate);

    Local<Object> param = Object::New(isolate);

    wchar *str = new wchar[result_.size() + 1];
    memcpy(str, result_.c_str(), result_.size() * 2);
    str[result_.size()] = '\0';
   
    param->Set(context,
        String::NewFromUtf8(isolate, "result", NewStringType::kNormal).ToLocalChecked(),
        String::NewFromTwoByte(isolate, reinterpret_cast<uint16*>(str)));

    Local<Value> argv[] = {
        param
    };


    Local<Function> local = Local<Function>::New(isolate, cb_);
	v8::TryCatch try_catch(isolate);
	Local<Value> result;
	if (!local->Call(context, context->Global(), 1, argv).ToLocal(&result))
	{
		Backtrace::GetInstance().ReportException(isolate, &try_catch);
	}

    delete[] str;
}
