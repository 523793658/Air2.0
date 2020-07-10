struct PixelShaderInput
{
    float2 v_TexCoord0 : TEXCOORD0;
};

struct PixelShaderOutput
{
    float4 color : COLOR0;
};

sampler2D u_SrcTex ;
float2 u_SrcSize ;
float3 u_StrokeColor ;


PixelShaderOutput main(PixelShaderInput input)
{

    PixelShaderOutput output;
    output.color = float4(0.0, 0.0, 0.0, 0.0);
    float2 texCoord = input.v_TexCoord0;
    float2 offset = float2(1.0, 1.0) / float2(u_SrcSize.x, u_SrcSize.y);
    float4 color = tex2D(u_SrcTex, texCoord);
    if(color.a > 0.0001){
        output.color = color;
    }else{
        float intensity = 0.0;
        int i;
        int j;
        float4 srcColor;
        float2 coord;

        for(i = -5; i < 5; i+=2)
        {
            for(j = -5; j < 5; j+=2)
            {
                coord = float2(texCoord.x + i*offset.x, texCoord.y + j * offset.y);
                srcColor = tex2D(u_SrcTex, coord);
                if(srcColor.a > 0.0001)
                {
                   intensity += 4.0;
                }
            }
        }
        intensity = intensity / (10 * 10);
        float3 c = float3(0.0, 1.0, 0.0);
        output.color = float4(c * intensity, intensity);
    }
    return output;
}
