#include "pi_converter.h"

bool std_wstring_FromV8(v8::Isolate* isolate, v8::Local<v8::Value> val, std::wstring* out)
{
  if (!val->IsString())
      return false; 
  v8::Local<v8::String> str = v8::Local<v8::String>::Cast(val);
  int length= str->Length(); 
  out->resize(length); 
  str->Write((uint16_t*)&(*out)[0], 0, length, v8::String::NO_NULL_TERMINATION);
  return true; 
}

bool std_string_FromV8(v8::Isolate* isolate, v8::Local<v8::Value> val,
                                    std::string* out) {
  if (!val->IsString())
    return false;
  v8::Local<v8::String> str = v8::Local<v8::String>::Cast(val);
  int length = str->Utf8Length();
  out->resize(length);
  str->WriteUtf8(&(*out)[0], length, NULL, v8::String::NO_NULL_TERMINATION);
  return true;
}

std::wstring V8ToWString(v8::Local<v8::Value> value){

  if (value.IsEmpty())
    return std::wstring();
  std::wstring result;
  if (!std_wstring_FromV8(NULL, value, &result))
    return std::wstring();
  return result;
}

std::string V8ToString(v8::Local<v8::Value> value) {
  if (value.IsEmpty())
    return std::string();
  std::string result;
  if (!std_string_FromV8(NULL, value, &result))
    return std::string();
  return result;
}