float4 sRGBTexture2D(sampler2D tex, float2 texCoord)
{
    float4 color = tex2D(tex, texCoord);
    color.xyz = pow(color.xyz, float3(2.2, 2.2, 2.2));

    return color;
}

struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
};

sampler2D u_SrcTex ;
float4 u_LightPassParams ;
struct PixelShaderOutput
{
    float4 color : COLOR0;
};

PixelShaderOutput main(PixelShaderInput input)
{
    float4 c = sRGBTexture2D( u_SrcTex, input.v_TexCoord0);
    c = (c - u_LightPassParams.y) / u_LightPassParams.z;
    PixelShaderOutput output;
    output.color = clamp(c * u_LightPassParams.x, 0.0, 1000.0);
    return output;
}
