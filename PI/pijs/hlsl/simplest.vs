float4x4 g_WorldViewProjMatrix ;


struct vertexShaderInput
{
    float4 g_Position : POSITION0;
    float2 g_TexCoord0 : TEXCOORD0;
#ifdef VERTEX_COLOR
    float4 g_Color : COLOR0;
#endif
};

struct vertexShaderOutput
{
    float2 v_TexCoord0 : TEXCOORD0;
    float4 v_ProjPos : POSITION;

    #ifdef DEPTH
        float2 v_Depth : TEXCOORD1;
    #endif

#ifdef VERTEX_COLOR
    float4 v_Color : COLOR0;
#endif
};



vertexShaderOutput main(vertexShaderInput input)
{
    vertexShaderOutput output;
    output.v_TexCoord0 = input.g_TexCoord0;

    output.v_ProjPos = mul(input.g_Position, g_WorldViewProjMatrix);

    #ifdef DEPTH
        output.v_Depth = output.v_ProjPos.z / output.v_ProjPos.w;
    #endif

    #ifdef VERTEX_COLOR
		output.v_Color = input.g_Color;
    #endif

    return output;
}