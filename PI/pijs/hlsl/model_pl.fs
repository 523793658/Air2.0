#define PI 3.1415926
#define PIInv 0.31830988
#define PIx8Inv 0.039788735
float4 getDecal(in sampler2D decalMap, in float2 decalCoord, in float vDepth, in float zFar)
{
	float2 coord =  decalCoord * float2(0.5, -0.5 ) + 0.5;
    float4 color = tex2D(decalMap, coord);
    float fade = max( clamp( ( 1.0 - length(decalCoord)) * 5.0, 0.0, 1.0 ), 1.0 - clamp(-vDepth - zFar, 0.0, 1.0) );

    return float4( float3(color.rgb * fade), 1.0 - fade + color.a * fade );
}
float offset_lookup(sampler2D map, float4 loc, float2 offset, float sizeInv)
 {
     return tex2Dproj(map, float4(loc.xy + offset * sizeInv * loc.w, loc.z - 0.00050, loc.w)).r;
 }

float getShadow(in sampler2D shadowMap, in float4 shadowCoord, in float zFar, float sizeInv)
{
	float4 coord = float4(0.5 * shadowCoord.xy + float2(0.5, 0.5), shadowCoord.z, shadowCoord.w);

	coord.y = 1.0  - coord.y;

	float shadow =(
	offset_lookup(shadowMap, coord, float2(-0.75, 0.75), sizeInv).r +
	offset_lookup(shadowMap, coord, float2(0.75, 0.75), sizeInv).r +
	offset_lookup(shadowMap, coord, float2(-0.75, -0.75), sizeInv).r +
	offset_lookup(shadowMap, coord, float2(0.75, -0.75), sizeInv).r
	) * 0.25;
    return shadow;
}

float4 sRGBTexture2D(sampler2D tex, float2 texCoord) {
    float4 color = tex2D(tex, texCoord);
    color.xyz = pow(color.xyz, float3(2.2, 2.2, 2.2));

    return color;
}

float4 textureCylinderLod(sampler2D tex, float4 dir) {

	float4 texCoord;
	texCoord.x = acos(normalize(dir.xz).x) * PIInv * sign(dir.z);
	texCoord.x = texCoord.x * 0.5 + 0.5;
	texCoord.y = acos(dir.y) * PIInv;
	texCoord.z = 0.0;
	texCoord.w = dir.w;
	float4 color = tex2Dlod(tex, texCoord);
	return color;
}


float getLuminance(float3 rgbColor) {
	return rgbColor.r * 0.299 + rgbColor.g * 0.587 + rgbColor.b * 0.114;
}

float3 getFresnelReflectance(float cosIn, float x1, float x2) {
	float sinIn = sqrt(1 - cosIn * cosIn);
	float rFactor = x1 / x2 * sinIn;
	rFactor = sqrt(max(1.0 - rFactor * rFactor, 0.0));
	float rs = (x1 * cosIn - x2 * rFactor) / (x1 * cosIn + x2 * rFactor);
	float rp = (x1 * rFactor - x2 * cosIn) / (x1 * rFactor + x2 * cosIn);
	rs *= rs;
	rp *= rp;
	float3 rf = float3(rs, rp, (rs + rp) * 0.5);
	return rf;
}

float3 sRGBToRGB(float3 color) {
	return pow(color, float3(2.2, 2.2, 2.2));
}

float3 RGBTosRGB(float3 color) {
	return pow(color, float3(0.454545, 0.454545, 0.454545));
}

struct PixelShaderInput
{
	float4 pos : vPos;
	// DEPTH 对应后两个成员
	float4 v_TexCoord0 : TEXCOORD0;

	float3 v_WorldPos : TEXCOORD1;
	float4 v_WorldViewDir : TEXCOORD2;
	float3 v_VertexNormal : TEXCOORD3;
#ifdef NORMAL_MAP
	float3 v_VertexTangent : TEXCOORD4;
	float3 v_VertexBinormal : TEXCOORD5;
#endif
    float4 v_ProjPos : TEXCOORD6;


#ifdef SHADOW
	float4 v_ShadowProjCoord : TEXCOORD7;
#endif

#ifdef LIGHTMAP
    float4 v_TexCoord1 : COLOR1;
#else
    #ifdef DECAL
    #ifndef DECAL_IGNORE
        float4 v_TexCoord1 : COLOR1;
    #endif
    #endif
#endif


#ifdef VERTEX_COLOR
	float4 v_VertexColor : COLOR0;
#endif
	float frontFace : VFACE;
};

