#pragma once

#include "ConstantBuffers/View.hlsl"
#include "ConstantBuffers/InstancedView.hlsl"

#include "GeneratedInstancedStereo.hlsl"

static ViewState ResolvedView;

ViewState ResolveView()
{
	return getPrimaryView();
}