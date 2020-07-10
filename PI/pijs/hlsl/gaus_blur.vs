struct VertexShaderInput
{
    float4 g_Position : POSITION;
    float2 g_TexCoord0 : TEXCOORD0;


};
struct VertexShaderOutput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
};
float4x4 g_WorldViewProjMatrix ;

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.pos = mul(input.g_Position, g_WorldViewProjMatrix);
    output.v_TexCoord0 = input.g_TexCoord0;
    return output;
}


