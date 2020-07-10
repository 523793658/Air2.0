#include "pi_backtrace.h"

#include "pi_lib.h"
#include "game_tools/GameTools.h"
#include "pi_converter.h"
#include "game_service/PiScriptingEngine/backtrace.h"

using v8::Isolate;
using v8::Local;
using v8::Persistent;
using v8::Context;
using v8::String;
using v8::NewStringType;
using v8::HandleScope;
using v8::Value;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::CopyablePersistentTraits;
using v8::Object;
using v8::Array;

NAN_METHOD(BacktraceBinding::Filter)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);

	String::Utf8Value str(info[0]);
	std::string str2 = *str;
	std::string bt = Backtrace::GetInstance().Filter(str2);

	info.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, bt.c_str()));
}

NAN_METHOD(BacktraceBinding::AddPlugin)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);

	// 插件目录
	String::Utf8Value str(info[0]);
	std::string pluginName = *str;

	// 插件文件名
	std::vector<std::string> fileVec;
	Local<Array> fileArray = Local<Array>::Cast(info[1]);
	int fileArrayLen = fileArray->Length();

	for (int i = 0; i < fileArrayLen; i++)
	{
		std::string file = *String::Utf8Value(fileArray->Get(i)->ToString());
		fileVec.push_back(file);
	}

	// 插件内容
	std::vector<std::string> contentVec;
	Local<Array> contentArray = Local<Array>::Cast(info[2]);
	int contentArrayLen = contentArray->Length();

	for (int i = 0; i < contentArrayLen; i++)
	{
		std::string content = *String::Utf8Value(contentArray->Get(i)->ToString());
		contentVec.push_back(content);
	}

	Backtrace::GetInstance().AddPlugin(pluginName, fileVec, contentVec);
}

