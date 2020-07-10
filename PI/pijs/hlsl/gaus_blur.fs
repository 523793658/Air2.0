#define weight0     0.199470997
#define weight1     0.176033005
#define weight2     0.120985001
#define weight3     0.0647590011
#define weight4     0.0269949995
#define weight5     0.00876399968
#define weight6     0.00221600011

sampler2D u_SrcTex;
float u_Params;
struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
};
float4 u_Offset[6];
struct PixelShaderOutput
{
    float4 color : COLOR0;
};
PixelShaderOutput main(PixelShaderInput input)
{
    float2 tex1 = input.v_TexCoord0 + u_Offset[0].xy;
    float2 tex2 = input.v_TexCoord0 + u_Offset[0].zw;
    float2 tex3 = input.v_TexCoord0 + u_Offset[1].xy;
    float2 tex4 = input.v_TexCoord0 + u_Offset[1].zw;
    float2 tex5 = input.v_TexCoord0 + u_Offset[2].xy;
    float2 tex6 = input.v_TexCoord0 + u_Offset[2].zw;
    float2 tex7 = input.v_TexCoord0 + u_Offset[3].xy;
    float2 tex8 = input.v_TexCoord0 + u_Offset[3].zw;
    float2 tex9 = input.v_TexCoord0 + u_Offset[4].xy;
    float2 tex10 = input.v_TexCoord0 + u_Offset[4].zw;
    float2 tex11 = input.v_TexCoord0 + u_Offset[5].xy;
    float2 tex12 = input.v_TexCoord0 + u_Offset[5].zw;

    float4 color = tex2D(u_SrcTex, input.v_TexCoord0) * weight0;
    color += tex2D(u_SrcTex, tex1) * weight1;
    color += tex2D(u_SrcTex, tex2) * weight1;
    color += tex2D(u_SrcTex, tex3) * weight2;
    color += tex2D(u_SrcTex, tex4) * weight2;
    color += tex2D(u_SrcTex, tex5) * weight3;
    color += tex2D(u_SrcTex, tex6) * weight3;
    color += tex2D(u_SrcTex, tex7) * weight4;
    color += tex2D(u_SrcTex, tex8) * weight4;
    color += tex2D(u_SrcTex, tex9) * weight5;
    color += tex2D(u_SrcTex, tex10) * weight5;

    PixelShaderOutput output;
    output.color = color * u_Params;
    return output;
}
