#ifndef PI_BINDINGS_H_
#define PI_BINDINGS_H_

/// Todo:js��c++��binding��client�����ֽ׶αȽϻ���,����ֱ�Ӱ�,����ʹ��nan,֮����ֱ��ȫ��ʹ��nan����v8pp
/// nan���ں�,v8pp����ģ��,nan��nodejs�Ĺٷ���Ŀ�����������node�Ĳ������,v8pp����Ҫ
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