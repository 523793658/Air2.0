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
     return tex2Dproj(map, float4(loc.xy + offset * sizeInv * loc.w, loc.z - 0.0001, loc.w)).r;
 }

float getShadow(in sampler2D shadowMap, in float4 shadowCoord, float shadowIntensity, float sizeInv)
{
	float4 coord = float4(0.5 * shadowCoord.xy + float2(0.5, 0.5), shadowCoord.z, shadowCoord.w);

	coord.y = 1.0  - coord.y;

	float shadow =(
	offset_lookup(shadowMap, coord, float2(-0.75, 0.75), sizeInv).r +
	offset_lookup(shadowMap, coord, float2(0.75, 0.75), sizeInv).r +
	offset_lookup(shadowMap, coord, float2(-0.75, -0.75), sizeInv).r +
	offset_lookup(shadowMap, coord, float2(0.75, -0.75), sizeInv).r
	) * 0.25;
    return max(0.0, (1.0 - shadowIntensity + shadow * shadowIntensity));
}

float4 sRGBTexture2D(sampler2D tex, float2 texCoord) {
    float4 color = tex2D(tex, texCoord);
    color.xyz = pow(color.xyz, 2.2);
    return color;
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
	//float3 v_ViewPos : TEXCOORD6;

#ifdef SHADOW
	float4 v_ShadowProjCoord : TEXCOORD7;
#endif

#ifdef DECAL
#ifndef DECAL_IGNORE
	float2 v_DecalProjCoord : COLOR1;
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

#define MAX_MULTI_LIGHTS 4

#define n1 1.000293

//全局变量声明，使用固定register index。保证每帧更新一次
struct EnvironmentData
{
    float4 u_DirLightDir;
    float3 u_ExtraLightDir;
    float3 u_ExtraLightColor ;
    float4 u_ViewPosition;  //w是g_Time
    float4 u_DirLightColor;
    float4 u_AmbientColor;
} ;


EnvironmentData g_EnvironmentData : register(c0);

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
#endif

float2 u_TerrainTexScale;

#ifdef DIFFUSE_MAP
sampler2D u_DiffuseMap;
#endif

#ifdef SPECULAR_MAP
sampler2D u_SpecularMap;
#endif

#ifdef GLOW_MAP
sampler2D u_GlowMap;
float u_GlowScale;
#endif
float u_Glossiness;
float u_Reflection;

#ifdef MULTI_LIGHTING
int u_PointLightNum;
float4 u_PointLightColorRadiusArray[MAX_MULTI_LIGHTS];
float4 u_WorldPointLightPosDecayArray[MAX_MULTI_LIGHTS];
#endif


#ifdef LIGHTMAP
sampler2D u_LightMap;
sampler2D u_AOMap;
float4 u_LightMapOS;
#endif

#ifdef DECAL
sampler2D u_DecalMap;
float4 u_DecalZFar;
#endif


PixelShaderOutput main(PixelShaderInput input)
{
	PixelShaderOutput output;
	float alpha = 1.0;
#ifdef ALPHA_MAP
    float2 alphaMapTexcoord = input.v_TexCoord0.xy;
    alpha = tex2D(u_AlphaMap, alphaMapTexcoord).a;
    clip(alpha - g_AlphaCullOff - 0.000001);
#endif
	float2 diffuseTexcoord = input.v_TexCoord0.xy * u_TerrainTexScale;
	float4 diffuseColor = input.v_VertexColor;
#ifdef DIFFUSE_MAP
	diffuseColor *= sRGBTexture2D(u_DiffuseMap, diffuseTexcoord);
#endif
	diffuseColor.xyz *= u_DiffuseRevise;
	alpha *= diffuseColor.a;

	float3 specularColor = u_SpecularRevise;
#ifdef SPECULAR_MAP
	specularColor *= sRGBTexture2D(u_SpecularMap, diffuseTexcoord).xyz;
#endif
	float shadow = 1.0;
#ifdef SHADOW
    shadow = getShadow(g_ShadowMap, input.v_ShadowProjCoord, g_EnvironmentData.u_DirLightDir.w, g_EnvironmentData.u_DirLightColor.w);
#endif
    float shadowParams = 0.4 + shadow * 0.6;
	float3 worldLightDir = g_EnvironmentData.u_DirLightDir.xyz;
	float3 worldNormal = normalize(input.v_VertexNormal);

#ifdef NORMAL_MAP
	float3 texNormal = tex2D(u_NormalMap, diffuseTexcoord).xyz * 2.0 - 1.0;
	float3 tangnet = normalize(input.v_VertexTangent);
	float3 binormal = normalize(input.v_VertexBinormal);
	float3x3 tbmMat = float3x3(tangnet, binormal, worldNormal);
	worldNormal = mul(texNormal, tbmMat);
	worldNormal = normalize(worldNormal);
#endif
	float3 ambient = g_EnvironmentData.u_AmbientColor.xyz;
	float3 dirLightColor = g_EnvironmentData.u_DirLightColor.xyz;
	float3 diffuse = dirLightColor * max(dot(worldLightDir, worldNormal), 0.0);
    diffuse *= shadow;
	float3 worldViewDir = normalize(input.v_WorldViewDir.xyz);
    float worldViewDirDotNormal = dot(worldViewDir, worldNormal);
	float shininess = pow(2.0, u_Glossiness * 10.0 + 1.0);
	float3 specular = (shininess + 8.0) * PIx8Inv * dirLightColor * pow(max(dot(normalize(worldViewDir + worldLightDir), worldNormal), 0.0), shininess);
	specular *= shadow;
#ifdef MULTI_LIGHTING
	float3 pointLightDir;
	float distance;
	float decay;
	for (int i = 0; i < u_PointLightNum; i++)
	{
		pointLightDir = u_WorldPointLightPosDecayArray[i].xyz - input.v_WorldPos;
		distance = length(pointLightDir);
		decay = pow(clamp((u_PointLightColorRadiusArray[i].w - distance) / u_PointLightColorRadiusArray[i].w, 0.0, 1.0), u_WorldPointLightPosDecayArray[i].w);
		pointLightDir = normalize(pointLightDir);
		diffuse += u_PointLightColorRadiusArray[i].xyz * max(dot(pointLightDir, worldNormal), 0.0) * decay * shadowParams;
		specular += (shininess + 8.0) * PIx8Inv * u_PointLightColorRadiusArray[i].xyz * pow(max(dot(normalize(worldViewDir + pointLightDir), worldNormal), 0.0), shininess) * decay * shadowParams;
	}
#endif

#ifdef DECAL
	float4 decal = getDecal(u_DecalMap, input.v_DecalProjCoord, input.v_WorldViewDir.w, u_DecalZFar);
	diffuse += decal.xyz * shadowParams;
	diffuseColor.xyz *= decal.a;
#endif
    float worldViewWorldNormal = clamp(worldViewDirDotNormal, 0.0, 1.0);
	float n2 = pow(1.2, u_Reflection * 20.0) + 0.000293;
	float3 viewFR = getFresnelReflectance(worldViewWorldNormal, n1, n2);
	#ifdef GLOW_MAP
        float3 glowColor = sRGBTexture2D(u_GlowMap, diffuseTexcoord).xyz * u_GlowScale * diffuseColor.a;
    	float3 finalColor = diffuseColor.xyz * ambient + diffuseColor.xyz * diffuse + specularColor * (specular * (viewFR.x * 0.75 + viewFR.y * 0.25)) + glowColor;
	#else
    	float3 finalColor = diffuseColor.xyz * ambient + diffuseColor.xyz * diffuse + specularColor * (specular * (viewFR.x * 0.75 + viewFR.y * 0.25));
	#endif
	output.color = float4(finalColor, alpha);
	output.color.xyz = RGBTosRGB(output.color.xyz);
    float depth = input.v_TexCoord0.z / input.v_TexCoord0.w;
	output.depth = float4(depth, depth, depth, 1.0);
	return output;
}
