struct PixelShaderInput
{
	float4 pos : POSITION;
	// DEPTH 定义了后两个成员
	float4 v_TexCoord0 : TEXCOORD0;
    float3 v_WorldPos : TEXCOORD1;
    float2 v_ProjPos : TEXCOORD6;
};

#ifdef DIFFUSE_MAP
sampler2D u_DiffuseMap ;
#endif

float4 u_PlaneParams;

#ifdef ALPHA_MAP
sampler2D u_AlphaMap;
float g_AlphaCullOff;
#endif

struct PixelShaderOutput
{
    float4 color : COLOR0;

#ifdef DEPTH
    float4 depth : COLOR1;
#endif
};

PixelShaderOutput main(PixelShaderInput input)
{
    clip(dot(u_PlaneParams.yzw, input.v_WorldPos) + u_PlaneParams.x);

#ifdef COLOR
    float4 color = float4(1.0, 1.0, 1.0, 1.0);
#else
    float4 color = tex2D(u_DiffuseMap, input.v_TexCoord0);
#endif

    float alpha = 1.0;

#ifdef ALPHA_MAP
    float2 alphaMapTexcoord = input.v_TexCoord0.xy;
	alpha = tex2D(u_AlphaMap, alphaMapTexcoord).a;
    clip(alpha - g_AlphaCullOff - 0.000001);
#endif


    PixelShaderOutput output;
    output.color = color;
#ifdef DEPTH
    float depth = input.v_Depth.x / input.v_Depth.y;
    output.depth = float4(depth, depth, depth, depth);
#endif
    return output;
}
