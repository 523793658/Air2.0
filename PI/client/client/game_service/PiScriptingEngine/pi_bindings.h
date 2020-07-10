#ifndef PI_BINDINGS_H_
#define PI_BINDINGS_H_

/// Todo:js与c++的binding在client里面现阶段比较混乱,部分直接绑定,部分使用nan,之后考虑直接全部使用nan或者v8pp
/// nan基于宏,v8pp基于模板,nan是nodejs的官方项目，因此有依赖node的部分组件,v8pp则不需要
/// nan: https://github.com/nodejs/nan
/// v8pp: https://github.com/pmed/v8pp

#include "v8.h"

class PiBindings {
public:
	PiBindings();
	virtual ~PiBindings();

	v8::Local<v8::FunctionTemplate> CreateGlobalPiObj();
	void BindToPi(const v8::Local<v8::FunctionTemplate>& pi_obj);
	void BindToVCall(const v8::Local<v8::FunctionTemplate>& vcall_obj);
	void BindToV8(const v8::Local<v8::FunctionTemplate>& v8_obj);
	void BindToRes(const v8::Local<v8::FunctionTemplate>& res_obj);
	void BindToWindow(const v8::Local<v8::FunctionTemplate>& window_obj);
	void BindToBacktrace(const v8::Local<v8::FunctionTemplate>& backtrace_obj);
	void BindToHeap(const v8::Local<v8::FunctionTemplate>& heap_obj);
	void BindToCpu(const v8::Local<v8::FunctionTemplate>& cpu_obj);
	void BindToAudio(const v8::Local<v8::FunctionTemplate>& audio_obj);

};

#endif 