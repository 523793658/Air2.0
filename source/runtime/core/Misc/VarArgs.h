#pragma once
#include "CoreType.h"
#include "Misc/CString.h"
namespace Air
{
#define GET_VARARGS_RESULT(msg, msgsize, len, lastarg, fmt, result)	\
	{	\
		va_list ap;	\
		va_start(ap, lastarg);	\
		result = CString::getVarArgs(msg, msgsize, len, fmt, ap);	\
		if(result >= msgsize)	\
		{	\
			result =-1;	\
		}\
	}
}

#define GET_VARARGS_RESULT_WIDE(msg, msgsize, len, lastarg, fmt, result) \
	{\
		va_list ap;	\
		va_start(ap, lastarg);\
		result = CStringWide::getVarArgs(msg, msgsize, len, fmt, ap);\
		if(result >= msgsize)\
		{\
			result = -1;\
		}\
	}

#define GET_VARARGS_RESULT_ANSI(msg, msgsize, len, lastarg, fmt, result) \
	{\
		va_list ap;\
		va_start(ap, lastarg);\
		result = CStringAnsi::getVarArgs(msg, msgsize, len, fmt, ap);\
		if(result >= msgsize)\
		{\
			result = -1;\
		}\
	}

#define GET_VARARGS(msg, msgsize, len, lastarg, fmt) \
	{\
		va_list ap;	\
		va_start(ap, lastarg);	\
		CString::getVarArgs(msg, msgsize, len, fmt, ap);\
	}
