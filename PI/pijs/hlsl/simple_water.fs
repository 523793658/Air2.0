#define PI 3.1415926

float4 textureCylinder(sampler2D tex, float3 dir) {
    float2 texCoord;
    texCoord.x = acos(normalize(dir.xz).x) / PI * sign(dir.z);
    texCoord.x = texCoord.x * 0.5 + 0.5;
    texCoord.y = acos(dir.y) / PI;
    float4 color = tex2D(tex, texCoord);
    return color;
}

float3 RGBTosRGB(float3 color) {
	return pow(color, float3(0.454545, 0.454545, 0.454545));
}

float3 sRGBToRGB(float3 color) {
	return pow(color, float3(2.2, 2.2, 2.2));
}

float4 sRGBTexture2D(sampler2D tex, float2 texCoord) {
    float4 color = tex2D(tex, texCoord);
    color.xyz = pow(color.xyz, float3(2.2, 2.2, 2.2));

    return color;
}


float3 getFresnelReflectance(in float angleIn, in float n1, in float n2) {
    float sinIn = sin(angleIn);
    float cosIn = cos(angleIn);
    float rFactor = n1 / n2 * sinIn;
    rFactor = sqrt(max(1.0 - rFactor * rFactor, 0.0));

    float rs = (n1 * cosIn - n2 * rFactor) / (n1 * cosIn + n2 * rFactor);
    float rp = (n1 * rFactor - n2 * cosIn) / (n1 * rFactor + n2 * cosIn);
    rs *= rs;
    rp *= rp;

    float3 rf = float3(rs, rp, (rs + rp) / 2.0);

    return rf;
}

struct PixelShaderInput
{
    float4 v_WorldPos : TEXCOORD0;
    float4 v_ViewPos : TEXCOORD1;
    float4 v_ProjPos : TEXCOORD2;
};
struct PixelShaderOutput
{
    float4 color : COLOR0;
#ifdef DEPTH
    float4 depth : COLOR1;
#endif
};

float4 u_Params[5];

//float3 u_FogColor ;
//float u_FogDensity;

//float3 u_SkyColor;
// float u_Shininess;

//float2 u_WaveDir;
//float u_WaveSpeed;
//float u_WaveScale;

//float u_waveDensity;
//float u_RefractiveDensity;
//float u_RefractiveIndex;
//float u_CausticsDensity;


float g_Time ;
float3 g_ViewPosition ;

sampler2D u_SceneColorTex ;
sampler2D u_SceneDepthTex ;
sampler2D u_NormalMap ;

float3 u_LightDir  ;


float3 u_LightColor  ;
float4x4 u_ViewProjMatrixInverse ;

float3 getNormal(in sampler2D normalMap, in float4 worldPos, float time) {
    float2 srcCoord = worldPos.xz / 40.0;
    srcCoord.y = -srcCoord.y;
    float2 originalDir = normalize(float2(1.0, 1.0));

    #ifdef FLOWING
        float2 animCoord0 = srcCoord * 0.6  + (originalDir + float2(0.0, 0.3)) * time / 2.0 + float2(cos(time * 2.0), sin(time * 2.0)) * sin(srcCoord.x * 1.0 + srcCoord.y * 2.0) * 0.1;
        float2 animCoord1 = srcCoord * 1.2  + (originalDir + float2(3.0, 0.0)) * time / 8.0 + float2(cos(time * 2.0), sin(time * 2.0)) * sin(srcCoord.x * 3.0 + srcCoord.y * 2.0) * 0.1;
    #else
        float2 animCoord0 = srcCoord * 0.6  + (-originalDir) * time / 4.0 + (float2(1, cos(time * 2.0)) * 0.1 + 0.2) * sin(srcCoord.x * 1.0 + srcCoord.y * 2.0) * 0.1;
        float2 animCoord1 = srcCoord * 1.2  + (originalDir) * time / 4.0 + (float2(1, sin(time * 2.0)) * 0.1 + 0.2) * sin(srcCoord.x * 3.0 + srcCoord.y * 2.0) * 0.1;
    #endif
    float3 normal = tex2D(u_NormalMap, animCoord0 / u_Params[2].w).xyz;
    normal += tex2D(u_NormalMap, animCoord1 / u_Params[2].w).xyz;
    normal /= 2.0;
    normal = float3(normal.r, normal.b, normal.g);
    normal = (normal - 0.5) * 2.0;
    normal.xz *= 0.5;
    normal = normalize(normal);

    return normal;
}

PixelShaderOutput main(PixelShaderInput input)
{
    float2 sceneCoord = (input.v_ProjPos.xy / input.v_ProjPos.w + 1.0) * 0.5;
    sceneCoord.y = 1.0 - sceneCoord.y;
    float srcSceneDepth = tex2D(u_SceneDepthTex, sceneCoord).x;
    float depth = input.v_ProjPos.z / input.v_ProjPos.w;
    clip(srcSceneDepth - depth);
    PixelShaderOutput output;

    float time = g_Time * u_Params[2].z;

    float3 worldNormal = getNormal(u_NormalMap, input.v_WorldPos, time);

    sceneCoord += worldNormal.xz * 0.005 * u_Params[3].y * max((1.0 - min(input.v_ProjPos.w * 0.005, 1.0)) * 3.0, 0.3);

    worldNormal.xz *= u_Params[3].x;
    worldNormal = normalize(worldNormal);

    float3 lightDir = normalize(u_LightDir);

    float3 worldViewDir = normalize(input.v_WorldPos.xyz - g_ViewPosition);

    float viewAngleIn = min(acos(dot(worldViewDir, float3(worldNormal.x, -worldNormal.y, worldNormal.z))), 1.555);
    float3 viewFR;
    viewFR = getFresnelReflectance(viewAngleIn, 1.000293, u_Params[3].z);

    float4 sceneColor = sRGBTexture2D(u_SceneColorTex, sceneCoord);
    float4 originalSceneColor = sceneColor;

    float4 worldScenePos = mul(float4(input.v_ProjPos.xy / input.v_ProjPos.w, srcSceneDepth, 1.0), u_ViewProjMatrixInverse );
    worldScenePos /= worldScenePos.w;

    float fogDistence =  distance(worldScenePos.xyz, input.v_WorldPos.xyz);
    float fogFactor = 1.0 - clamp(exp(-fogDistence / 300.0 * u_Params[0].w), 0.0, 1.0);

    sceneColor.xyz = u_Params[0].xyz * fogFactor + sceneColor.xyz *(1.0 - fogFactor);
    float3 refraction = sceneColor.xyz * (1.0 - viewFR.z);
    float3 specular = (u_Params[1].w + 8.0) / (8.0 * PI) * u_LightColor * pow(max(dot(normalize(-worldViewDir + lightDir), worldNormal), 0.0), u_Params[1].w);
    float3 reflection = (u_Params[1].xyz + specular) * viewFR.z;

    worldNormal.xz *= 4.0;
    worldNormal = normalize(worldNormal);

    float4 finalColor = float4(refraction + reflection, 1.0);
    float gradFactor = clamp((srcSceneDepth - depth) * 1000.0, 0.0, 1.0);
    finalColor = finalColor * gradFactor + originalSceneColor * (1.0 - gradFactor);



    output.color = finalColor;
    output.color.xyz = RGBTosRGB(output.color.xyz);
    #ifdef DEPTH
    output.depth = float4(depth, depth, depth, 1.0);
    #endif
    return output;
}