struct PixelShaderOutput
{
	float4 color : COLOR0;
#ifdef DEPTH
	float4 depth : COLOR1;
#endif
};

#define MAX_MULTI_LIGHTS 10

#define n1 1.000293

//全局变量声明，使用固定register index。保证每帧更新一次
struct DefaultLight
{
    float4 ambientColor;
    float4 diffuseColor;
    float4 diffuseDir;
};

struct ExtraLight
{
    float4 ambientColor;
    float4 diffuseColor1;
    float4 diffuseDir1;
    float4 diffuseColor2;
    float4 diffuseDir2;
};

#ifdef EXTRA_LIGHTING
    ExtraLight u_ExtraLightData;

    #define u_DirLightDir           u_ExtraLightData.diffuseDir1.xyz
    #define u_DirLightColor         u_ExtraLightData.diffuseColor1.xyz
    #define u_AmbientColor          u_ExtraLightData.ambientColor.xyz
    #define u_ExtraLightColor       u_ExtraLightData.diffuseColor2.xyz
    #define u_ExtraLightDir         u_ExtraLightData.diffuseDir2.xyz
    #define u_ShadowDensity         u_ExtraLightData.ambientColor.w
    #define u_ShadowZFar            u_ExtraLightData.diffuseColor1.w
    #define u_ShadowMapSizeInv      u_ExtraLightData.diffuseDir1.w
#else
    DefaultLight u_DefaultLightData;
    #define u_DirLightDir           u_DefaultLightData.diffuseDir.xyz
    #define u_DirLightColor         u_DefaultLightData.diffuseColor.xyz
    #define u_AmbientColor          u_DefaultLightData.ambientColor.xyz
    #define u_ShadowDensity         u_DefaultLightData.ambientColor.w
    #define u_ShadowZFar            u_DefaultLightData.diffuseColor.w
    #define u_ShadowMapSizeInv        u_DefaultLightData.diffuseDir.w
#endif


float g_Time;

#ifdef SHADOW
sampler2D g_ShadowMap;
#endif

float3 u_DiffuseRevise;
float3 u_SpecularRevise;


#ifdef NORMAL_MAP
sampler2D u_NormalMap;
#endif



#ifdef ALPHA_MAP
sampler2D u_AlphaMap;
float g_AlphaCullOff;
#ifdef ALPHA_UV_ANIM
float2 u_AlphaUVVector;
#endif
#endif


#ifdef DIFFUSE_UV_ANIM

float2 u_DiffuseUVVector;

#endif

#ifdef DIFFUSE_MAP
sampler2D u_DiffuseMap;
#endif

#ifdef SPECULAR_MAP
sampler2D u_SpecularMap;
#endif

#ifdef GLOW_MAP

sampler2D u_GlowMap;
float u_GlowScale;


#ifdef GLOW_UV_ANIM

float2 u_GlowUVVector;

#endif

#ifdef GLOW_ALPHA_MAP
sampler2D u_GlowAlphaMap;
#endif
#endif

#ifdef GLOSSINESS_MAP
sampler2D u_GlossinessMap;
#else
float u_Glossiness;
#endif

#ifdef REFLECTION_MAP
sampler2D u_ReflectionMap;
#else
float u_Reflection;
#endif

#ifdef MULTI_LIGHTING
int u_PointLightNum;
float4 u_PointLightColorRadiusArray[MAX_MULTI_LIGHTS];
float4 u_WorldPointLightPosDecayArray[MAX_MULTI_LIGHTS];
#endif


#ifdef ENVIRONMENT_MAPPING
#ifdef CYLINDER_MAPPING

sampler2D u_EnvironmentMap;

#else

samplerCUBE u_EnvironmentMap;

#endif
#endif


#ifdef MTRL_SHADOW
float u_MtrlShadow;
#endif

#ifdef MTRL_ALPHA
float u_MtrlAlpha;
#endif

#ifdef LIGHTMAP

sampler2D u_AOMap;
float4 u_LightMapOS;

#ifdef LIGHTMAP_LIGHT

sampler2D u_LightMap;
float4 u_LightMapOS2;

#endif
#endif


#ifdef DECAL
#ifndef DECAL_IGNORE

sampler2D u_DecalMap;
float4 u_DecalZFar;

#endif
#endif


#ifdef REFRACTION
float4x3 g_ViewMatrix;
sampler2D u_RefractionMap;

#ifdef UNIFORM_REFRACTION_TRANSPARENT
float u_RefractionTransparent;
#endif
#endif



#ifdef VANISH
float u_VanishSpeed;
float3  u_VanishColor ;
float u_VanishGap ;
float u_VanishPower ;
sampler2D u_VanishMap ;
#endif




