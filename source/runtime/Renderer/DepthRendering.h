#pragma once
#include "RendererMininal.h"
namespace Air
{
	enum EDepthDrawingMode
	{
		DDM_None			= 0,
		DDM_NonMaskedOnly	= 1,
		DDM_AllOccluders	= 2,
		DDM_ALLOpaque		= 3,
	};
}