struct VertexShaderInput
{
    float4 g_Position : POSITION0;
    float4 g_Normal : NORMAL0;

    float2 g_TexCoord0 : TEXCOORD0;

    #ifndef BEAST_UV_0
        float2 g_TexCoord1 : TEXCOORD1;
    #endif


    #ifdef NORMAL_MAP
        float4 g_Tangent : TANGENT0;
    #endif
};


float4x4 g_WorldMatrix ;struct VertexShaderOutput
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
float4x3 g_NormalMatrix ;


float4x4 u_UVMatrix;

#ifdef MIRRORING
float3 u_Mirroring ;
#endif

#ifdef TERRAIN
float2 u_TerrainTexScale;
#endif

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;

    #ifndef BEAST_UV_0
        #define texCoord1 input.g_TexCoord1
    #else
        #define texCoord1 input.g_TexCoord0
    #endif

    float4 uv = float4(texCoord1, 1.0, 1.0);
    output.v_ProjPos = float4(uv.xyz * 2.0 - 1.0, 1.0);







    //光照相关
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

    float4 modelPos = input.g_Position;
    float3 modelNormal = input.g_Normal;
#ifdef NORMAL_MAP
    float4 modelTangent = tangent;
    float3 modelBinormal = cross(modelNormal, modelTangent.xyz) * modelTangent.w;
#endif

#ifdef MIRRORING
    modelPos.xyz *= u_Mirroring;
    modelNormal *= u_Mirroring;
    #ifdef NORMAL_MAP
        modelTangent.xyz *= u_Mirroring;
        modelBinormal *= u_Mirroring;
    #endif
#endif


#ifdef NORMAL_MAP
    output.v_VertexTangent = normalize(mul(modelTangent.xyz, g_NormalMatrix).xyz);
    output.v_VertexBinormal = normalize(mul(modelBinormal, g_NormalMatrix).xyz);
#endif

    output.v_VertexNormal = normalize(mul(modelNormal, g_NormalMatrix).xyz);
    output.v_WorldPos = mul(modelPos, g_WorldMatrix).xyz;


#ifdef TERRAIN
    output.v_TexCoord =  input.g_TexCoord0 * u_TerrainTexScale;
#else
    output.v_TexCoord =  input.g_TexCoord0;
#endif
    return output;
}