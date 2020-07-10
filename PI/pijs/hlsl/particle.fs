struct PixelShaderInput
{
    float4 pos : POSITION;
#ifdef DEPTH
    float2 depth: TEXCOORD4;
#endif
#ifdef TEXTURE
    float2 v_TexCoord0 : TEXCOORD0;
#ifdef TILED
#ifdef BLEND
    float2 v_TexCoord1 : TEXCOORD1;
    float v_Factor : TEXCOORD2;
#endif
#endif
#endif

#ifdef DISTORTION_EFFECT
    float4 v_ProjPos : TEXCOORD3;
#else
    #ifdef DEPTH
        float4 v_ProjPos : TEXCOORD3;
    #endif
#endif

#ifdef TAIL
    float v_TailStrength : TEXCOORD4;
#endif
};

float4 u_Color ;

#ifdef TEXTURE
sampler2D u_Texture ;
#endif

#ifdef DISTORTION_EFFECT
sampler2D u_SceneColorTex ;
#endif

struct PixelShaderOutput
{
    float4 color : COLOR0;
#ifdef DEPTH
    float4 depth : COLOR1;
#endif
};

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
#ifdef TEXTURE
    float4 texel = tex2D(u_Texture, input.v_TexCoord0);
#ifdef TILED
#ifdef BLEND
    float4 blendTex = tex2D(u_Texture, input.v_TexCoord1);
    texel = lerp(texel, blendTex, input.v_Factor);
#endif
#endif
#else
    float4 texel = float4(1.0, 1.0, 1.0, 1.0);
#endif

#ifndef DISTORTION_EFFECT
    float4 finalColor = texel;
    finalColor *= u_Color;
#ifdef TAIL
    finalColor.a *= input.v_TailStrength;
#endif
    clip(finalColor.a - 0.0001);
    output.color = finalColor;
#else
    float2 normal = texel.xy * 2.0 - 1.0;
    if(normal.x == 0.0 && normal.y == 0.0)
    {
        clip(-1);
    }
    float density = 0.05 * u_Color.a;
#ifdef TAIL
    density *= input.v_TailStrength;
#endif
    float2 sceneCoord = input.v_ProjPos.xy / input.v_ProjPos.w * float2(0.5, -0.5) + float2(0.5, 0.5) + normal.xy * density;
    output.color = tex2D(u_SceneColorTex, sceneCoord);
#endif

#ifdef DEPTH
    float depth = input.v_ProjPos.z / input.v_ProjPos.w;
    output.depth = float4(depth, depth,depth, 1.0);
#endif

#ifdef REVERSE
    output.color.rgb = output.color.rgb * output.color.a;
    output.color.a = 1.0 - output.color.a;
#endif

    return output;
}
