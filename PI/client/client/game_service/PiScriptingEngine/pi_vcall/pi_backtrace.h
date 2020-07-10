#ifndef SCRIPT_ENGINE_PI_BACKTRACE_H_
#define SCRIPT_ENGINE_PI_BACKTRACE_H_

#include "v8.h"
#include "nan/nan.h"


class BacktraceBinding
{
public:
	static NAN_METHOD(Filter);
	static NAN_METHOD(AddPlugin);
};


#endif