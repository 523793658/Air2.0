#define PI 3.1415926
float4 textureCylinder(sampler2D tex, float3 dir) {
	float2 texCoord;
	texCoord.x = acos(normalize(dir.xz).x) / PI * sign(dir.z);
	texCoord.x = texCoord.x * 0.5 + 0.5;
	texCoord.y = acos(dir.y) / PI;
	float4 color = tex2D(tex, texCoord);
	return color;
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


 float g_Time ;
 float3 g_ViewPosition ;

 sampler2D u_SceneDepthTex ;
 sampler2D u_NormalMap  ;

#ifdef ENVIRONMENT_MAPPING
    #ifdef CYLINDER_MAPPING
         sampler2D u_EnvironmentMap ;
    #else
         samplerCUBE u_EnvironmentMap ;
    #endif
#endif

float4 u_Params[4];

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

 float3 u_LightDir ;


 float3 u_LightColor ;

float3 getNormal(in sampler2D normalMap, in float4 worldPos, float time) {
    float2 srcCoord = worldPos.xz / 40.0;
    srcCoord.y = -srcCoord.y;

    float2 srcDir = normalize(float2(-1.0, 1.0));
    float2 dstDir = normalize(-u_Params[2].yx);
    float a = srcDir.x;
    float b = srcDir.y;
    float c = dstDir.x;
    float d = dstDir.y;
    float divisor = (a * a + b * b);
    float sinA = (a * d - b * c) / divisor;
    float cosA = (a * c + b * d) / divisor;
    float2x2 coordMat = float2x2(cosA, -sinA, sinA, cosA);
    srcCoord = mul(srcCoord, coordMat );

    float2 originalDir = normalize(float2(1.0, 1.0));

    #ifdef FLOWING
        float2 animCoord0 = srcCoord * 0.6 + (originalDir + float2(0.0, 0.3)) * time / 2.0 + float2(cos(time * 2.0), sin(time * 2.0)) * sin(srcCoord.x * 1.0 + srcCoord.y * 2.0) * 0.1;
        float2 animCoord1 = srcCoord * 1.2 + (originalDir + float2(3.0, 0.0)) * time / 8.0 + float2(cos(time * 2.0), sin(time * 2.0)) * sin(srcCoord.x * 3.0 + srcCoord.y * 2.0) * 0.1;
    #else        
        float2 animCoord0 = srcCoord * 0.6 + (-originalDir) * time / 16.0 + (float2(1, cos(time * 2.0)) * 0.1 + 0.2) * sin(srcCoord.x * 1.0 + srcCoord.y * 2.0) * 0.1;
        float2 animCoord1 = srcCoord * 1.2 + (originalDir) * time / 16.0 + (float2(1, sin(time * 2.0)) * 0.1 + 0.2) * sin(srcCoord.x * 3.0 + srcCoord.y * 2.0) * 0.1;
    #endif

    float3 normal = tex2D(u_NormalMap, animCoord0 / u_Params[2].w).xyz;
    normal += tex2D(u_NormalMap, animCoord1 / u_Params[2].w).xyz;
    normal /= 2.0;
    normal = float3(normal.r, normal.b, normal.g);
    normal = (normal - 0.5) * 2.0;
    normal.xz *= 1.0 / 2.0;
    normal = normalize(normal);

    return normal;
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
    worldNormal.xz *= u_Params[3].x;
    worldNormal = normalize(worldNormal);

    float decay = max(pow(max(1.0 + input.v_ViewPos.z / 1000.0, 0.0), 4.0), 0.1);
    float3 decayWorldNormal = worldNormal;
    decayWorldNormal.xz *= decay;
    decayWorldNormal = normalize(decayWorldNormal);

    float3 lightDir = normalize(u_LightDir);

    float3 worldViewDir = normalize(input.v_WorldPos.xyz - g_ViewPosition);

    float viewAngleIn = min(acos(dot(-worldViewDir, float3(-decayWorldNormal.x, decayWorldNormal.y, -decayWorldNormal.z))), 3.1415926 / 2.0);
    float3 viewFR;
    viewFR.xyz = getFresnelReflectance(viewAngleIn, 1.000293, u_Params[3].z);

    float3 specular = 8.0 * u_LightColor * pow(max(dot(normalize(-worldViewDir + lightDir), decayWorldNormal), 0.0), u_Params[1].w);

    #ifdef ENVIRONMENT_MAPPING
        float3 refNor = decayWorldNormal;
        refNor.xz *= sign(dot(-worldViewDir, decayWorldNormal));
        #ifdef CYLINDER_MAPPING
            float3 skyColor = textureCylinder(u_EnvironmentMap, refNor).xyz;
        #else
            float3 skyColor = texCUBE(u_EnvironmentMap, refNor).xyz;
        #endif
    #else
        float3 skyColor = float3(0.44, 0.55, 0.6);
    #endif
    float3 reflection = (skyColor / 1.5 + specular) * min(viewFR.z, 0.2);

    decayWorldNormal.xz *= 4.0;
    decayWorldNormal = normalize(decayWorldNormal);
    float3 diffSpecular =  u_LightColor * pow(max(dot(normalize(-worldViewDir + lightDir), decayWorldNormal), 0.0), u_Params[1].w / 4.0);
    float3 refraction = (u_LightColor * u_Params[0].xyz * max(dot(lightDir, decayWorldNormal), 0.0) + diffSpecular * 0.3 + u_Params[0].xyz * 0.5) * max((1.0 - viewFR.z), 0.5);

    float4 finalColor = float4(refraction + reflection, 1.0);
    float gradFactor = clamp((srcSceneDepth - depth) * 1000.0, 0.0, 1.0);
    finalColor.a = max(gradFactor, (specular.r + specular.g + specular.b) / 3.0);
    output.color = finalColor;
#ifdef DEPTH
    output.depth = float4(depth, 1.0, 1.0, 1.0);
#endif
    return output;
}