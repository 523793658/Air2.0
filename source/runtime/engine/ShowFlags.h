#pragma once
#include "CoreType.h"
#include "EngineMininal.h"
namespace Air
{
	enum EShowFlagGroup
	{
		SFG_Normal,				// currently one level higher in the hierarchy the the other groups
		SFG_Advanced,
		SFG_PostProcess,
		SFG_CollisionModes,
		SFG_Developer,
		SFG_Visualize,
		SFG_LightTypes,
		SFG_LightingComponents,
		SFG_LightingFeatures,
		SFG_Hidden,
		SFG_Max
	};

#if 0
#define BUILD_OPTIMIZED_SHOWFLAGS	1
#else 
#define BUILD_OPTIMIZED_SHOWFLAGS	0 
#endif



	struct EngineShowFlags
	{
#define SHOWFLAG_ALWAYS_ACCESSIBLE(a,...) uint32 a : 1; void set##a(bool bVal){a = bVal ? 1 : 0;}

#if BUILD_OPTIMIZED_SHOWFLAGS
#else
#define SHOWFLAG_FIXED_IN_SHIPPING(v, a, b, c) SHOWFLAG_ALWAYS_ACCESSIBLE(a, b, c)
#endif

#include "ShowFlagsValues.inl"
	};


	ENGINE_API void engineShowFlagOrthographicOverride(bool isPerspective, EngineShowFlags& engineShowFlags);

}