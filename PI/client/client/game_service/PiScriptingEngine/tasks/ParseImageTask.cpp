#include "ParseImageTask.h"

#include "game_service/Manager/PiGameServiceManager.h"
#include "game_service/PiScriptingEngine/backtrace.h"
#include "game_service/V8/V8Service.h"


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

ParseImageTask::ParseImageTask()
: result_(nullptr)
{

}

ParseImageTask::~ParseImageTask()
{

}

void ParseImageTask::SetParam(byte* data, uint size, PiBool isDecompress, const CallBackType& cb)
{
	data_ = data;
	size_ = size;
	isDecompress_ = isDecompress;
	cb_ = cb;
}

bool ParseImageTask::Run()
{
	ImageResult* result = app_parse_load_image(data_, size_, isDecompress_);

	result_ = result;

	return true;
}

void ParseImageTask::Callback()
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	V8Service *mpV8Service = (V8Service *)tpManager->GetServiceByName(L"V8");

	Isolate* isolate = mpV8Service->GetIsolate();
	Local<Context> context = mpV8Service->GetContext();
	Context::Scope scope(context);
	HandleScope handle_scope(isolate);

	Local<Object> param = Object::New(isolate);

	param->Set(context,
			   String::NewFromUtf8(isolate, "data", NewStringType::kNormal).ToLocalChecked(),
			   Integer::New(isolate, reinterpret_cast<int32_t>(result_->data)));
	param->Set(context,
			   String::NewFromUtf8(isolate, "type", NewStringType::kNormal).ToLocalChecked(),
			   Integer::New(isolate, result_->type));
	param->Set(context,
			   String::NewFromUtf8(isolate, "width", NewStringType::kNormal).ToLocalChecked(),
			   Integer::New(isolate, result_->width));
	param->Set(context,
			   String::NewFromUtf8(isolate, "height", NewStringType::kNormal).ToLocalChecked(),
			   Integer::New(isolate, result_->height));
	param->Set(context,
			   String::NewFromUtf8(isolate, "depth", NewStringType::kNormal).ToLocalChecked(),
			   Integer::New(isolate, result_->depth));
	param->Set(context,
			   String::NewFromUtf8(isolate, "format", NewStringType::kNormal).ToLocalChecked(),
			   Integer::New(isolate, result_->format));
	param->Set(context,
			   String::NewFromUtf8(isolate, "numMipmap", NewStringType::kNormal).ToLocalChecked(),
			   Integer::New(isolate, result_->numMipmap));
	param->Set(context,
			   String::NewFromUtf8(isolate, "arraySize", NewStringType::kNormal).ToLocalChecked(),
			   Integer::New(isolate, result_->arraySize));

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
	app_parse_image_result_free(result_);
}
