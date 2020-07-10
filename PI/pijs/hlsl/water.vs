float4x4 g_WorldViewProjMatrix ;
float4x4 g_ViewMatrix ;
float4x4 g_WorldMatrix ;

#ifdef DIRECTLY_MAPPING
float2 u_RenderTargetSize ;
#endif
float4x3 g_NormalMatrix ;
struct VertexShaderInput
{
    float4 g_Position : POSITION0;
    float3 g_Normal : NORMAL0;
};
struct VertexShaderOutput
{
    float4 v_WorldPos : TEXCOORD0;
    float4 v_ViewPos : TEXCOORD1;
    float4 pos : POSITION;
    float4 v_ProjPos : TEXCOORD2;
    float3 v_VertexNormal : TEXCOORD3;
};

VertexShaderOutput main( VertexShaderInput input)
{
    VertexShaderOutput output;
    output.v_ProjPos = mul(input.g_Position, g_WorldViewProjMatrix);
#ifdef DIRECTLY_MAPPING
    output.v_ProjPos.x -= u_RenderTargetSize.x;
    output.v_ProjPos.y += u_RenderTargetSize.y;
#endif
    output.v_WorldPos = mul(input.g_Position, g_WorldMatrix);
    output.v_ViewPos = mul(output.v_WorldPos, g_ViewMatrix );
    output.pos = output.v_ProjPos;
    output.v_VertexNormal = normalize(mul(input.g_Normal, g_NormalMatrix).xyz);
    return output;
}