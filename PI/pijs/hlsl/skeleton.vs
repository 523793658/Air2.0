struct VertexShaderInput
{
    float4 g_Position : POSITION;
    float2 g_TexCoord0 : TEXCOORD0;
	float3 g_Normal : NORMAL0;

#ifdef SKELETON
    float2 g_BlendIndices : BLENDINDICES0;
#elif HARDWARE_SKELETON
    float4 g_BlendIndices : BLENDINDICES0;
    float4 g_BlendWeights : BLENDWEIGHT0;
#endif
};
float4x4 g_WorldViewProjMatrix ;
float4x3 g_NormalMatrix ;


#ifdef SKELETON
sampler2D u_BoneTex;
float u_BoneNumInv;
#elif HARDWARE_SKELETON
float u_boneInfo;
float4x3 u_boneMatrices[60] ;
#endif


struct PixelShaderInput
{
    float4 pos : POSITION;
	float2 v_TexCoord0 : TEXCOORD0;
	float3 v_VertexNormal : TEXCOORD3;
};

PixelShaderInput main(VertexShaderInput input)
{
#ifdef SKELETON
    float4x4 m;
    float idx = (input.g_BlendIndices.x + 0.5 ) * u_BoneNumInv;
    m[0] = tex2Dlod(u_BoneTex, float4(0.125, idx, 0.0, 0.0));
    m[1] = tex2Dlod(u_BoneTex, float4(0.375, idx, 0.0, 0.0));
    m[2] = tex2Dlod(u_BoneTex, float4(0.625, idx, 0.0, 0.0));
    m[3] = tex2Dlod(u_BoneTex, float4(0.875, idx, 0.0, 0.0));
    float4 blendPos = mul(input.g_Position, m);
	float3 normal = mul(input.g_Normal, m);
#elif HARDWARE_SKELETON
    float4x3 m = u_boneMatrices[input.g_BlendIndices[0] - u_boneInfo] * input.g_BlendWeights[0];
    for (int i = 1; i < 4; i++)
    {
        m += u_boneMatrices[input.g_BlendIndices[i] - u_boneInfo] * input.g_BlendWeights[i];
    }
    float4 blendPos = float4(mul(input.g_Position, m), 1.0);
    float3 normal =  mul(input.g_Normal, m);
#else

	float4 blendPos = input.g_Position;
	#define normal input.g_Normal
#endif
    PixelShaderInput output;
    output.pos = mul(blendPos, g_WorldViewProjMatrix);
	output.v_VertexNormal = normalize(mul(normal, g_NormalMatrix).xyz);
    output.v_TexCoord0 = input.g_TexCoord0;
    return output;
}
