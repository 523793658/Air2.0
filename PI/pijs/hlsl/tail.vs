struct VertexShaderInput
{
    float4 g_Position : POSITION;
    float g_TexCoord1 : TEXCOORD1;
#ifdef FACING_CAMERA
    float3 g_TexCoord2 : TEXCOORD2;
#endif
    float g_TexCoord3 : TEXCOORD3;
};

#ifdef TEXTURE
#ifdef TILED
float4 u_TextureTileInfo ;     //xyΪTile����zΪ��������
float2 u_PrivateInfo ;
#endif
#endif

#ifdef FACING_CAMERA
float4x4 g_WorldViewMatrix ;
float4x4 g_ProjMatrix ;
float u_Width ;
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

    float v_TailStrength : TEXCOORD4;
};

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    float indexMod = fmod(input.g_TexCoord1, 2.0);
    output.v_TailStrength = input.g_TexCoord3;

#ifdef TEXTURE
    float2 texCoord = float2(1.0, 1.0) - float2(indexMod, output.v_TailStrength);
#ifdef TILED
    float tileCount = u_TextureTileInfo.x * u_TextureTileInfo.y;
    float tileIndex = fmod(floor(u_PrivateInfo.x / u_TextureTileInfo.z + u_PrivateInfo.y), tileCount);

    output.v_TexCoord0.x = (texCoord.x + fmod(tileIndex, u_TextureTileInfo.x)) / u_TextureTileInfo.x;
    output.v_TexCoord0.y = (texCoord.y + floor(tileIndex / u_TextureTileInfo.x)) / u_TextureTileInfo.y;

#ifdef BLEND
    float nextTileIndex = fmod(tileIndex + 1.0, tileCount);
    output.v_TexCoord1.x = (texCoord.x + fmod(nextTileIndex, u_TextureTileInfo.x)) / u_TextureTileInfo.x;
    output.v_TexCoord1.y = (texCoord.y + floor(nextTileIndex / u_TextureTileInfo.x)) / u_TextureTileInfo.y;
    output.v_Factor = fmod(u_PrivateInfo.x + u_PrivateInfo.y * u_TextureTileInfo.z, u_TextureTileInfo.z) / u_TextureTileInfo.z;
#endif
#else
    output.v_TexCoord0 = texCoord;
#endif
#endif

#ifdef FACING_CAMERA
    float4 vPos = mul(input.g_Position, g_WorldViewMatrix);
    float offset = (indexMod - 0.5) * u_Width;
    float2 dir = mul(float4(normalize(input.g_TexCoord2), 0.0), g_WorldViewMatrix).xy;
    dir = normalize(dir).yx;
    dir.x = -dir.x;
    vPos.xy += offset * dir;
    output.pos = mul(vPos, g_ProjMatrix);
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
