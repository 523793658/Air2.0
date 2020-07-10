#ifndef SCRIPT_ENGINE_PI_CPU_H_
#define SCRIPT_ENGINE_PI_CPU_H_

#include "v8.h"
#include "nan/nan.h"


class CpuBinding
{
public:
	static NAN_METHOD(StartProfiling);
	static NAN_METHOD(StopProfiling);
	static NAN_METHOD(SetSamplingInterval);

};


#endif