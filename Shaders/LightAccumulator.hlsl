#pragma once

struct LightAccumulator
{
	float3 TotalLight;

	float ScatterableLightLuma;

	float3 ScatterableLight;

	float EstimatedCost;
};

void lightAccumulator_Add(inout LightAccumulator acc, float3 totalLight, float3 scatterableLight, float3 commonMultiplier, const bool bNeedsSeperateSubsurfaceLightAccumulation)
{
	acc.TotalLight += totalLight * commonMultiplier;

	if (bNeedsSeperateSubsurfaceLightAccumulation)
	{

	}
}

float4 lightAccumulator_GetResult(LightAccumulator acc)
{
	float4 ret;
	if (VISUALIZE_LIGHT_CULLING == 1)
	{
		ret = 0.1f * float4(1.0f, 0.25f, 0.075f, 0) * acc.EstimatedCost;
	}
	else
	{
		ret = float4(acc.TotalLight, 0);

	}
	return ret;
}