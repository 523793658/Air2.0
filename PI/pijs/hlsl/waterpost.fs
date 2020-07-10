struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
#ifdef DEPTH
    float2 v_Depth : TEXCOORD1;
#endif
#ifdef VERTEX_COLOR
    float4 v_Color : COLOR;
#endif
};

struct PixelShaderOutput
{
    float4 color : COLOR0;
    float4 depth : COLOR1;
    float fragDepth : DEPTH0;
};

sampler2D u_WaterColorTex ;
sampler2D u_WaterDepthTex ;

PixelShaderOutput main( PixelShaderInput input )
{
    float4 waterColor = tex2D(u_WaterColorTex, input.v_TexCoord0);
    float waterDepth = tex2D(u_WaterDepthTex, input.v_TexCoord0).x;
    clip(0.9999-waterDepth);
    PixelShaderOutput output;
    output.color = waterColor;
    output.fragDepth = waterDepth;
    output.depth = float4(waterDepth, 1.0, 1.0, 1.0);
    return output;
}