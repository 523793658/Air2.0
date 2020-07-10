struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
};

sampler2D u_ColorTex ;
#ifdef DEPTH
sampler2D u_DepthTex ;
#endif

struct PixelShaderOutput
{
    float4 color : COLOR0;
#ifdef DEPTH
    float fragDepth : DEPTH0;
#endif
};

PixelShaderOutput main(PixelShaderInput input)
{
	PixelShaderOutput output;
    float4 color = tex2D(u_ColorTex, input.v_TexCoord0);
    clip(color.a - 0.001);
    output.color.rgb = color.rgb * color.a;
    output.color.a = 1.0 - color.a;
#ifdef DEPTH
   float depth = tex2D(u_DepthTex, input.v_TexCoord0).r;
   if(depth> 0)
   {
        output.fragDepth = depth;
   }
   else
   {
        output.fragDepth = 1.0;
   }
#endif
	return output;
}
