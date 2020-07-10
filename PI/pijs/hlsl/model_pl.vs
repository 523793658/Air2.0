struct VertexShaderInput
{
	float4 g_Color : COLOR0;
	float4 g_Position : POSITION0;
	float3 g_Normal : NORMAL0;
	float2 g_TexCoord0 : TEXCOORD0;
#ifndef BEAST_UV_0
    float2 g_TexCoord1 : TEXCOORD1;
#endif
	
#ifdef HARDWARE_SKELETON
	float4 g_BlendIndices : BLENDINDICES0;
	float4 g_BlendWeights : BLENDWEIGHT0;
#endif

#ifdef LIGHTING
#ifdef NORMAL_MAP
	float4 g_Tangent : TANGENT0;
#endif
#endif
};

struct VertexShaderOutput
{
	float4 pos : POSITION;
	
	// DEPTH 定义了后两个成员
	float4 v_TexCoord0 : TEXCOORD0;
	float3 v_WorldPos : TEXCOORD1;
#ifdef LIGHTING
	float4 v_WorldViewDir : TEXCOORD2;
	float3 v_VertexNormal : TEXCOORD3;
#ifdef NORMAL_MAP
	float3 v_VertexTangent : TEXCOORD4;
	float3 v_VertexBinormal : TEXCOORD5;
#endif
#ifdef TERRAIN
    float4 v_ProjPos : TEXCOORD6;
#else
    float2 v_ProjPos : TEXCOORD6;
#endif


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
#endif
};


float4 smoothTriangleWave(float4 x)
{
    float4 y = abs(frac(x + 0.5) * 2.0 - 1.0);
	return y * y * (3.0 - 2.0 * y);
}

float4x4 g_WorldViewProjMatrix ;
float4x4 g_WorldMatrix ;
float g_Time ;

float3 g_ViewPosition;

#ifdef MIRRORING
float3 u_Mirroring ;
#endif

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
	float4x4 g_WorldViewMatrix ;
	float4x3 g_NormalMatrix ;
#ifdef LIGHTMAP
    float4 u_LightMapOS;
#endif

#ifdef SHADOW
	float4x4 u_ShadowMatrix ;
#endif

#ifdef DECAL
#ifndef DECAL_IGNORE
	float4x4 u_DecalMatrix ;
#endif
#endif
#endif

float4x4 g_ViewProjMatrix;

#ifdef TERRAIN
float2 u_TerrainTexScale;
#endif

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	output.v_TexCoord0 = float4(input.g_TexCoord0, 0, 0);
#ifdef TERRAIN
    output.v_ProjPos.zw = input.g_TexCoord0.xy * u_TerrainTexScale;
#endif


#ifdef LIGHTING
#ifdef NORMAL_MAP
#ifdef DEBUG_TANGENT
	float4 tangent;
	tangent.xyz = normalize(float3(input.g_Normal.y, -input.g_Normal.z, input.g_Normal.x));
	float3 binormal = normalize(cross(input.g_Normal, tangent.xyz));
	tangent.xyz = normalize(cross(binormal, input.g_Normal));
	tangent.w = 1.0;
#else
	float4 tangent = input.g_Tangent;
#endif
#endif
#endif

#ifdef HARDWARE_SKELETON
	float4x3 m = u_boneMatrices[max(input.g_BlendIndices[0] - u_boneInfo, 0)] * input.g_BlendWeights[0];
	for (int i = 1; i < 4; i++)
	{
        m += u_boneMatrices[input.g_BlendIndices[i] - u_boneInfo] * input.g_BlendWeights[i];
	}
    float4 modelPos = float4(mul(input.g_Position, m), 1.0);
    float3 modelNormal =  mul(input.g_Normal, m);



#ifdef LIGHTING
#ifdef NORMAL_MAP
    float4 modelTangent = float4(mul(tangent, m), tangent.w);
#endif
#endif
#else

	float4 modelPos = input.g_Position;
    float3 modelNormal = input.g_Normal;
#ifdef LIGHTING
#ifdef NORMAL_MAP
	float4 modelTangent = tangent;
