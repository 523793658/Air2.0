struct VertexShaderInput
{
    float4 g_Position : POSITION;
#ifdef TEXTURE
    float2 g_TexCoord0 : TEXCOORD0;
#endif
};

#ifdef TEXTURE
#ifdef TILED
float4 u_TextureTileInfo ;     //xyΪTile����zΪ��������
float2 u_PrivateInfo ;
#endif
#endif

#ifdef STRETCHED
float4x4 g_ViewMatrix ;
float4x4 g_ProjMatrix ;
float3 u_WorldPos ;
float3 u_Dir ;
float2 u_Scale ;
#else
float4x4 g_WorldViewProjMatrix ;
#endif

struct PixelShaderInput
{
    float4 pos : POSITION;
#ifdef TEXTURE
    float2 v_TexCoord0 : TEXCOORD0;
#ifdef TILED
#ifdef BLEND
    float2 v_TexCoord1 : TEXCOORD1;
    float v_Factor : TEXCOORD2;
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
};

float mod(float x, float y)
{
    float r = x - y * floor(x / y );
    return r;
}

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;

#ifdef TEXTURE
#ifdef TILED
    float tileCount = u_TextureTileInfo.x * u_TextureTileInfo.y;
    float tileIndex = mod(floor(u_PrivateInfo.x / u_TextureTileInfo.z + u_PrivateInfo.y), tileCount);

    output.v_TexCoord0.x = (input.g_TexCoord0.x + mod(tileIndex, u_TextureTileInfo.x)) / u_TextureTileInfo.x;
    output.v_TexCoord0.y = (input.g_TexCoord0.y + floor(tileIndex / u_TextureTileInfo.x)) / u_TextureTileInfo.y;

#ifdef BLEND
    float nextTileIndex = mod(tileIndex + 1.0, tileCount);
    output.v_TexCoord1.x = (input.g_TexCoord0.x + mod(nextTileIndex, u_TextureTileInfo.x)) / u_TextureTileInfo.x;
    output.v_TexCoord1.y = (input.g_TexCoord0.y + floor(nextTileIndex / u_TextureTileInfo.x)) / u_TextureTileInfo.y;
    output.v_Factor = mod(u_PrivateInfo.x + u_PrivateInfo.y * u_TextureTileInfo.z, u_TextureTileInfo.z) / u_TextureTileInfo.z;
#endif
#else
    output.v_TexCoord0 = input.g_TexCoord0;
#endif
#endif

#ifdef STRETCHED
    float3 viewDir = normalize(mul(float4(u_Dir, 0.0), g_ViewMatrix).xyz);
    float2 offsetWidth = normalize( float2(viewDir.y, -viewDir.x) ) * input.g_Position.x * u_Scale.x / 2.0;
    float3 pos = u_WorldPos + input.g_Position.y * u_Dir * u_Scale.y / 2.0;
    float4 viewPos = mul(float4(pos, 1.0), g_ViewMatrix);
    viewPos.xy += offsetWidth;
    output.pos = mul(viewPos, g_ProjMatrix);
#else
    output.pos = mul(input.g_Position, g_WorldViewProjMatrix);
#endif

#ifdef DISTORTION_EFFECT
    output.v_ProjPos = output.pos;
#else
#ifdef DEPTH
    output.v_ProjPos = output.pos;
#endif
#endif

    return output;
}
