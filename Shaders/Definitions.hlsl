#pragma once

#ifndef MATERIAL_TWOSIDED
#define MATERIAL_TWOSIDED	0
#endif

#ifndef INSTANCED_STEREO
#define INSTANCED_STEREO 0
#endif

#ifndef USING_TESSELLATION
#define USING_TESSELLATION 0
#endif

#ifndef MATERIALBLENDING_MASKED
#define MATERIALBLENDING_MASKED				0
#endif

#if SM5_PROFILE || COMPILER_SUPPORTS_ATTRIBUTES
#define UNROLL					[unroll]
#define LOOP					[loop]
#define BRANCH					[branch]
#define FLATTEN					[flatten]
#define ALLOW_UAV_CONDITION		[allow_uav_condition]
#else

#ifndef UNROLL
#define UNROLL
#endif


#ifndef LOOP					
#define LOOP
#endif

#ifndef BRANCH
#define BRANCH
#endif

#ifndef FLATTEN
#define FLATTEN
#endif

#ifndef ALLOW_UAV_CONDITION
#define ALLOW_UAV_CONDITION
#endif
#endif