#endif
#endif
#endif
#ifdef RAGDOLL
  float4x4 m11 = {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    };
  #define worldViewMat g_ViewProjMatrix
    #define worldMat m11

#else
   #define worldViewMat g_WorldViewProjMatrix
    #define worldMat g_WorldMatrix
#endif


#ifdef VEGETATION_ANIM
#ifdef VEGETATION_ANIM_LEAF
	float centerScale = min(length(modelPos.xz), 1.0);
    #ifdef VEGETATION_LEAF_MAP
        float leafValue = tex2Dlod(u_VegetationLeafMap, float4(output.v_TexCoord0.xy, 0.0, 0.0)).a;
    #endif
    float branchPhase = dot(modelPos.xyz, float3(1.0, 1.0, 1.0));
	branchPhase += u_WindBlend.w;
	float vertexPhase = dot(modelPos.xyz, float3(branchPhase, branchPhase, branchPhase));
	float2 wavesIn = g_Time + float2(vertexPhase, branchPhase);
	float4 waves = wavesIn.xxyy * float2(u_VegetationParams.y, 0.4).xxyy * float2(0.4, u_VegetationParams.w).xxyy;
	waves = smoothTriangleWave(waves);
	float2 wavesSum = waves.xz + waves.yw;
	#ifdef VEGETATION_LEAF_MAP
		modelPos.xzy += wavesSum.xxy * float3(leafValue * u_VegetationParams.x * modelNormal.xz, leafValue * u_VegetationParams.z);
	#else
		modelPos.xzy += wavesSum.xxy * float3(centerScale * u_VegetationParams.x * modelNormal.xz, centerScale * u_VegetationParams.z);
	#endif
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
	output.pos = mul(modelPos, worldViewMat);
    output.v_WorldPos = mul(modelPos, worldMat).xyz;
#ifdef LIGHTING
	output.v_WorldViewDir.xyz = g_ViewPosition - output.v_WorldPos;
	float3 viewPos = mul(modelPos, g_WorldViewMatrix).xyz;
	output.v_WorldViewDir.w = viewPos.z;
	output.v_ProjPos.xy = output.pos.xy / output.pos.w * float2(0.5, -0.5) + float2(0.5, 0.5);
#ifdef NORMAL_MAP
	float3 modelBinormal = cross(modelNormal, modelTangent.xyz) * modelTangent.w;
#ifdef MIRRORING
	modelTangent.xyz *= u_Mirroring;
	modelBinormal *= u_Mirroring;
#endif
	output.v_VertexTangent = normalize(mul(modelTangent.xyz, g_NormalMatrix).xyz);
	output.v_VertexBinormal = normalize(mul(modelBinormal, g_NormalMatrix).xyz);
#endif
#ifdef MIRRORING
	modelNormal *= u_Mirroring;
#endif
	output.v_VertexNormal = normalize(mul(modelNormal, g_NormalMatrix).xyz);

#ifdef LIGHTMAP
    output.v_TexCoord1 = 0.0;
#else
    #ifdef DECAL
    #ifndef DECAL_IGNORE
        output.v_TexCoord1 = 0.0;
    #endif
    #endif
#endif

#ifdef LIGHTMAP
#ifdef BEAST_UV_0
    output.v_TexCoord1.xy = input.g_TexCoord0;// * u_LightMapOS.zw + u_LightMapOS.xy;
#else
    output.v_TexCoord1.xy = input.g_TexCoord1;//* u_LightMapOS.zw + u_LightMapOS.xy;
#endif
#endif



#ifdef DECAL
#ifndef DECAL_IGNORE
	output.v_TexCoord1.zw = mul(float4(viewPos, 1.0), u_DecalMatrix).xy;
#endif
#endif

#ifdef SHADOW
	output.v_ShadowProjCoord = mul(float4(viewPos, 1.0), u_ShadowMatrix);
#endif

#ifdef VERTEX_COLOR
	output.v_VertexColor = input.g_Color;
#endif

#endif
#ifdef DEPTH
	output.v_TexCoord0.zw = output.pos.zw;
#endif
	return output;
}