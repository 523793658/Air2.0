struct PixelShaderInput
{
    float4 pos : POSITION;
    float4 v_TexCoord0 : TEXCOORD0;
};

float g_Time ;
float g_AlphaCullOff ;

#ifdef ALPHA_MAP
sampler2D u_AlphaMap ;
#ifdef ALPHA_UV_ANIM
float2 u_AlphaUVVector ;
#endif
#endif

struct PixelShaderOutput
{
    float4 color : COLOR0;
};

PixelShaderOutput main(PixelShaderInput input)
{
#ifdef ALPHA_MAP
    float2 alphaMapTexcoord = input.v_TexCoord0;
#ifdef ALPHA_UV_ANIM
    alphaMapTexcoord += u_AlphaUVVector * g_Time;
#endif
    float alpha = tex2D(u_AlphaMap, alphaMapTexcoord).a;
    clip(alpha - 0.8);
#endif

    PixelShaderOutput output;
#ifdef DEPTH
    float depth = input.v_TexCoord0.z / input.v_TexCoord0.w;
    output.color = float4(depth, depth, depth, 1.0);
#else
    output.color = float4(1.0, 1.0, 1.0, 1.0);
#endif
    return output;
}
