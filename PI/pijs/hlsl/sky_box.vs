struct VertexShaderInput
{
    float3 pos : POSITION;
};

float4x4 g_WorldViewProjMatrix ;

struct PixelShaderInput
{
    float4 pos : POSITION;
    float3 v_CubeCoord : TEXCOORD0;
};

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    output.pos = mul(float4(input.pos, 1.0), g_WorldViewProjMatrix);
    output.v_CubeCoord = input.pos;
    return output;
}
