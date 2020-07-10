#define sizeInv 0.000244140625
#define PIx8Inv 0.039788735
#define MAX_MULTI_LIGHTS 40
float offset_lookup(sampler2D map, float4 loc, float2 offset)
{
    float shadow = tex2Dproj(map, float4(loc.xy + offset * sizeInv * loc.w, loc.z - 0.0005, loc.w)).r;
    return shadow;
}

struct PixelShaderInput
{
    float4 v_ProjPos : POSITION;
    float3 v_WorldPos : TEXCOORD0;
    float3 v_VertexNormal : NORMAL;
    float2 v_TexCoord : TEXCOORD1;
#ifdef NORMAL_MAP
    float3 v_VertexTangent : TEXCOORD4;
    float3 v_VertexBinormal : TEXCOORD5;
#endif
};

struct PixelShaderOutput
{


#ifdef MULTI_LIGHTING
    float4 lighting: COLOR0;
    float4 specular : COLOR1;
#else
    float4 ao : COLOR0;
#endif
};

float3 u_WorldViewDir;

sampler2D u_ShadowMap0;
float4x4 u_ShadowMatrix0;

float4x4 g_WorldMatrix ;

#ifdef NORMAL_MAP
    sampler2D u_NormalMap;
#endif

int u_PointLightNum;
float4 u_PointLightColorRadiusArray[MAX_MULTI_LIGHTS];
float4 u_WorldPointLightPosDecayArray[MAX_MULTI_LIGHTS];



#ifdef GLOSSINESS_MAP
   sampler2D u_GlossinessMap;
#else
   float u_Glossiness;
#endif


PixelShaderOutput main(PixelShaderInput input)
{

    PixelShaderOutput output;
#ifndef MULTI_LIGHTING
    float shadow = 1.0;
    float4 shadow_pos0 = mul(float4(input.v_WorldPos, 1.0), u_ShadowMatrix0);
    float4 shadow_coord0 = float4(0.5 * shadow_pos0.xy + float2(0.5, 0.5) * shadow_pos0.w, shadow_pos0.z, shadow_pos0.w);
    shadow_coord0.y = 1.0 - shadow_coord0.y;
    float shadow0 =  (
        offset_lookup(u_ShadowMap0, shadow_coord0, float2(-0.75, 0.75)).r +
        offset_lookup(u_ShadowMap0, shadow_coord0, float2(0.75, 0.75)).r +
        offset_lookup(u_ShadowMap0, shadow_coord0, float2(-0.75, -0.75)).r +
        offset_lookup(u_ShadowMap0, shadow_coord0, float2(0.75, -0.75)).r
        ) * 0.25;
    shadow = min(shadow, shadow0);
    output.ao = float4(shadow, 0.0, 0.0, 1.0);
#else

    float3 worldNormal = normalize(input.v_VertexNormal);
#ifdef NORMAL_MAP
    float3 texNormal = tex2D(u_NormalMap, input.v_TexCoord0).xyz * 2.0 - 1.0;
    float3 tangnet = normalize(input.v_VertexTangent);
    float3 binormal = normalize(input.v_VertexBinormal);
    float3x3 tbmMat = float3x3(tangnet, binormal, worldNormal);

    worldNormal = mul(texNormal, tbmMat);
    worldNormal = normalize(worldNormal);
#endif


#ifdef GLOSSINESS_MAP
	float glossiness = tex2D(u_GlossinessMap, diffuseTexcoord).a;
#else
	float glossiness = u_Glossiness;
#endif

    float shininess = pow(2.0, glossiness * 10.0 + 1.0);
    float3 pointLightDir;
    float distance;
    float decay;
    float3 specular = 0;
    float3 diffuse = 0;
    for(int i = 0; i < u_PointLightNum; i++)
    {
         pointLightDir = u_WorldPointLightPosDecayArray[i].xyz - input.v_WorldPos;
         distance = length(pointLightDir);
         if(distance < u_PointLightColorRadiusArray[i].w)
         {
            decay = pow(clamp((u_PointLightColorRadiusArray[i].w - distance) / u_PointLightColorRadiusArray[i].w, 0.0, 1.0),  u_WorldPointLightPosDecayArray[i].w);
            pointLightDir = normalize(pointLightDir);
            diffuse += u_PointLightColorRadiusArray[i].xyz * max(dot(pointLightDir, worldNormal), 0.0) * decay;
            specular += (shininess + 8.0) * PIx8Inv * u_PointLightColorRadiusArray[i].xyz * pow(max(dot(normalize(u_WorldViewDir + pointLightDir), worldNormal), 0.0), shininess) * decay;

         }
    }
    float ls = 0.299*specular.r + 0.587*specular.g + 0.114*specular.b;
    float ld = 0.299*diffuse.r + 0.587*diffuse.g + 0.114*diffuse.b;
    output.lighting = float4(diffuse / 3.0, 1.0);
    float ss = saturate(ls / ld / 3.0);
    output.specular = float4(ss, 0.0, 0.0, 1.0);
#endif
    return output;
}