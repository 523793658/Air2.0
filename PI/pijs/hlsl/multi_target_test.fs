struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
};

sampler2D u_RedTex ;
sampler2D u_GreenTex ;
sampler2D u_BlueTex ;
sampler2D u_AlphaTex ;

struct PixelShaderOutput
{
    float4 color : COLOR0;
};

PixelShaderOutput main(PixelShaderInput input)
{
    float4 color;
    color.r = tex2D(u_RedTex, input.v_TexCoord0).r;
    color.g = tex2D(u_GreenTex, input.v_TexCoord0).g;
    color.b = tex2D(u_BlueTex, input.v_TexCoord0).b;
    color.a = tex2D(u_AlphaTex, input.v_TexCoord0).a;
    
    PixelShaderOutput output;
    output.color = color;
    return output;
}
