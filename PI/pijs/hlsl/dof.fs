struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
};

sampler2D u_SceneColorTex ;
sampler2D u_SceneDepthTex ;
float4x4 u_ProjMatrixInverse ;
float3 u_FocalPoint;
float3 u_RegionRangeScale;
float2 u_BlurTemplate[41] ;
struct PixelShaderOutput
{
    float4 color : COLOR0;
};

float computeCustomBlurFactor(in float dist, float3 RegionRangeScale)
{
    return saturate((max(dist - RegionRangeScale.x, 0.0) / RegionRangeScale.x) / RegionRangeScale.y) * RegionRangeScale.z;
}

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;

    float depth = tex2D(u_SceneDepthTex, input.v_TexCoord0).r;
    float4 projPos = float4((input.v_TexCoord0.x - 0.5) * 2.0, (0.5 - input.v_TexCoord0.y) * 2.0, depth, 1.0);
    float4 tempPos = mul(projPos, u_ProjMatrixInverse);
    float3 viewPos = tempPos.xyz / tempPos.w;
    viewPos.z *= -1.0;

    float dis = distance(viewPos, u_FocalPoint);
    float blurFactor = computeCustomBlurFactor(dis, u_RegionRangeScale);
    float3 color = float3(0.0, 0.0, 0.0);
    for(int i = 0; i < 41; i++)
    {
        color += tex2D(u_SceneColorTex, input.v_TexCoord0 + u_BlurTemplate[i] * blurFactor).rgb;
    }
    output.color = float4(color.rgb / 41.0, 1.0);
    return output;
}
