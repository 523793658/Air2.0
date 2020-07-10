struct PixelShaderInput
{
    float2 v_TexCoord0 : TEXCOORD0;
    float4 v_ProjPos : POSITION;
};

struct PixelShaderOutput
{
    float4 color : COLOR0;
};
    sampler2D u_DiffuseMap ;
    float4x4 u_uv_Matrix ;

float2 mod(float2 a, float b)
{
    float2 value;
    value.x = a.x - b * floor(a.x / b );
    value.y = a.y - b * floor(a.y / b );
    return value;
}
float4 u_OptionFactor;
float2 u_SizeInv;

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    float4 color = float4(1.0, 1.0, 1.0, 1.0);
    float4 texCoord = float4(input.v_TexCoord0, 1.0, 1.0);
    texCoord = mul(texCoord, u_uv_Matrix);
    float alpha = 0;
    #define threshold 0.2

    #ifdef BLEND
        float alpha0 = tex2D(u_DiffuseMap, texCoord.xy + float2(-1.0, -1.0 ) * u_SizeInv).a;
        float alpha1 = tex2D(u_DiffuseMap, texCoord.xy + float2(0.0, -1.0 ) * u_SizeInv).a;
        float alpha2 = tex2D(u_DiffuseMap, texCoord.xy + float2(1.0, -1.0 ) * u_SizeInv).a;
        float alpha3 = tex2D(u_DiffuseMap, texCoord.xy + float2(-1.0, 0.0 ) * u_SizeInv).a;
        float alpha4 = tex2D(u_DiffuseMap, texCoord.xy + float2(0.0, 0.0 ) * u_SizeInv).a;
        float alpha5 = tex2D(u_DiffuseMap, texCoord.xy + float2(1.0, 0.0 ) * u_SizeInv).a;
        float alpha6 = tex2D(u_DiffuseMap, texCoord.xy + float2(-1.0, 1.0 ) * u_SizeInv).a;
        float alpha7 = tex2D(u_DiffuseMap, texCoord.xy + float2(0.0, 1.0 ) * u_SizeInv).a;
        float alpha8 = tex2D(u_DiffuseMap, texCoord.xy + float2(1.0, 1.0 ) * u_SizeInv).a;


        if(u_OptionFactor.z > 0.5)
        {
            alpha0 = step(threshold, alpha0);
            alpha1 = step(threshold, alpha1);
            alpha2 = step(threshold, alpha2);
            alpha3 = step(threshold, alpha3);
            alpha4 = step(threshold, alpha4);
            alpha5 = step(threshold, alpha5);
            alpha6 = step(threshold, alpha6);
            alpha7 = step(threshold, alpha7);
            alpha8 = step(threshold, alpha8);
        }
        alpha += 0.0947416 * alpha0;
        alpha += 0.118318 * alpha1;
        alpha += 0.0947416 * alpha2;
        alpha += 0.118318 * alpha3;
        alpha += 0.147761 * alpha4;
        alpha += 0.118318 * alpha5;
        alpha += 0.0947416 * alpha6;
        alpha += 0.118318 * alpha7;
        alpha += 0.0947416 * alpha8;
        alpha = saturate(alpha);
    #else
        alpha = tex2D(u_DiffuseMap, texCoord.xy).a;
        clip(alpha - threshold);
    #endif
    clip(alpha - 0.001);
    alpha = alpha * u_OptionFactor.x + u_OptionFactor.y;
    color *= float4(1.0, 1.0, 1.0, alpha);
	output.color = color;
	return output;
}