struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
};
sampler2D u_SrcTex ;
float4 u_Offset[2];

struct PixelShaderOutput
{
    float4 color : COLOR0;
};

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    float4 color = tex2D(u_SrcTex, input.v_TexCoord0 + u_Offset[0].xy);
        color += tex2D(u_SrcTex, input.v_TexCoord0 + u_Offset[0].zw);
        color += tex2D(u_SrcTex, input.v_TexCoord0 + u_Offset[1].xy);
        color += tex2D(u_SrcTex, input.v_TexCoord0 + u_Offset[1].zw);
        color *= 0.25;
    output.color = color;
    return output;
}
