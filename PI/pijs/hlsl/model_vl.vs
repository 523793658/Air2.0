#define MAX_MULTI_LIGHTS 4
#define PI 3.1415926

float getLuminance(float3 rgbColor) {
	return rgbColor.r * 0.299 + rgbColor.g * 0.587 + rgbColor.b * 0.114;
}

float4 smoothCurve(float4 x)
{
	return x * x * (3.0 - 2.0 * x);
}


float4 triangleWave(float4 x)
{
	return abs(frac(x + 0.5) * 2.0 - 1.0);
}


float4 smoothTriangleWave(float4 x)
{
	return smoothCurve(triangleWave(x));
}


struct VertexShaderInput
{
	float4 g_Position : POSITION0;
	float4 g_Color : COLOR0;
	float2 g_TexCoord0 : TEXCOORD0;

#ifdef HARDWARE_SKELETON
	int4 g_BlendIndices : BLENDINDICES0;
	float4 g_BlendWeights : BLENDWEIGHT0;
#endif

#ifndef BEAST_UV_0
	float2 g_TexCoord1 : TEXCOORD1;
#endif


#ifdef LIGHTING
	float3 g_Normal : NORMAL0;
#endif

};

struct VertexShaderOutput
{
	float4 pos : POSITION;

	// DEPTH 对应后两个成�? v_Depth
	float4 v_TexCoord0 : TEXCOORD0;

#ifdef LIGHTMAP
	float2 v_TexCoord1 : TEXCOORD1;
#endif

#ifdef LIGHTING
	// FLOOD 对应diffuse的最后一个分�? v_FloodFactor
	float4 v_Diffuse : COLOR0;

	float4 v_ViewPos : TEXCOORD2;
	float3 v_EnvLightColor : TEXCOORD3;

#ifdef ENVIRONMENT_MAPPING
	float3 v_EnvironmentCoord : TEXCOORD4;
#endif

#ifdef DECAL
	#ifndef DECAL_IGNORE
		float2 v_DecalProjCoord : TEXCOORD6;
	#endif
#endif

#ifdef VERTEX_COLOR
	float4 v_VertexColor : TEXCOORD7;
#endif

    float3 v_Ambient : TEXCOORD5;

#endif
};



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
    #define u_ShadowMapSizeInv        u_ExtraLightData.diffuseDir1.w
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


float4x4 g_WorldViewProjMatrix ;
float4x4 g_WorldMatrix ;
#ifdef MIRRORING
float3 u_Mirroring ;
#endif

#ifdef FLOOD2
	float4 u_Flood2 ;
#endif

float3 g_ViewPosition;
#ifdef HARDWARE_SKELETON
float u_boneInfo;
float4x3 u_boneMatrices[60] ;
#endif

#ifdef VEGETATION_ANIM
float4 u_WindBlend ;
#ifdef VEGETATION_ANIM_LEAF
    float4 u_VegetationParams;
#ifdef VEGETATION_LEAF_MAP
	sampler2D u_VegetationLeafMap ;
#endif
#endif
#endif

#ifdef LIGHTING
float u_Glossiness ;
float4x4 g_WorldViewMatrix ;
float4x4 g_NormalMatrix ;

#ifdef ENVIRONMENT_MAPPING
float3 v_EnvironmentCoord ;
#endif

#ifdef DECAL
#ifndef DECAL_IGNORE
float4x4 u_DecalMatrix ;
#endif
#endif


#ifdef LIGHTMAP
    float4 u_LightMapOS;
#endif

#endif