#ifdef FLOOD2
	float3 u_Flood2Color ;
	float4 u_Flood2 ;
#ifdef FLOOD_MAP
	sampler2D u_FloodMap2 ;
#endif
#else
#ifdef FLOOD
float u_FloodScale ;
float3 u_FloodColor ;
#ifdef FLOOD_TEX
float2 u_FloodUVVector ;
sampler2D u_FloodMap ;
#endif
#endif
#endif

#ifdef FADE
float u_FadeProcess ;
#endif



PixelShaderOutput main(PixelShaderInput input)
{





	PixelShaderOutput output;
#ifdef VANISH
	float vanishAlpha = tex2D(u_VanishMap, input.v_TexCoord0.xy).a;
	clip(vanishAlpha - u_VanishSpeed + u_VanishGap);
#endif

	float alpha = 1.0;
#ifdef ALPHA_MAP

float2 alphaMapTexcoord = input.v_TexCoord0.xy;

#ifdef ALPHA_UV_ANIM
	alphaMapTexcoord += u_AlphaUVVector * g_Time;
#endif

	alpha = tex2D(u_AlphaMap, alphaMapTexcoord).a;
    clip(alpha - g_AlphaCullOff - 0.000001);
#endif


#ifdef TERRAIN
    float2 diffuseTexcoord = input.v_ProjPos.zw;
#else
    float2 diffuseTexcoord = input.v_TexCoord0.xy;
#endif

#ifdef DIFFUSE_UV_ANIM
	diffuseTexcoord += u_DiffuseUVVector * g_Time;
#endif

	float4 diffuseColor = float4(1.0, 1.0, 1.0, 1.0);
#ifdef DIFFUSE_MAP
	diffuseColor = sRGBTexture2D(u_DiffuseMap, diffuseTexcoord);
#endif
#ifdef VERTEX_COLOR
	diffuseColor *= input.v_VertexColor;
#endif
	diffuseColor.xyz *= u_DiffuseRevise;

	alpha *= diffuseColor.a;

	float3 specularColor = float3(1.0, 1.0, 1.0);
#ifdef SPECULAR_MAP
	specularColor = sRGBTexture2D(u_SpecularMap, diffuseTexcoord).xyz;
#endif
	specularColor *= u_SpecularRevise;

	float3 glowColor = float3(0.0, 0.0, 0.0);
#ifdef GLOW_MAP

float2 glowMapTexcoord = input.v_TexCoord0.xy;
#ifdef GLOW_UV_ANIM
	glowMapTexcoord += u_GlowUVVector * g_Time;
#endif
#ifdef GLOW_ALPHA_MAP
	glowColor = sRGBTexture2D(u_GlowMap, glowMapTexcoord).xyz * u_GlowScale * tex2D(u_GlowAlphaMap, diffuseTexcoord).a;
#else
	glowColor = sRGBTexture2D(u_GlowMap, glowMapTexcoord).xyz * u_GlowScale * diffuseColor.a;
#endif
#endif

#ifdef GLOSSINESS_MAP
	float glossiness = tex2D(u_GlossinessMap, diffuseTexcoord).a;
#else
	float glossiness = u_Glossiness;
#endif

	float shadow = 1.0;

#ifdef MTRL_SHADOW
    float shadowIntensity = u_MtrlShadow * u_ShadowDensity;
#else
    #define shadowIntensity u_ShadowDensity
#endif

#ifdef LIGHTMAP
    float2 shadow_texCoord =  input.v_TexCoord1.xy * u_LightMapOS.zw + u_LightMapOS.xy;
    shadow_texCoord.y = 1.0 - shadow_texCoord.y;
    shadow = tex2D(u_AOMap, shadow_texCoord).a;
    #ifdef LIGHTMAP_LIGHT
        float2 light_texCoord = input.v_TexCoord1.xy * u_LightMapOS2.zw + u_LightMapOS2.xy;
        light_texCoord.y = 1.0 - light_texCoord.y;
        float4 lightMapColor = tex2D(u_LightMap, light_texCoord);
    #endif
#endif

#ifdef SHADOW
    shadow *= getShadow(g_ShadowMap, input.v_ShadowProjCoord, u_ShadowZFar, u_ShadowMapSizeInv);
#endif

    shadow = saturate(1.0 - shadowIntensity + shadowIntensity * shadow);



    float shadowParams = 0.4 + shadow * 0.6;
	float3 worldLightDir = u_DirLightDir;
	float3 worldNormal = normalize(input.v_VertexNormal);

#ifdef NORMAL_MAP
	float3 texNormal = tex2D(u_NormalMap, diffuseTexcoord).xyz * 2.0 - 1.0;
	float3 tangnet = normalize(input.v_VertexTangent);
	float3 binormal = normalize(input.v_VertexBinormal);
	float3x3 tbmMat = float3x3(tangnet, binormal, worldNormal);

	worldNormal = mul(texNormal, tbmMat);
	worldNormal = normalize(worldNormal);
#endif
	#ifndef MIRRORING
		worldNormal *= -input.frontFace;
	#endif

	float3 ambient = u_AmbientColor;
	float3 dirLightColor = u_DirLightColor.xyz;
	float3 diffuse = dirLightColor * max(dot(worldLightDir, worldNormal), 0.0);
    diffuse *= shadow;
	float3 worldViewDir = normalize(input.v_WorldViewDir.xyz);
    float worldViewDirDotNormal = dot(worldViewDir, worldNormal);
	float shininess = pow(2.0, glossiness * 10.0 + 1.0);
	float3 specular = (shininess + 8.0) * PIx8Inv * dirLightColor * pow(max(dot(normalize(worldViewDir + worldLightDir), worldNormal), 0.0), shininess);

	specular *= shadow;
	float luminance = getLuminance(dirLightColor + ambient);
	float3 envLightColor = float3(luminance, luminance, luminance);

#ifdef EXTRA_LIGHTING
	diffuse += u_ExtraLightColor * max(dot(u_ExtraLightDir, worldNormal), 0.0) * shadowParams;
	specular += (shininess + 8.0) * PIx8Inv * u_ExtraLightColor * pow(max(dot(normalize(worldViewDir + u_ExtraLightDir), worldNormal), 0.0), shininess) * shadowParams;
	envLightColor += u_ExtraLightColor;
#endif

#ifdef LIGHTMAP_LIGHT
    lightMapColor.rgb *= 3.0;
    diffuse += lightMapColor.rgb * shadowParams;
    specular += lightMapColor.rgb / 3.0 * lightMapColor.a * 3.0 * shadowParams;
#else

#ifdef MULTI_LIGHTING
	float3 pointLightDir;
	float distance;
	float decay;
	#ifdef EDITOR
	    float num_light = 0;
	#endif
	for (int i = 0; i < u_PointLightNum; i++)
	{

	    #ifdef EDITOR
	        num_light ++;
	    #endif
		pointLightDir = u_WorldPointLightPosDecayArray[i].xyz - input.v_WorldPos;
		distance = length(pointLightDir);
		if(distance < u_PointLightColorRadiusArray[i].w)
		{
		    decay = pow(clamp((u_PointLightColorRadiusArray[i].w - distance) / u_PointLightColorRadiusArray[i].w, 0.0, 1.0), u_WorldPointLightPosDecayArray[i].w);
            pointLightDir = normalize(pointLightDir);
            diffuse += u_PointLightColorRadiusArray[i].xyz * max(dot(pointLightDir, worldNormal), 0.0) * decay * shadowParams;
            specular += (shininess + 8.0) * PIx8Inv * u_PointLightColorRadiusArray[i].xyz * pow(max(dot(normalize(worldViewDir + pointLightDir), worldNormal), 0.0), shininess) * decay * shadowParams;
            envLightColor += u_PointLightColorRadiusArray[i].xyz * decay;
		}
	}
#endif

#endif

#ifdef DECAL
#ifndef DECAL_IGNORE
	float4 decal = getDecal(u_DecalMap, input.v_TexCoord1.zw, input.v_WorldViewDir.w, u_DecalZFar);
	diffuse += decal.xyz * shadowParams;
	diffuseColor.xyz *= decal.a;
#endif
#endif

#ifdef REFLECTION_MAP
	float reflection = tex2D(u_ReflectionMap, diffuseTexcoord).a;
#else
	float reflection = u_Reflection;
#endif
    float worldViewWorldNormal = clamp(worldViewDirDotNormal, 0.0, 1.0);
	float n2 = pow(1.2, reflection * 20.0) + 0.000293;
	float3 viewFR = getFresnelReflectance(worldViewWorldNormal, n1, n2);

#ifdef REFRACTION
	float3 viewNormal = normalize(mul(worldNormal.xyz, g_ViewMatrix).xyz);
	//float2 xxx = input.v_ProjPos.xy / input.v_ProjPos.w * float2(0.5, -0.5) + float2(0.5, 0.5);
	float2 refractionCoord = input.v_ProjPos  * 0.7 + 0.15 + float2(viewNormal.x, viewNormal.y) * 0.15;

#ifdef UNIFORM_REFRACTION_TRANSPARENT
	float transparent = u_RefractionTransparent;
#else
	float transparent = alpha;
#endif

	float4 refractionColor = (1.0 - transparent) * sRGBTexture2D(u_RefractionMap, refractionCoord);

	float viewFRInternal = abs(worldViewDirDotNormal) * 0.25 + 0.5;        //¶ÔÓÚÈ«ÄÚ·´Éäµ¼ÖÂµÄ°ß²µµÄHack£¬¶ÔÓÚ·¨Ïß·Ç³£Æ½»¬ÓÖÈ±·¦Ï¸½ÚµÄ±íÃæ´æÔÚÒ»¶¨è¦´Ã
	float3 refraction = refractionColor.xyz;
	ambient = ambient * transparent * viewFR.z;
	diffuse = (refraction * (1.0 - transparent) + diffuse * transparent) * viewFRInternal;
	glowColor *= viewFRInternal;
#endif

#ifdef ENVIRONMENT_MAPPING
	float4 refDir = float4(reflect(-worldViewDir, worldNormal), 0.0);
#ifdef CYLINDER_MAPPING
	float3 environment = textureCylinderLod(u_EnvironmentMap, refDir).xyz;
#else
	float3 environment = texCUBElod(u_EnvironmentMap, refDir).xyz;
#endif
	environment = sRGBToRGB(environment);
	environment *= 0.75 * shadowParams * envLightColor;

    #ifdef BACKLIGHT
        float3 finalColor = diffuseColor.xyz * ambient * shadowParams;
    #else
        float3 finalColor = diffuseColor.xyz * ambient + diffuseColor.xyz * diffuse * (1.0 - viewFR.z) + glowColor;
        #ifndef DISABLE_SPECULAR
            finalColor += specularColor * ((specular + environment) * (viewFR.x * 0.75 + viewFR.y * 0.25));
        #endif
	#endif
#else
    #ifdef BACKLIGHT
        float3 finalColor = diffuseColor.xyz * ambient * shadowParams;
    #else
	    float3 finalColor = diffuseColor.xyz * ambient + diffuseColor.xyz * diffuse + glowColor;
	     #ifndef DISABLE_SPECULAR
	        finalColor += specularColor * (specular * (viewFR.x * 0.75 + viewFR.y * 0.25));
	     #endif
	#endif
#endif
	output.color = float4(finalColor.xyz, alpha);
	output.color.xyz = RGBTosRGB(output.color.xyz);



#ifdef FLOOD2
	float3 flood2Color = u_Flood2Color;
	#ifdef FLOOD_MAP
		flood2Color *= tex2D(u_FloodMap2, input.v_TexCoord0.xy + u_Flood2.zw * g_Time).xyz;
	#endif
	flood2Color = max(flood2Color, float3(0.1, 0.1, 0.1)) * u_Flood2.x * pow((1.0 - worldViewDirDotNormal), u_Flood2.y);
	output.color.rgb += flood2Color;
#else
#ifdef FLOOD
	float3 floodColor = u_FloodColor;
#ifdef FLOOD_TEX
	floodColor *= tex2D(u_FloodMap, v_TexCoord0.xy + u_FloodUVVector * g_Time).xyz;
#endif
	floodColor = max(floodColor, float3(0.1, 0.1, 0.1)) * u_FloodScale * pow((1.0 - worldViewDirDotNormal), 1.5);
	output.color.xyz += floodColor;
#endif
#endif
#ifdef DISCARDALPHA
    output.color.a = 1.0;
#endif

#ifdef FADE
    output.color.a *= u_FadeProcess;
#endif
#ifdef VANISH
	if (vanishAlpha < u_VanishSpeed)
	{
		output.color.rgb *= float3(u_VanishColor * u_VanishPower);
	}
#endif

#ifdef DEPTH
    float depth = input.v_TexCoord0.z / input.v_TexCoord0.w;
	output.depth = float4(depth, depth, depth, 1.0);
#endif

#ifdef MTRL_ALPHA
	output.color.a *= u_MtrlAlpha;
#endif

#ifdef EDITOR
    #ifdef MULTI_LIGHTING
        if(num_light > 4.5)
        {
            output.color.rgb = lerp(output.color.rgb, float3(1.0, 0.0, 0.0), 0.5);
        }
    #endif
#endif

#ifdef REVERSE
    output.color.rgb = output.color.rgb * output.color.a;
    output.color.a = 1.0 - output.color.a;
#endif

	return output;
}
