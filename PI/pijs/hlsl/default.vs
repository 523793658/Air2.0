struct VertexShaderInput
{
    float4 g_Position : POSITION;
    float2 g_TexCoord0 : TEXCOORD0;
    float3 g_Normal : NORMAL0;
#ifdef VERTEX_COLOR
    float4 g_Color : COLOR0;
#endif
};

float4x4 g_WorldViewProjMatrix ;
#ifdef DIRECTLY_MAPPING
float2 u_RenderTargetSize ;
#endif

struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
    float3 v_Normal : TEXCOORD2;
#ifdef DEPTH
    float2 v_Depth : TEXCOORD1;
#endif
#ifdef VERTEX_COLOR
    float4 v_Color : COLOR0;
#endif
#ifdef TONE_MAPPING
    float v_luminance : TEXCOORD2;
#endif
};
#ifdef TONE_MAPPING
    sampler2D u_LuminanceMap ;
#endif

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    float4 pos = mul(input.g_Position, g_WorldViewProjMatrix);
#ifdef DIRECTLY_MAPPING
    pos.x -= u_RenderTargetSize.x;
    pos.y += u_RenderTargetSize.y;
#endif
    output.pos = pos;
    output.v_TexCoord0 = input.g_TexCoord0;
#ifdef DEPTH
    output.v_Depth = output.pos.zw;
#endif
#ifdef TONE_MAPPING
    output.v_luminance = tex2Dlod(u_LuminanceMap, float4(0.5, 0.5, 0.0, 0.0));
#endif
    output.v_Normal = input.g_Normal;

#ifdef VERTEX_COLOR
    output.v_Color = input.g_Color;
#endif
    return output;
}
