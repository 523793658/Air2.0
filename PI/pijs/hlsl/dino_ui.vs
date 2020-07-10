struct VertexShaderInput
{
float4 g_Position : POSITION0;
float2 g_TexCoord0 : TEXCOORD0;
#ifdef VERTEX_COLOR
float4 g_Color : COLOR0;
#endif

};

struct VertexShaderOutput
{
	
float2 v_TexCoord0 : TEXCOORD0;
	float4 v_ProjPos : POSITION;
#ifdef MASK_MAP
	float2 v_MaskTexCoord : TEXCOORD1;
#endif

#ifdef VERTEX_COLOR
    float4 v_Color : COLOR0;
#endif

#ifdef TILED
#ifdef BLEND
	float3 nextTileInfo : TEXCOORD2;
#endif
#endif
};


float4x4 g_WorldViewProjMatrix ;

#ifdef UV_MATRIX
	float4x4 u_uv_Matrix ;
#endif

#ifdef UV_MIRROR
	float4 uv_MirrorFactor ;
#endif

#ifdef MASK_MAP
	float4x4 u_MaskUVMatrix ;
#endif

#ifdef TILED
	float4 u_TextureTileInfo ;     //x行y列，每帧z秒
	float u_Time ;
#endif


float mod(float x, float y)
{
    float r = x - y * floor(x / y );
    return r;
}


VertexShaderOutput main( VertexShaderInput input )
{
	VertexShaderOutput output;
	float4 texCoord = float4(input.g_TexCoord0, 1.0, 1.0);

	#ifdef UV_MIRROR_X
		texCoord.x = 1.0 - texCoord.x;
	#endif

	#ifdef UV_MIRROR_Y
		texCoord.y = 1.0 - texCoord.y;
	#endif

	#ifdef TILED
        float tileCount = u_TextureTileInfo.x * u_TextureTileInfo.y;
        float tileIndex = mod(floor(u_Time / u_TextureTileInfo.z + u_TextureTileInfo.w), tileCount);
		float4 currentTileCoord = float4(1.0, 1.0, 1.0, 1.0);
        currentTileCoord.x = (texCoord.x + mod(tileIndex, u_TextureTileInfo.x)) / u_TextureTileInfo.x;
        currentTileCoord.y = (texCoord.y + floor(tileIndex / u_TextureTileInfo.x)) / u_TextureTileInfo.y;

    #ifdef BLEND
        float nextTileIndex = mod(tileIndex + 1.0, tileCount);
        float4 nextTileCoord = float4(1.0, 1.0, 1.0, 1.0);
        nextTileCoord.x = (texCoord.x + mod(nextTileIndex, u_TextureTileInfo.x)) / u_TextureTileInfo.x;
        nextTileCoord.y = (texCoord.y + floor(nextTileIndex / u_TextureTileInfo.x)) / u_TextureTileInfo.y;
        float blendFactor = mod(u_Time + u_TextureTileInfo.w * u_TextureTileInfo.z, u_TextureTileInfo.z) / u_TextureTileInfo.z;
    #endif
        texCoord = currentTileCoord;
    #endif

	#ifndef UVANIMATION
		#ifdef UV_MATRIX
			output.v_TexCoord0 = mul(texCoord, u_uv_Matrix).xy;

			#ifdef TILED
			#ifdef BLEND
			output.nextTileInfo.xy = mul(nextTileCoord, u_uv_Matrix).xy;
			output.nextTileInfo.z = blendFactor;
			#endif
			#endif
		#else
			#ifdef TILED
            #ifdef BLEND
                output.nextTileInfo.xy = nextTileCoord.xy;
                output.nextTileInfo.z = blendFactor;
            #endif
            #endif
            output.v_TexCoord0 = texCoord;
		#endif
	#else
		#ifdef TILED
        #ifdef BLEND
            output.nextTileInfo.xy = nextTileCoord.xy;
            output.nextTileInfo.z = blendFactor;
        #endif
        #endif
        output.v_TexCoord0 = texCoord;
	#endif

	#ifdef MASK_MAP
        output.v_MaskTexCoord = mul(texCoord, u_MaskUVMatrix ).xy;
    #endif


    output.v_ProjPos = mul(input.g_Position, g_WorldViewProjMatrix);

    #ifdef VERTEX_COLOR
        output.v_Color = input.g_Color;
    #endif

    return output;
}