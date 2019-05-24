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

	enum EShowFlagInitMode
	{
		ESFIM_Game,
		ESFIM_Editor,
		ESFIM_All0
	};


	struct EngineShowFlags
	{
#define SHOWFLAG_ALWAYS_ACCESSIBLE(a,...) uint32 a : 1; void set##a(bool bVal){a = bVal ? 1 : 0;}

#if BUILD_OPTIMIZED_SHOWFLAGS
#else
#define SHOWFLAG_FIXED_IN_SHIPPING(v, a, b, c) SHOWFLAG_ALWAYS_ACCESSIBLE(a, b, c)
#endif

#include "ShowFlagsValues.inl"

		EngineShowFlags(EShowFlagInitMode initMode)
		{
			init(initMode);
		}

		EngineShowFlags()
		{
			init(ESFIM_Game);
		}

	private:
		void init(EShowFlagInitMode initMode)
		{
			if (initMode == ESFIM_All0)
			{
				Memory::Memset(this, 0x00, sizeof(*this));
				return;
			}

			Memory::Memset(this, 0xff, sizeof(*this));
			setReflectionOverride(false);
			setShaderComplexity(false);
			setStationaryLightOverlap(false);
			setWireframe(false);
			setVisualizeLightCulling(false);
			setPostProcessing(false);
		}
	};


	ENGINE_API void engineShowFlagOrthographicOverride(bool isPerspective, EngineShowFlags& engineShowFlags);

}