struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
};

#ifdef TEXTURE2
sampler2D u_SrcTex2 ;
#endif
sampler2D u_SrcTex ;
float2 u_Size;

struct PixelShaderOutput
{
    float4 color : COLOR0;
};

PixelShaderOutput main(PixelShaderInput input)
{
	PixelShaderOutput output;
    float4 color = tex2D(u_SrcTex, input.v_TexCoord0);
    #ifdef TEXTURE2
    float4 color2 = tex2D(u_SrcTex2, input.v_TexCoord0);
    #endif
    if(color.a < 0.8)
    {
        color = float4(0.0, 0.0, 1.0, 1.0);
        float v = 0.0;
        for(int i = -2 ; i < 3; i++)
        {
            for(int j = -2 ;j < 3; j++)
            {
                float4 value = tex2D(u_SrcTex, input.v_TexCoord0 + float2(u_Size.x * i, u_Size.y * j));
                if(value.a > 0.9)
                {
                    if(value.a > v)
                    {
                        color.rgb = value.rgb;
                        #ifdef TEXTURE2
                            float4 value2 = tex2D(u_SrcTex2, input.v_TexCoord0 + float2(u_Size.x * i, u_Size.y * j));
                            color.a = value2.r;
                        #else
                            color.a = 1.0;
                        #endif
                        v = value.a;
                    }
                }
            }
        }
    }
#ifdef TEXTURE2
    else
    {
        color.a = color2.r;
    }
#endif


    output.color = color;
	return output;
}
