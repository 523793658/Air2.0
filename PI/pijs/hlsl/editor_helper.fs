struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
#ifdef DEPTH
    float2 v_Depth : TEXCOORD1;
#endif
#ifdef VERTEX_COLOR
    float4 v_Color : COLOR;
#endif
};

struct PixelShaderOutput
{
	float4 color : COLOR0;
};

float mod(float x, float y)
{
    float r = x - y * floor(x / y );
    return r;
}

sampler2D u_ColorTex ;
sampler2D u_DepthTex ;

#ifdef ENABLE

    float4x4 u_ViewProjMatrixInverse ;

    #ifdef PROJECTION_ENABLE
        sampler2D u_ProjectionTexture ;
        float2 u_ProjectionOrigin ;
        float2 u_ProjectionSize ;
        float u_ProjectionFactor ;
    #endif

    #ifdef GRID_ENABLE
        float u_GridSnap ;
        float u_GridLineWidth ;
        float u_GridFactor ;
    #endif

    #ifdef BRUSH_ENABLE
        float2 u_BrushPos ;
        float u_BrushSize ;
    #endif

    #ifdef DRAW_CIRCLE_ENABLE
        int u_ShapeCircleNum ;
        float2 u_ShapeCirclePos[10] ;
        float u_ShapeCircleRadius[10] ;
    #endif

    #ifdef DRAW_RECTANGLE_ENABLE
        int u_ShapeRectangleNum ;
        float2 u_ShapeRectanglePos[10] ;
        float3x3 u_ShapeRectangleRotationMat[10] ;
        float u_ShapeRectangleWidth[10] ;
        float u_ShapeRectangleHeight[10] ;
    #endif

    #ifdef DRAW_SECTOR_ENABLE
        int u_ShapeSectorNum ;
        float2 u_ShapeSectorPos[10] ;
        float2 u_ShapeSectorDir[10] ;
        float u_ShapeSectorRadius[10] ;
        float u_ShapeSectorAngle[10] ;
    #endif

#endif

 PixelShaderOutput main(PixelShaderInput input)
{
    float4 color = tex2D(u_ColorTex, input.v_TexCoord0);
	PixelShaderOutput output;
    #ifdef ENABLE

        float depth = tex2D(u_DepthTex, input.v_TexCoord0).x;

        if(depth < 1.0)
        {
            float4 projPos = float4((input.v_TexCoord0.x - 0.5) * 2.0, (0.5 - input.v_TexCoord0.y) * 2.0, depth, 1.0);
            float4 worldPos = mul(projPos , u_ViewProjMatrixInverse);
            worldPos /= worldPos.w;

            #ifdef PROJECTION_ENABLE
                float2 projectionTexCoord = (worldPos.xz - u_ProjectionOrigin) / u_ProjectionSize;
                if(projectionTexCoord.x <= 1.0 && projectionTexCoord.x >= 0.0 && projectionTexCoord.y <= 1.0 && projectionTexCoord.y >= 0.0)
                {
                    float4 rojectionTexColor = tex2D(u_ProjectionTexture, projectionTexCoord);
                    float projectionFactor = u_ProjectionFactor * rojectionTexColor.a;
                    color.xyz = color.xyz * (1.0 - projectionFactor) + rojectionTexColor.xyz * projectionFactor;
                }
            #endif

            #ifdef GRID_ENABLE
                float offsetX = mod(worldPos.x, u_GridSnap);
                float offsetZ = mod(worldPos.z, u_GridSnap);
                float inv = u_GridSnap - u_GridLineWidth;
                if(offsetX <= u_GridLineWidth || offsetX >= inv || offsetZ <= u_GridLineWidth || offsetZ >= inv)
                {
                    #ifdef INVERT
                        color.xyz = float3(1.0, 1.0, 1.0) - color.xyz;
                    #else
                        color.xyz = color.xyz * (1.0 - u_GridFactor) + float3(u_GridFactor, u_GridFactor, u_GridFactor);
                    #endif
                }
            #endif

            #ifdef BRUSH_ENABLE
                float brushDepth = tex2D(u_DepthTex, u_BrushPos).x ;
                float4 brushProjPos = float4((u_BrushPos.x - 0.5) * 2.0, (0.5 - u_BrushPos.y) * 2.0, brushDepth, 1.0);
                float4 brushWorldPos = mul(brushProjPos, u_ViewProjMatrixInverse);
                brushWorldPos /= brushWorldPos.w;

                float dist = distance(worldPos.xz, brushWorldPos.xz);
                if(dist < u_BrushSize && dist > u_BrushSize - 0.05)
                {
                    color.xyz = float3(0.0, 0.7, 0.0);
                }
            #endif
            #ifdef DRAW_SHAPE_ENABLE
                #ifdef DRAW_CIRCLE_ENABLE
                    for(int i = 0; i < u_ShapeCircleNum; ++i)
                    {
                        float dist = distance(worldPos.xz, u_ShapeCirclePos[i].xy);
                        if(dist < u_ShapeCircleRadius[i] && dist > u_ShapeCircleRadius[i] - 0.2)
                        {
                            color.xyz = float3(0.0, 0.7, 0.0);
                            break;
                        }
                    }
                #endif

                #ifdef DRAW_RECTANGLE_ENABLE
                    for(int i = 0; i < u_ShapeRectangleNum; ++i)
                    {
                        float edgeSize = 0.2f;

                        float width = u_ShapeRectangleWidth[i];
                        float height = u_ShapeRectangleHeight[i];
                        float2 pos = mul(float3(worldPos.x - u_ShapeRectanglePos[i].x, worldPos.z - u_ShapeRectanglePos[i].y , 0), u_ShapeRectangleRotationMat[i]).xy;
                        if(pos.x < width / 2 && pos.x > - width / 2 && pos.y < height / 2 && pos.y > -height / 2 &&(pos.x > width / 2 - edgeSize ||pos.x < -width / 2 + edgeSize ||pos.y > height / 2 - edgeSize ||pos.y < -height / 2 + edgeSize))
                        {
                            color.xyz = float3(0.0, 0.7, 0.0);
                            break;
                        }
                    }
                #endif

                #ifdef DRAW_SECTOR_ENABLE
                    for(int i = 0; i < u_ShapeSectorNum; ++i)
                    {
                        float angleSize = 0.017f;
                        float edgeSize = 0.2f;
                        float dist = distance(worldPos.xz, u_ShapeSectorPos[i].xy);

                        float2 worldDir = worldPos.xz - u_ShapeSectorPos[i].xy;
                        float angle = /*degrees(*/acos(dot(u_ShapeSectorDir[i], worldDir)/ length(u_ShapeSectorDir[i] * length(worldDir)))/*)*/;
                        if(dist < u_ShapeSectorRadius[i] && dist > u_ShapeSectorRadius[i] - edgeSize && angle < u_ShapeSectorAngle[i] || dist < u_ShapeSectorRadius[i] && angle < u_ShapeSectorAngle[i] + angleSize && angle > u_ShapeSectorAngle[i] - angleSize)
                        {
                            color.xyz = float3(0.0, 0.7, 0.0);
                            break;
                        }
                    }
                #endif
            #endif
        }
        float v = (1.0 - depth) * 100;
       // color = float4(v, v,v, 1.0);
    #endif

    output.color = color;
    return output;
}