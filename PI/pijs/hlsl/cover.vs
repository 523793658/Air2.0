struct VertexShaderInput
{
    float4 g_Position : POSITION0;
    float3 g_Normal : NORMAL0;
#ifdef HARDWARE_SKELETON
    int4 g_BlendIndices : BLENDINDICES0;
    float4 g_BlendWeights : BLENDWEIGHT0;
#endif
};
struct VertexShaderOutput
{
    float3 v_WorldPos : COLOR0;
    float3 v_VertexNormal : TEXCOORD0;
    float4 pos : POSITION;
};

float4x4 g_WorldViewProjMatrix ;
float4x4 g_WorldMatrix ;
float4x4 g_NormalMatrix ;




#ifdef HARDWARE_SKELETON
    float u_boneInfo;
    float4x3 u_boneMatrices[60] ;
#endif

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    #ifdef HARDWARE_SKELETON
        float4x3 m = u_boneMatrices[input.g_BlendIndices[0] - u_boneInfo] * input.g_BlendWeights[0];
      	for (int i = 1; i < 4; i++)
      	{
            m += u_boneMatrices[input.g_BlendIndices[i] - u_boneInfo] * input.g_BlendWeights[i];
      	}
        float4 modelPos = float4(mul(input.g_Position, m), 1.0);
        float3 modelNormal =  mul(input.g_Normal, m);
    #else
        float3 modelNormal = input.g_Normal;
        float4 modelPos = input.g_Position;
    #endif
    output.v_WorldPos = mul(modelPos, g_WorldMatrix ).xyz;
   	output.v_VertexNormal = normalize(mul(float4(modelNormal, 0.0), g_NormalMatrix).xyz);
    output.pos = mul(modelPos, g_WorldViewProjMatrix);
    return output;
}