struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
};

sampler2D u_SrcTex ;
sampler2D u_AccTex ;
float4 u_LuminanceParams;

struct PixelShaderOutput
{
    float4 color : COLOR0;
};

PixelShaderOutput main(PixelShaderInput input)
{
    float4 t = float4(input.v_TexCoord0 + 0.5, 0.0, 10.0);
    float src = clamp(tex2Dlod(u_SrcTex, t).r, 0.0, 5.0);
    float acc = tex2D(u_AccTex, float2(1.0, 1.0)).r;
    PixelShaderOutput output;
    output.color = float4( max(acc + (src - acc) * u_LuminanceParams.x, u_LuminanceParams.y), 0.0, 0.0, 1.0 );
    return output;
}
