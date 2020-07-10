struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
};

sampler2D u_ColorTex ;
#ifdef COPY_DEPTH
sampler2D u_DepthTex ;
#endif

struct PixelShaderOutput
{
    float4 color : COLOR0;
#ifdef COPY_DEPTH
    float fragDepth : DEPTH0;
#endif
};

PixelShaderOutput main(PixelShaderInput input)
{
	PixelShaderOutput output;
    output.color = tex2D(u_ColorTex, input.v_TexCoord0);
    #ifdef LIGHTMAP
    output.color.a = output.color.r;
    #endif
#ifdef COPY_DEPTH
    output.fragDepth = tex2D(u_DepthTex, input.v_TexCoord0).r;
#endif
	return output;
}
