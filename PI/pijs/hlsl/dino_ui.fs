struct PixelShaderInput
{
    float2 v_TexCoord0 : TEXCOORD0;
    float4 v_ProjPos : POSITION;
#ifdef VERTEX_COLOR
    float4 v_Color : COLOR0;
#endif

#ifdef MASK_MAP
    float2 v_MaskTexCoord : TEXCOORD1;
#endif
    #ifdef TILED
    #ifdef BLEND
    	float3 nextTileInfo : TEXCOORD2;
    #endif
    #endif
};

struct PixelShaderOutput
{
    float4 color : COLOR0;
};
#ifdef BASIC_COLOR
    float4 u_BasicColor ;
#endif

#ifdef OUTLINE
    float4 u_OutlineColor ;
#endif

#ifdef DIFFUSE_MAP
    sampler2D u_DiffuseMap ;
#endif

#ifdef MASK_MAP
    sampler2D u_MaskMap ;
#endif

#ifdef UVANIMATION
    float4 u_UVAnimationSpeed ;
    float u_Time ;
#endif

#ifdef UV_MATRIX
    float4x4 u_uv_Matrix ;
#endif


float2 mod(float2 a, float b)
{
    float2 value;
    value.x = a.x - b * floor(a.x / b );
    value.y = a.y - b * floor(a.y / b );
    return value;
}

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
#ifdef VERTEX_COLOR
    float4 color = input.v_Color;
#else
    float4 color = float4(1.0, 1.0, 1.0, 1.0);
#endif
#ifdef BASIC_COLOR
    color *= u_BasicColor;
#endif
#ifdef MASK_MAP
    float mask = tex2D(u_MaskMap, input.v_MaskTexCoord).a;
    color.a *= mask;
#endif
    float4 texCoord = float4(input.v_TexCoord0, 1.0, 1.0);
    #ifdef UVANIMATION
        texCoord.xy += u_UVAnimationSpeed.xy * u_Time;
        texCoord.xy = mod(texCoord.xy, 1.0);
    #ifdef UV_MATRIX
        texCoord = mul(texCoord, u_uv_Matrix);
    #endif
    #endif
    #ifdef DIFFUSE_MAP

    	float4 diffuseColor = tex2D(u_DiffuseMap, texCoord.xy);


        #ifdef TILED
        #ifdef BLEND
            float4 nextTileColor = tex2D(u_DiffuseMap, input.nextTileInfo.xy);
            diffuseColor = lerp(diffuseColor, nextTileColor, input.nextTileInfo.z);
        #endif
        #endif
        #ifdef INVERT
            color.rgb *= diffuseColor.rgb * color.a;
            color.a = 1.0 - (1.0- diffuseColor.a) * color.a;
            //clip(0.9999 - color.a);
        #else
        #ifdef OUTLINE
            float x = step(diffuseColor.a, 0.5);
            float fontValue = saturate((diffuseColor.a - 0.5) * 2.0 + 0.1 * (1.0 - x));
            //字的颜色
            float3 fontColor1 = fontValue * u_BasicColor.rgb + (1-fontValue) *  float3(0.0, 0.0, 0.0);

            float a = 0;
            float b = 1.0;
            float shadowValue = (clamp(diffuseColor.a * 2.0 * x, a, b) - a) / (b - a);

            color.rgb = saturate(u_OutlineColor * x +  fontColor1 * (1.0 - x));
            color.a = saturate((1.0 - x) + shadowValue * 1.5);
        #else
            color *= diffuseColor;
        #endif
            clip(color.a - 0.00001);
        #endif
    #endif

    #ifdef GRAY
        float valueA = min(color.r, min(color.g, color.b));
        float valueB = max(color.r, max(color.g, color.b));
        valueA = (valueA + valueB) / 2.0;
        color.rgb = float3(valueA, valueA, valueA);
    #endif

	output.color = color;

#ifdef INVERSEDRAW
    output.color.rgb = output.color.rgb * output.color.a;
    output.color.a = 1.0 - output.color.a;
#endif

	return output;
}