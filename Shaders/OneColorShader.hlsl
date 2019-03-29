#include "Common.hlsl"

struct OneColorVertexOutput
{
	float4 Position : SV_POSITION;
#if USING_VERTEX_SHADER_LAYER && USING_LAYERS
	uint LayerIndex : SV_RenderTargetArrayIndex;
#endif
};

#ifndef USING_NDC_POSITIONS
#define USING_NDC_POSITIONS 1
#endif

void MainVertexShader(
	in float4 inPosition : ATTRIBUTE0,
#if USING_VERTEX_SHADER_LAYER && USING_LAYERS
	in uint instanceID : SV_InstanceID,
#endif
	out OneColorVertexOutput output
)
{
	output.Position = inPosition;
#if !USING_DNC_POSITIONS
	drawRectangle(inPosition, output.Position);
#endif

#if USING_VERTEX_SHADER_LAYER && USING_LAYERS
	ouput.LayerIndex = instanceID;
#endif // USING_VERTEX_SHADER_LAYER && USING_LAYERS

}

#ifndef NUM_OUTPUTS
#define NUM_OUTPUTS 1
#endif

void MainPixelShaderMRT(out float4 outColor[NUM_OUTPUTS] : SV_Target0)
{
	for (int i = 0; i < NUM_OUTPUTS; i++)
	{
		outColor[i] = ClearShaderUB.DrawColorMRT[i];
	}
}

void MainPixelShader(out float4 outColor : SV_Target0)
{
	outColor = ClearShaderUB.DrawColorMRT[0];
}

void MainLongGPUTask(out float4 outColor : SV_Target0)
{
	outColor = 0;

#if FEATURE_LEVEL >= FEATURE_LEVEL_SM4
	for (int i = 0; i < 4000; i++)
	{
		outColor += 0.0001f * cos(i * 0.0001f) * sin(i * i *0.00001f);
	}
#else
	for (int i = 0; i < 255; i++)
	{
		outColor += 0.001f * cos(i * 0.0001f) * sin(i * i*0.00001f);
	}
#endif

	outColor *= 0.000001f;
}

#if FEATURE_LEVEL >= FEATURE_LEVEL_SM5

float4 FillValue;
RWTexture2D<float4> FillTexture;
float4 Params0;
float4 Params1;
float4 Params2;

[numthreads(8, 8, 1)]
void MainFillTextureCS(uint2 xy : SV_DispatchThreadID)
{
	float width = Params0.x;
	float height = Params0.y;
	bool bUseExcludeRect = (Params0.z != 0);

	float x = xy.x;
	float y = xy.y;

	float includeX0 = Params1.x;
	float includeY0 = Params1.y;
	float includeX1 = Params1.z;
	float includeY1 = Params1.w;

	if (x < width && y < height)
	{
		if (x >= includeX0 && x <= includeX1 && y >= includeY0 && y < includeY1)
		{
			bool bDoWrite = true;
			if (bDoWrite)
			{
				FillTexture[xy] = FillValue;
			}
		}
	}
}
#endif