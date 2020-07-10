struct PixelShaderInput
{
    float4 pos : POSITION;
	float2 v_TexCoord0 : TEXCOORD0;
	float3 v_VertexNormal : TEXCOORD3;

#ifdef DEPTH
    float2 v_Depth : TEXCOORD1;
#endif
#ifdef VERTEX_COLOR
    float4 v_Color : COLOR0;
#endif
};

#ifdef COLOR
float4 u_Color ;
#else
sampler2D u_DiffuseMap ;
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
#ifdef COLOR
    float4 color = u_Color;
#else
    float4 color = tex2D(u_DiffuseMap, input.v_TexCoord0);
#endif

#ifdef VERTEX_COLOR
    color = color * input.v_Color;
#endif


    PixelShaderOutput output;
	float3 diffuseColor;
    output.color = color;
    output.color.rgb += saturate(dot(normalize(input.v_VertexNormal), float3(0.5, 1.0, 1.0))) * float3(0.4, 0.4, 0.4);
	//output.color.rgb = input.v_VertexNormal;
#ifdef DEPTH
    float depth = input.v_Depth.x / input.v_Depth.y;
    output.depth = float4(depth, depth, depth, depth);
#endif
    return output;
}
