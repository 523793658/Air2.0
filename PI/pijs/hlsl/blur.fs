struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
};

sampler2D u_SrcTex ;
float2 u_Size;

struct PixelShaderOutput
{
    float4 color : COLOR0;
};

PixelShaderOutput main(PixelShaderInput input)
{
	PixelShaderOutput output;
    float4 sum = 0;
    float4 color = tex2D(u_SrcTex, input.v_TexCoord0);
    sum += 0.0947416 * tex2D(u_SrcTex, input.v_TexCoord0 + float2(-1.0, -1.0 ) * u_Size);
    sum += 0.118318 * tex2D(u_SrcTex, input.v_TexCoord0 + float2(0.0, -1.0 ) * u_Size);
    sum += 0.0947416 * tex2D(u_SrcTex, input.v_TexCoord0 + float2(1, -1.0 ) * u_Size);
    sum += 0.118318 * tex2D(u_SrcTex, input.v_TexCoord0 + float2(-1.0, 0.0 ) * u_Size);
    sum += 0.147761 * tex2D(u_SrcTex, input.v_TexCoord0 + float2(0.0, 0.0 ) * u_Size);
    sum += 0.118318 * tex2D(u_SrcTex, input.v_TexCoord0 + float2(1.0, 0.0 ) * u_Size);
    sum += 0.0947416 * tex2D(u_SrcTex, input.v_TexCoord0 + float2(-1.0, 1.0 ) * u_Size);
    sum += 0.118318 * tex2D(u_SrcTex, input.v_TexCoord0 + float2(0.0, 1.0 ) * u_Size);
    sum += 0.0947416 * tex2D(u_SrcTex, input.v_TexCoord0 + float2(1.0, 1.0 ) * u_Size);
    output.color = sum;
	return output;
}
