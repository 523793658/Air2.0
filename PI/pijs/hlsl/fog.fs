struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
};
struct PixelShaderOutput
{
    float4 color : COLOR0;
};

sampler2D u_SceneColorTex ;
sampler2D u_SceneDepthTex ;
float4x4 u_ProjMatrixInverse;
float4 u_Params[3];
int4 u_Switchs;

float4 fog(float4 sceneColor, float depth, float4 projPos)
{
    float4 finalColor = sceneColor;
    if(u_Switchs.x == 0)
    {
        return sceneColor;
    }
    if(depth != 1.0)
    {

        float4 tempPos = mul(projPos, u_ProjMatrixInverse);
        float3 viewPos = tempPos.xyz / tempPos.w;
        float d;
        if(u_Switchs.y != 0)
        {
            d = length(viewPos);
        }
        else
        {
            d = -viewPos.z;
        }
        float factor;
        if(u_Switchs.x == 3)
        {
            factor = (u_Params[1].y - d) * u_Params[0].w;
        }
        else if(u_Switchs.x == 2)
        {
            factor = exp(-u_Params[0].w * d);
        }
        else
        {
            factor = exp(-u_Params[0].w * d * d);
        }
        factor = saturate(factor);
        finalColor = float4(lerp(u_Params[0].xyz, sceneColor.xyz, factor), sceneColor.a);
    }
    return finalColor;
}

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    float4 texCoord = float4(input.v_TexCoord0.xy, 0.0, 0.0);
    float4 sceneColor = tex2Dlod(u_SceneColorTex, texCoord);
    float depth = tex2Dlod(u_SceneDepthTex, texCoord);
    float4 projPos = float4((texCoord.x - 0.5) * 2.0, (0.5 - texCoord.y) * 2.0, depth, 1.0);
    sceneColor = fog(sceneColor, depth, projPos);
    output.color = sceneColor;
    return output;
}