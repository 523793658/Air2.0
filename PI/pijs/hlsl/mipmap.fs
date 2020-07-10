struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
};

struct PixelShaderOutput
{
    float4 color : COLOR0;
};


sampler2D u_SrcTex ;
#ifdef SAMPLE4X4
float2 u_offsetUV ;
float4 downSampler4x4(float2 texCoord)
{
    float4 color = tex2D(u_SrcTex, texCoord + float2(u_offsetUV.x, u_offsetUV.y));
    color += tex2D(u_SrcTex, texCoord + float2(-u_offsetUV.x, -u_offsetUV.y));
    color += tex2D(u_SrcTex, texCoord + float2(-u_offsetUV.x, u_offsetUV.y));
    color += tex2D(u_SrcTex, texCoord + float2(u_offsetUV.x, -u_offsetUV.y));
    color *= 0.25;
    return color;
}
#endif

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
#ifdef SAMPLE4X4
    output.color = downSampler4x4(input.v_TexCoord0);
#else
    output.color = tex2D(u_SrcTex,input.v_TexCoord0);
#endif

    return output;
}
