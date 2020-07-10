struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
};

struct PixelShaderOutput
{
    float4 color1 : COLOR0;
    float4 color2 : COLOR1;
    float4 color3 : COLOR2;
    float4 color4 : COLOR3;
};

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;

    output.color1 = float4(1.0, 0.0, 0.0, 0.0);
    output.color2 = float4(0.0, 0.2, 0.0, 0.0);
    output.color3 = float4(0.0, 0.0, 0.5, 0.0);
    output.color4 = float4(0.0, 0.0, 0.0, 1.0);

    return output;
}