VertexShaderOutput main(VertexShaderInput input)
{

	VertexShaderOutput output;
	output.v_TexCoord0 = float4(input.g_TexCoord0, 0.0, 0.0);
#ifdef LIGHTMAP
    #ifdef BEAST_UV_0
        output.v_TexCoord1 = input.g_TexCoord0 * u_LightMapOS.zw + u_LightMapOS.xy;
    #else
    	output.v_TexCoord1 = input.g_TexCoord1 * u_LightMapOS.zw + u_LightMapOS.xy;
    #endif
	output.v_TexCoord1.y = 1.0 - output.v_TexCoord1.y;
#endif
#ifdef HARDWARE_SKELETON
	float4x3 m = u_boneMatrices[input.g_BlendIndices[0] - u_boneInfo] * input.g_BlendWeights[0];
    for (int i = 1; i < 4; i++)
    {
        m += u_boneMatrices[input.g_BlendIndices[i] - u_boneInfo] * input.g_BlendWeights[i];
    }
    float4 modelPos = float4(mul(input.g_Position, m), 1.0);
#ifdef LIGHTING
    float3 modelNormal =  mul(input.g_Normal, m);
#endif
#else
	float4 modelPos = input.g_Position;
    float3 modelNormal = input.g_Normal;
#endif

#ifdef VEGETATION_ANIM
#ifdef VEGETATION_ANIM_LEAF
	float centerScale = min(length(modelPos.xz), 1.0);
    #ifdef VEGETATION_LEAF_MAP
        float leafValue = tex2Dlod(u_VegetationLeafMap, float4(output.v_TexCoord0.xy, 0.0, 0.0)).a;
        float edgeAttenuation = leafValue * 0.1;
        float branchAttenuation = leafValue * 0.02;
    #else
        float edgeAttenuation = centerScale * 0.1;
        float branchAttenuation = centerScale * 0.02;
    #endif
    float branchPhase = dot(modelPos.xyz, float3(1.0, 1.0, 1.0));
	branchPhase += u_WindBlend.w;
	float vertexPhase = dot(modelPos.xyz, float3(branchPhase, branchPhase, branchPhase));
	float2 wavesIn = g_Time + float2(vertexPhase, branchPhase);
	float4 waves = (frac(wavesIn.xxyy * float2(u_VegetationParams.y, 1).xxyy * float2(1.0, u_VegetationParams.w).xxyy * float4(1.975, 0.793, 0.375, 0.193)) * 2.0 - 1.0);
	waves = smoothTriangleWave(waves);
	float2 wavesSum = waves.xz + waves.yw;
	modelPos.xzy += wavesSum.xxy * float3(edgeAttenuation * u_VegetationParams.x * modelNormal.xz, branchAttenuation * u_VegetationParams.z);
#endif
	float posLength = length(modelPos.xyz);
	float bf = max(modelPos.y, 0.0) * u_WindBlend.z;
	bf += 1.0;
	bf *= bf;
	bf = bf * bf - bf;
	float3 newPos = modelPos.xyz;
	newPos.xz += u_WindBlend.xy * bf;
	modelPos.xyz = normalize(newPos) * posLength;
#endif
#ifdef MIRRORING
	modelPos.xyz *= u_Mirroring;
#endif
	output.pos = mul(modelPos, g_WorldViewProjMatrix);

#ifdef LIGHTING
	float3 worldPos = mul(modelPos, g_WorldMatrix).xyz;
	output.v_Ambient = u_AmbientColor;
#ifdef MIRRORING
	modelNormal *= u_Mirroring;
#endif
	float3 worldNormal = normalize(mul(float4(modelNormal, 0.0), g_NormalMatrix).xyz);
	float3 worldLightDir = normalize(u_DirLightDir);
	float3 worldViewDir = normalize(worldPos - g_ViewPosition);
	float3 diffuse = float3(0.0, 0.0, 0.0);
	float diffuseDotValue = dot(worldLightDir, worldNormal);
	#ifdef BACKLIGHT
	    diffuseDotValue = abs(diffuseDotValue);
	#endif
	diffuse = u_DirLightColor * max(diffuseDotValue, 0.0);
	float shininess = pow(2.0, u_Glossiness * 10.0 + 1.0);
#ifdef ENVIRONMENT_MAPPING
	float3 refNor = worldNormal;
	output.v_EnvironmentCoord = reflect(worldViewDir, refNor);
#endif
	float x = getLuminance(u_DirLightColor + u_AmbientColor);
	output.v_EnvLightColor = float3(x, x, x);
#ifdef EXTRA_LIGHTING
    float extraDotValue = dot(u_ExtraLightDir, worldNormal);
    #ifdef BACKLIGHT
        extraDotValue = abs(extraDotValue);
    #endif
	diffuse += u_ExtraLightColor * max(extraDotValue, 0.0);
	output.v_EnvLightColor += u_ExtraLightColor;
#endif
	output.v_Diffuse = float4(diffuse, 0.0);
	output.v_ViewPos.w = u_ShadowDensity;
	output.v_ViewPos.xyz = mul(modelPos, g_WorldViewMatrix).xyz;

#ifdef DECAL
#ifndef DECAL_IGNORE
	output.v_DecalProjCoord = mul(float4(output.v_ViewPos.xyz, 1.0), u_DecalMatrix).xy;
#endif
#endif

#ifdef VERTEX_COLOR
	output.v_VertexColor = input.g_Color;
#endif
#ifdef FLOOD2
	output.v_Diffuse.w = pow((1.0 - dot(-worldNormal, worldViewDir)), u_Flood2.y);
#else
#ifdef FLOOD
    output.v_Diffuse.w = pow((1.0 - dot(-worldNormal, worldViewDir)), 1.5);
#endif
#endif


#ifdef FLOOD
	output.v_Diffuse.w = pow((1.0 - dot(-worldNormal, worldViewDir)), 1.5);
#endif
#endif

#ifdef DEPTH
	output.v_TexCoord0.zw = output.pos.zw;
#endif
return output;
}