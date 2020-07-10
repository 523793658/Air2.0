#ifndef PI_CONVERTER_H
#define PI_CONVERTER_H

#include "v8.h"
#include <string>

bool std_wstring_FromV8(v8::Isolate* isolate, v8::Local<v8::Value> input, std::wstring* result);
bool std_string_FromV8(v8::Isolate* isolate, v8::Local<v8::Value> input, std::string* result);
std::string V8ToString(v8::Local<v8::Value> value);
std::wstring V8ToWString(v8::Local<v8::Value> value);

#endif