#pragma once



#ifndef IS_MONOLITHIC
#define IS_MONOLITHIC	0
#else
#define IS_MONOLITHIC	1
#endif


#ifndef WITH_EDITOR
#define WITH_EDITOR	0
#endif

#ifndef IS_PROGRAM
#define IS_PROGRAM	0
#endif

#ifndef CHECK_PUREVIRTUALS
#define CHECK_PUREVIRTUALS	0
#endif

#ifndef BUILD_DEBUG
#define BUILD_DEBUG		0
#endif

#ifndef BUILD_DEVELOPMENT
#define BUILD_DEVELOPMENT		0
#endif

#ifndef BUILD_TEST
#define BUILD_TEST				0
#endif

#ifndef BUILD_SHIPPING
#define BUILD_SHIPPING			0
#endif


#define USE_KLAYGE_MATH			0

#if BUILD_DEBUG
#define DO_CHECK				1
#endif

#ifndef HACK_HEADER_GENERATOR
#define HACK_HEADER_GENERATOR 0
#endif
