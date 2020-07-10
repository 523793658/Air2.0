#ifndef PI_H_
#define PI_H_

#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>

#include <pi_lib.h>

#include <libplatform/libplatform.h>
#include <v8-profiler.h>
#include <v8.h>


#define MATE_HANDLE_SCOPE(isolate) v8::HandleScope handle_scope(isolate)

#define MATE_METHOD_ARGS_TYPE   v8::FunctionCallbackInfo<v8::Value>
#define MATE_METHOD_RETURN_TYPE void

#define MATE_METHOD_RETURN_VALUE(value) return info.GetReturnValue().Set(value)
#define MATE_METHOD_RETURN_UNDEFINED()  return
#define MATE_METHOD_RETURN_NULL()       return info.GetReturnValue().SetNull()
#define MATE_METHOD_RETURN(value)       args.Return(value)

#define MATE_STRING_NEW(isolate, data) \
	v8::String::NewFromUtf8(isolate, data, v8::String::kNormalString)
#define MATE_STRING_NEW_FROM_UTF8(isolate, data, length) \
	v8::String::NewFromUtf8(isolate, data, v8::String::kNormalString, length)
#define MATE_STRING_NEW_FROM_UTF16(isolate, data, length) \
	v8::String::NewFromTwoByte(isolate, data, v8::String::kNormalString, length)
#define MATE_STRING_NEW_SYMBOL(isolate, data, length) \
	v8::String::NewFromUtf8(isolate, data, v8::String::kInternalizedString, length)

#define MATE_UNDEFINED(isolate) v8::Undefined(isolate)
#define MATE_TRUE(isolate) v8::True(isolate)
#define MATE_FALSE(isolate) v8::False(isolate)
#define MATE_ARRAY_NEW(isolate, size) v8::Array::New(isolate, size)
#define MATE_NUMBER_NEW(isolate, data) v8::Number::New(isolate, data)
#define MATE_INTEGER_NEW(isolate, data) v8::Integer::New(isolate, data)
#define MATE_INTEGER_NEW_UNSIGNED(isolate, data) \
	v8::Integer::NewFromUnsigned(isolate, data)
#define MATE_EXTERNAL_NEW(isolate, data) v8::External::New(isolate, data)
#define MATE_BOOLEAN_NEW(isolate, data) v8::Boolean::New(isolate, data)
#define MATE_OBJECT_NEW(isolate) v8::Object::New(isolate)

#define MATE_SET_INTERNAL_FIELD_POINTER(object, index, value) \
	object->SetAlignedPointerInInternalField(index, value)
#define MATE_GET_INTERNAL_FIELD_POINTER(object, index) \
	object->GetAlignedPointerFromInternalField(index)

#define MATE_PERSISTENT_INIT(isolate, handle, value) \
	handle(isolate, value)
#define MATE_PERSISTENT_ASSIGN(type, isolate, handle, value) \
	handle.Reset(isolate, value)
#define MATE_PERSISTENT_RESET(handle) \
	handle.Reset()
#define MATE_PERSISTENT_TO_LOCAL(type, isolate, handle) \
	v8::Local<type>::New(isolate, handle)
#define MATE_PERSISTENT_SET_WEAK(handle, parameter, callback) \
	handle.SetWeak(parameter, callback)

#define MATE_WEAK_CALLBACK(name, v8_type, c_type) \
	void name(const v8::WeakCallbackData<v8_type, c_type>& data)
#define MATE_WEAK_CALLBACK_INIT(c_type) \
	c_type* self = data.GetParameter()

#define MATE_THROW_EXCEPTION(isolate, value) \
	isolate->ThrowException(value)


// Generally we should not provide utility macros, but this just makes things
// much more comfortable so we keep it.
#define MATE_METHOD(name) \
	MATE_METHOD_RETURN_TYPE name(const MATE_METHOD_ARGS_TYPE& info)


namespace mate {

	// A v8::Persistent handle to a V8 value which destroys and clears the
	// underlying handle on destruction.
	template <typename T>
	class ScopedPersistent {
	public:
		ScopedPersistent() : isolate_(v8::Isolate::GetCurrent()) {}

		ScopedPersistent(v8::Isolate* isolate, v8::Local<v8::Value> handle)
			: isolate_(isolate) {
			reset(isolate, v8::Local<T>::Cast(handle));
		}

		~ScopedPersistent() {
			reset();
		}

		void reset(v8::Isolate* isolate, v8::Local<T> handle) {
			if (!handle.IsEmpty()) {
				isolate_ = isolate;
				MATE_PERSISTENT_ASSIGN(T, isolate, handle_, handle);
			}
			else {
				reset();
			}
		}

		void reset() {
			MATE_PERSISTENT_RESET(handle_);
		}

		bool IsEmpty() const {
			return handle_.IsEmpty();
		}

		v8::Local<T> NewHandle() const {
			return NewHandle(isolate_);
		}

		v8::Local<T> NewHandle(v8::Isolate* isolate) const {
			if (handle_.IsEmpty())
				return v8::Local<T>();
			return MATE_PERSISTENT_TO_LOCAL(T, isolate, handle_);
		}

		template<typename P, typename C>
		void SetWeak(P* parameter, C callback) {
			MATE_PERSISTENT_SET_WEAK(handle_, parameter, callback);
		}

		v8::Isolate* isolate() const { return isolate_; }

	private:
		v8::Isolate* isolate_;
		v8::Persistent<T> handle_;
	};

}


inline v8::MaybeLocal<v8::String> OneByteString(v8::Isolate* isolate, const char* data, int length = -1) {
	return v8::String::NewFromOneByte(isolate, reinterpret_cast<const uint8_t*>(data), v8::String::kNormalString, length);
}


template <typename TypeName>
inline void SET_METHOD(const TypeName& recv,
							const char* name,
							v8::FunctionCallback callback) {
	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(isolate,
																  callback);
	v8::Local<v8::String> fn_name = v8::String::NewFromUtf8(isolate, name);
	recv->PrototypeTemplate()->Set(fn_name, t);
	t->SetClassName(fn_name);
}

inline void SetPrototypeMethod(
	v8::Local<v8::FunctionTemplate> recv
	, const char* name, v8::FunctionCallback callback)
{
	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(isolate,
																  callback);
	v8::Local<v8::String> fn_name = v8::String::NewFromUtf8(isolate, name);
	recv->PrototypeTemplate()->Set(fn_name, t);
	t->SetClassName(fn_name);
}

template <class TypeName>
inline v8::Local<TypeName> StrongPersistentToLocal(
	const v8::Persistent<TypeName>& persistent) {
	return *reinterpret_cast<v8::Local<TypeName>*>(
		const_cast<v8::Persistent<TypeName>*>(&persistent));
}


#endif