struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
};

struct PixelShaderOutput
{
    float4 color : COLOR0;
};
float u_Params;

sampler2D u_SrcTex;
PixelShaderOutput main(PixelShaderInput input)
{
    float4 color = tex2D(u_SrcTex, input.v_TexCoord0);
    color.rgb *= u_Params;
    float3 r1 = color.rgb * 1.5625 + 1.0;
    color.rgb = color.rgb * r1.rgb - 5.0f;
    r1.rgb = max(color.rgb, 0);
    color.rgb = r1.rgb + 10.0;
    float3 r2 = 1.0 / color.rgb;
    color.rgb = r1.rgb * r2.rgb;
    color.a = 1.0;
    PixelShaderOutput output;
    output.color = color;
    return output;
}
