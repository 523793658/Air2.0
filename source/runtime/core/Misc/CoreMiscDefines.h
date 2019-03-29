#pragma once
#ifndef WITH_EDITORONLY_DATA

#if !PLATFORM_CAN_SUPPORT_EDITORONLY_DATA
#define WITH_EDITORONLY_DATA	0
#else
#define WITH_EDITORONLY_DATA	1
#endif


#endif

#ifndef USING_CODE_ANALYSIS
#define USING_CODE_ANALYSIS 0
#endif


#if USING_CODE_ANALYSIS
#else
#define CA_IN 
#define CA_OUT
#define CA_READ_ONLY
#define CA_WRITE_ONLY
#define CA_VALID_POINTER
#define CA_CHECK_RETVAL
#define CA_NO_RETURN
#define CA_SUPPRESS( WarningNumber )
#define CA_ASSUME( Expr )
#define CA_CONSTANT_IF(Condition) if (Condition)
#endif

#if BUILD_DEBUG
#define FORCEINLINE_DEBUGGABLE FORCEINLINE_DEBUGGABLE_ACTUAL
#else
#define FORCEINLINE_DEBUGGABLE	FORCEINLINE
#endif

#if CHECK_PUREVIRTUALS
#define PURE_VIRTRUAL(func, extra) = 0;
#else
#define PURE_VIRTRUAL(func, extra){BOOST_ASSERT(false); extra}
#endif

enum EForceInit
{
	ForceInit,
	ForceInitToZero
};

enum { UNICODE_BOM = 0xfeff };


enum {INDEX_NONE	= -1	};

enum ENoInit { NoInit };