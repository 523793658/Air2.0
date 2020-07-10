struct PixelShaderInput
{
    float4 pos : POSITION;
    float3 v_CubeCoord : TEXCOORD0;
};

#ifndef CYLINDER_MAP
samplerCUBE u_EnvMap ;
#else
sampler2D u_EnvMap ;
#endif

struct PixelShaderOutput
{
    float4 color : COLOR0;
#ifdef DEPTH
    float4 depth : COLOR1;
#endif
};

#define PI 3.1415926

float4 textureCylinder(in sampler2D tex, in float3 dir) {

    float2 texCoord;
    texCoord.x = acos(normalize(dir.xz).x) / PI * sign(dir.z);
    texCoord.x = texCoord.x * 0.5 + 0.5;
    texCoord.y = acos(dir.y) / PI;
    //texCoord.y = 1.0 - (dir.y * 0.5 + 0.5);

    float4 color = tex2D(tex, texCoord);

    return color;
}

PixelShaderOutput main(PixelShaderInput input)
{
    float3 dir = normalize(input.v_CubeCoord);

    PixelShaderOutput output;
#ifndef CYLINDER_MAP
    output.color = texCUBE(u_EnvMap, dir);
#else
    output.color = textureCylinder(u_EnvMap, dir);
#endif
    return output;
}