#define MAX_INSTANCE_COUNT 30
struct VertexShaderInput
{
    float4 g_Position : POSITION;
#ifdef TEXTURE
    float2 g_TexCoord0 : TEXCOORD0;
#endif
    int2 g_InstanceId : TEXCOORD7;
};

#ifdef TEXTURE
#ifdef TILED
float4 u_TextureTileInfo ;     //xyΪTile����zΪ��������
float2 u_PrivateInfo[MAX_INSTANCE_COUNT] ;
#endif
#endif

float4x3 u_WorldMatrixArray[MAX_INSTANCE_COUNT];
float4x4 g_ViewProjMatrix;
#ifdef STRETCHED
float4x4 g_ViewMatrix ;
float4x4 g_ProjMatrix ;
float3 u_Dir[MAX_INSTANCE_COUNT] ;
float2 u_Scale[MAX_INSTANCE_COUNT];
#endif

float4 u_ColorArray[MAX_INSTANCE_COUNT] ;


struct PixelShaderInput
{
    float4 pos : POSITION;
#ifdef TEXTURE
    float2 v_TexCoord0 : TEXCOORD0;
#ifdef TILED
#ifdef BLEND
    float2 v_TexCoord1 : TEXCOORD1;
    float4 v_Factor : TEXCOORD2;
#endif
#endif
#endif

#ifdef DISTORTION_EFFECT
    float4 v_ProjPos : TEXCOORD3;
#else
    #ifdef DEPTH
        float4 v_ProjPos : TEXCOORD3;
    #endif
#endif

    float4 v_Color : TEXCOORD5;

};

float mod(float x, float y)
{
    float r = x - y * floor(x / y );
    return r;
}

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;

    #define instanceId input.g_InstanceId.x

#ifdef TEXTURE
#ifdef TILED
    float tileCount = u_TextureTileInfo.x * u_TextureTileInfo.y;
    float tileIndex = mod(floor(u_PrivateInfo[instanceId].x / u_TextureTileInfo.z + u_PrivateInfo[instanceId].y), tileCount);

    output.v_TexCoord0.x = (input.g_TexCoord0.x + mod(tileIndex, u_TextureTileInfo.x)) / u_TextureTileInfo.x;
    output.v_TexCoord0.y = (input.g_TexCoord0.y + floor(tileIndex / u_TextureTileInfo.x)) / u_TextureTileInfo.y;

#ifdef BLEND
    float nextTileIndex = mod(tileIndex + 1.0, tileCount);
    output.v_TexCoord1.x = (input.g_TexCoord0.x + mod(nextTileIndex, u_TextureTileInfo.x)) / u_TextureTileInfo.x;
    output.v_TexCoord1.y = (input.g_TexCoord0.y + floor(nextTileIndex / u_TextureTileInfo.x)) / u_TextureTileInfo.y;
    output.v_Factor = mod(u_PrivateInfo[instanceId].x + u_PrivateInfo[instanceId].y * u_TextureTileInfo.z, u_TextureTileInfo.z) / u_TextureTileInfo.z;
#endif
#else
    output.v_TexCoord0 = input.g_TexCoord0;
#endif
#endif


#ifdef STRETCHED
    float3 viewDir = normalize(mul(u_Dir[instanceId], g_ViewMatrix).xyz);
    float2 offsetWidth = normalize( float2(viewDir.y, -viewDir.x) ) * input.g_Position.x * u_Scale[instanceId].x / 2.0;

    float3 pos = u_WorldMatrixArray[instanceId][3].xyz + input.g_Position.y * u_Dir[instanceId].xyz * u_Scale[instanceId].y / 2.0;
    float4 viewPos = mul(float4(pos, 1.0), g_ViewMatrix);
    viewPos.xy += offsetWidth;
    output.pos = mul(viewPos, g_ProjMatrix);
#else
    float3 worldPos = mul(input.g_Position, u_WorldMatrixArray[instanceId]).xyz;
    output.pos = mul(float4(worldPos, 1.0), g_ViewProjMatrix);
#endif

#ifdef DISTORTION_EFFECT
    output.v_ProjPos = output.pos;
#else
#ifdef DEPTH
    output.v_ProjPos = output.pos;
#endif
#endif

    output.v_Color = u_ColorArray[instanceId];
    return output;
}
