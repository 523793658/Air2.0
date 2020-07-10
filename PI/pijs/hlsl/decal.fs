struct PixelShaderInput
{
    float2 v_TexCoord0 : TEXCOORD0;
};

struct PixelShaderOutput
{
    float4 color : COLOR0;
};

float g_Time ;
float g_AlphaCullOff ;

float3 u_DiffuseRevise ;
float u_Brightness ;


#ifdef ALPHA_MAP
    sampler2D u_AlphaMap ;
    #ifdef ALPHA_UV_ANIM
        float2 u_AlphaUVVector ;
    #endif
#endif

#ifdef DIFFUSE_MAP
    sampler2D u_DiffuseMap ;
    #ifdef DIFFUSE_UV_ANIM
        float2 u_DiffuseUVVector ;
    #endif
#endif

PixelShaderOutput main(PixelShaderInput input)
{
    #ifdef ALPHA_MAP
        float2 alphaMapTexcoord = input.v_TexCoord0;
        #ifdef ALPHA_UV_ANIM
            alphaMapTexcoord += u_AlphaUVVector * g_Time;
        #endif
        float alpha = tex2D(u_AlphaMap, alphaMapTexcoord).a;
        clip(alpha - g_AlphaCullOff);
    #else
        float alpha = 1.0;
    #endif
    PixelShaderOutput output;

    #ifdef DIFFUSE_MAP
        float2 diffuseTexcoord = input.v_TexCoord0;
        #ifdef DIFFUSE_UV_ANIM
            diffuseTexcoord += u_DiffuseUVVector * g_Time;
        #endif
        float4 diffuse = tex2D(u_DiffuseMap, diffuseTexcoord);
    #else
        float4 diffuse = float4(1.0, 1.0, 1.0, 1.0);
    #endif

    alpha = min(alpha, diffuse.a);

    #ifdef SHADOW
        output.color = float4( float3(0.0, 0.0, 0.0), pow(1.0 - alpha, u_Brightness));
    #else
        output.color = float4(diffuse.rgb * u_DiffuseRevise * u_Brightness, alpha);
    #endif
    return output;
}
