#define PI 3.1415926
#define PIInv 0.31830988


float4 textureCylinder(sampler2D tex, float3 dir) {
    float4 texCoord;
    dir.x += 0.0001;
    texCoord.x = acos(normalize(dir.xz).x) * PIInv * sign(dir.z);
    texCoord.x = texCoord.x * 0.5 + 0.5;
    texCoord.y = acos(dir.y) * PIInv;
    texCoord.z = 0.0;
    texCoord.w = 0.0;
    float4 color = tex2Dlod(tex, texCoord);
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

float g_Time ;
float3 g_ViewPosition ;

sampler2D u_SceneColorTex ;
sampler2D u_SceneDepthTex ;
sampler2D u_NormalMap ;

float3 u_LightDir  ;


float3 u_LightColor  ;


float4 u_Params[5];


float4x4 u_ViewProjMatrixInverse ;

#ifdef ENVIRONMENT_MAPPING
    #ifdef CYLINDER_MAPPING
        sampler2D u_EnvironmentMap ;
    #else
        samplerCUBE u_EnvironmentMap ;
    #endif
#endif

#ifdef REFLECTION
    sampler2D u_ReflectionTex ;
    float4x4 u_ReflectionMatrix ;
#endif

#ifdef CAUSTICS
    sampler2D u_CausticsMap ;
#endif

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
    normal.xz *= 1.0 / 2.0;
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

    float decay = max(pow(max(1.0 + input.v_ViewPos.z / 1000.0, 0.0), 4.0), 0.1);
    float3 decayWorldNormal = worldNormal;
    decayWorldNormal.xz *= decay;
    decayWorldNormal = normalize(decayWorldNormal);

    float3 lightDir = normalize(u_LightDir);

    float3 worldViewDir = normalize(input.v_WorldPos.xyz - g_ViewPosition);

    float viewAngleIn = min(acos(dot(-worldViewDir, float3(-decayWorldNormal.x, decayWorldNormal.y, -decayWorldNormal.z))), 3.1415926 / 2.0 * 0.99);
    float3 viewFR;
    viewFR = getFresnelReflectance(viewAngleIn, 1.000293, u_Params[3].z);

    float4 sceneColor = tex2D(u_SceneColorTex, sceneCoord);
    float4 originalSceneColor = sceneColor;

    float4 worldScenePos = mul(float4(input.v_ProjPos.xy / input.v_ProjPos.w, srcSceneDepth, 1.0), u_ViewProjMatrixInverse );
    worldScenePos /= worldScenePos.w;

    float fogDistence =  abs(distance(input.v_WorldPos.xyz, worldScenePos.xyz));
    float fogFactor = 1.0 - clamp(exp(-fogDistence / 300.0 * u_Params[0].w), 0.0, 1.0);

    float3 causticsColor = float3(0.75, 0.75, 0.75);

    #ifdef CAUSTICS
        if(fogFactor < 0.95)
        {
            float3 lightProjCoord = worldScenePos.xyz / 0.5;
            float3 causticsNormal = (getNormal(u_NormalMap, float4(lightProjCoord, 1.0), time * 1.2) + 1.0) / 2.0;
            float2 causticsCoord = causticsNormal.xz;
            float causticsFactor = tex2D(u_CausticsMap, causticsCoord).a;
            causticsFactor = pow(1.0 - causticsFactor, 8.0) * 200.0 * u_Params[3].w * (1.0 - fogFactor) - pow(causticsFactor, 2.0) * 0.7;
            causticsColor = float3(0.9, 0.9, 0.9) + causticsFactor;
        }
    #endif

    sceneColor.xyz *= causticsColor;
    sceneColor.xyz = u_Params[0].xyz * fogFactor + sceneColor.xyz *(1.0 - fogFactor);
    float3 refraction = sceneColor.xyz * (1.0 - viewFR.z);

    #ifdef REFLECTION
        float4 reflectionCoord = mul(input.v_WorldPos, u_ReflectionMatrix );
        reflectionCoord.xy /= reflectionCoord.w;
        reflectionCoord.xy = (reflectionCoord.xy + 1.0) * 0.5 + worldNormal.xz * 0.04;
        reflectionCoord.y = 1.0 - reflectionCoord.y;
        float4 reflectionColor = tex2D(u_ReflectionTex, reflectionCoord.xy);
        reflectionColor.xyz *= u_Params[4].x;
    #else
        float4 reflectionColor = float4(0.0, 0.0, 0.0, 0.0);
    #endif

    float3 specular = (u_Params[1].w + 8.0) / (8.0 * PI) * u_LightColor * pow(max(dot(normalize(-worldViewDir + lightDir), decayWorldNormal), 0.0), u_Params[1].w) * (1.0 - reflectionColor.a);

    #ifdef ENVIRONMENT_MAPPING
        float3 refNor = decayWorldNormal;
        refNor.xz *= sign(dot(-worldViewDir, decayWorldNormal));
        #ifdef CYLINDER_MAPPING
            float3 skyColor = textureCylinder(u_EnvironmentMap, refNor).xyz;
        #else
            float3 skyColor = texCUBE(u_EnvironmentMap, refNor).xyz;
        #endif
        skyColor.xyz = sRGBToRGB(skyColor.xyz);
    #else
        float3 skyColor = u_Params[1].xyz;
    #endif

     float3 reflection = (lerp(skyColor, reflectionColor.xyz, reflectionColor.a) + specular) * viewFR.z;

    decayWorldNormal.xz *= 4.0;
    decayWorldNormal = normalize(decayWorldNormal);

    float4 finalColor = float4(refraction + reflection, 1.0);
    float gradFactor = clamp((srcSceneDepth - depth) * 1000.0, 0.0, 1.0);
    output.color = finalColor;
#ifdef DEPTH
    output.depth = float4(depth, depth, depth, 1.0);
#endif
    return output;
} 