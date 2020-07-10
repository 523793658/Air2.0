struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
};

sampler2D u_Tex ;
float g_Time ;
float3 u_Color ;
float2 u_TexSize ;

struct PixelShaderOutput
{
    float4 color : COLOR0;
};

PixelShaderOutput main(PixelShaderInput input)
{
    float2 texCoord = input.v_TexCoord0;
    float2 offset = float2(2.5, 2.5) / u_TexSize;
    float3 color = tex2D(u_Tex, texCoord).rgb;

    float3 colorN = tex2D(u_Tex, texCoord + float2(0, offset.y)).rgb;
    float3 colorW = tex2D(u_Tex, texCoord + float2(-offset.x, 0)).rgb;
    float3 colorE = tex2D(u_Tex, texCoord + float2(offset.x, 0)).rgb;
    float3 colorS = tex2D(u_Tex, texCoord + float2(0, -offset.y)).rgb;

    float lumaM = color.r;
    float lumaN = colorN.r;
    float lumaW = colorW.r;
    float lumaE = colorE.r;
    float lumaS = colorS.r;

    float rangeMin = min(lumaM, min(min(lumaN, lumaW), min(lumaS, lumaE)));
    float rangeMax = max(lumaM, max(max(lumaN, lumaW), max(lumaS, lumaE)));
    float range = rangeMax - rangeMin;

    float final = range * ((sin(g_Time * 5.0) + 1.0) / 1.5 * 0.65 + 0.35) * (1.0 - color.r);

    PixelShaderOutput output;
    output.color = float4(u_Color, final * 1.0);
    return output;
}
