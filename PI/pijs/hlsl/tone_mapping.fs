#define BLOOM_LEVEL 4
float4 sRGBTexture2D(sampler2D tex, float2 texCoord)
{
    float4 color = tex2D(tex, texCoord);
    color.xyz = pow(color.xyz, float3(2.2, 2.2, 2.2));

    return color;
}

float getLuminance(float3 rgbColor)
{
    return rgbColor.r * 0.299 + rgbColor.g * 0.587 + rgbColor.b * 0.114;
}

float3 RGBTosRGB(float3 color)
{
    return pow(color, float3(0.454545, 0.454545, 0.454545));
}

struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
    float v_luminance : TEXCOORD2;
};

sampler2D u_BloomTex0 ;
sampler2D u_BloomTex1 ;
sampler2D u_BloomTex2 ;
sampler2D u_BloomTex3 ;

sampler2D u_SceneTex ;
float4 u_ToneMapParams;
struct PixelShaderOutput
{
    float4 color : COLOR0;
};
PixelShaderOutput main(PixelShaderInput input)
{
    //Bloom
    float4 color = tex2D(u_BloomTex0, input.v_TexCoord0);
    color += tex2D(u_BloomTex1, input.v_TexCoord0);
    color += tex2D(u_BloomTex2, input.v_TexCoord0);
    color += tex2D(u_BloomTex3, input.v_TexCoord0);

    color *= 0.25;

    color += sRGBTexture2D(u_SceneTex, input.v_TexCoord0);

    //Tone Mapping
    float exposure = u_ToneMapParams.y;
    float gaussianScalar = u_ToneMapParams.x;
    float pixelLuminance = getLuminance(color.rgb);
    pixelLuminance = (exposure / input.v_luminance) * pixelLuminance;
    float luminanceSqr = pow((input.v_luminance + gaussianScalar * input.v_luminance), 2.0);
    color *= ( pixelLuminance * ( 1.0 + ( pixelLuminance / ( luminanceSqr ) ) ) ) / ( 1.0 + pixelLuminance );

    PixelShaderOutput output;
    output.color = float4(RGBTosRGB(color.xyz), 1.0);
    return output;
}
