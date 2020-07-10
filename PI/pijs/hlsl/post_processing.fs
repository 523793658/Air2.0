#define FXAA_EDGE_THRESHOLD 0.25
#define FXAA_EDGE_THRESHOLD_MIN 0.0833333
#define FXAA_SUBPIX_TRIM 0.25
#define FXAA_SUBPIX_TRIM_SCALE 1.33333
#define FXAA_SUBPIX_CAP 0.6666
#define FXAA_SEARCH_THRESHOLD 0.25
#define FXAA_SEARCH_STEPS 4


//Uniforms=======================================================================
sampler2D u_SceneColorTex ;
sampler2D u_SceneDepthTex ;
sampler2D u_BloomTex;
#ifdef COLOR_GRADING
sampler3D u_CLUTTex;
#endif
float4 u_Params[3];
int4 u_Switchs;

//structures======================================================================
struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
};
struct PixelShaderOutput
{
    float4 color : COLOR0;
};

//functions=====================================================================================
float fxaaLuminance(float3 color) {
    return 0.299*color.r + 0.587*color.g + 0.114*color.b;
}

float4 fxaaLerp(float4 a, float4 b, float amountOfA) {
    return (float4(-amountOfA, -amountOfA, -amountOfA, -amountOfA) * b) + ((a * float4(amountOfA, amountOfA, amountOfA, amountOfA)) + b);
}



float4 tex2DLodWrap(sampler2D s, float2 coord, float lod)
{
    float4 r;
    float4 t = float4(coord, 0.0, lod);
    r = tex2Dlod(s, t);
    return r;
}



float4 fxaa(sampler2D sceneColorTex, float4 texCoord, float2 offset)
{
    float4 color = tex2Dlod(sceneColorTex, texCoord);

    float4 colorN = tex2Dlod(sceneColorTex, texCoord + float4(0, 1, 0, 0) * offset.xyxx);
    float4 colorW = tex2Dlod(sceneColorTex, texCoord + float4(-1, 0, 0, 0) * offset.xyxx);
    float4 colorE = tex2Dlod(sceneColorTex, texCoord + float4(1, 0, 0, 0) * offset.xyxx);
    float4 colorS = tex2Dlod(sceneColorTex, texCoord + float4(0, -1, 0, 0) * offset.xyxx);

    float lumaM = fxaaLuminance(color.xyz);
    float lumaN = fxaaLuminance(colorN.xyz);
    float lumaW = fxaaLuminance(colorW.xyz);
    float lumaE = fxaaLuminance(colorE.xyz);
    float lumaS = fxaaLuminance(colorS.xyz);

    float rangeMin = min(lumaM, min(min(lumaN, lumaW), min(lumaS, lumaE)));
    float rangeMax = max(lumaM, max(max(lumaN, lumaW), max(lumaS, lumaE)));
    float range = rangeMax - rangeMin;
    if(range < max(FXAA_EDGE_THRESHOLD_MIN, rangeMax * FXAA_EDGE_THRESHOLD)) {
        return color;
    }
    else {
        float lumaL = (lumaN + lumaW + lumaE + lumaS) * 0.25;
        float rangeL = abs(lumaL - lumaM);
        float blendL = max(0.0, (rangeL / range) - FXAA_SUBPIX_TRIM) * FXAA_SUBPIX_TRIM_SCALE;
        blendL = min(FXAA_SUBPIX_CAP, blendL);
        float4 colorNW = tex2Dlod(sceneColorTex, texCoord + float4(-1, 1, 0, 0) * offset.xyxx);
        float4 colorNE = tex2Dlod(sceneColorTex, texCoord + float4(1, 1, 0, 0) * offset.xyxx);
        float4 colorSW = tex2Dlod(sceneColorTex, texCoord + float4(-1, -1, 0, 0) * offset.xyxx);
        float4 colorSE = tex2Dlod(sceneColorTex, texCoord + float4(1, -1, 0, 0) * offset.xyxx);

        float4 colorL = (colorNW + colorNE + colorSW + colorSE + colorN + colorW + colorE + colorS + color) / 9.0;

        float lumaNW = fxaaLuminance(colorNW.xyz);
        float lumaNE = fxaaLuminance(colorNE.xyz);
        float lumaSW = fxaaLuminance(colorSW.xyz);
        float lumaSE = fxaaLuminance(colorSE.xyz);

        float edgeVert =
                abs((0.25 * lumaNW) + (-0.5 * lumaN) + (0.25 * lumaNE)) +
                abs((0.50 * lumaW ) + (-1.0 * lumaM) + (0.50 * lumaE )) +
                abs((0.25 * lumaSW) + (-0.5 * lumaS) + (0.25 * lumaSE));
        float edgeHorz =
                abs((0.25 * lumaNW) + (-0.5 * lumaW) + (0.25 * lumaSW)) +
                abs((0.50 * lumaN ) + (-1.0 * lumaM) + (0.50 * lumaS )) +
                abs((0.25 * lumaNE) + (-0.5 * lumaE) + (0.25 * lumaSE));
        float horzSpan = step(edgeVert, edgeHorz);

        float lengthSign = -(-offset.y * horzSpan + offset.x * (1.0 - horzSpan));
        lumaN = lumaW * (1.0 - horzSpan) + lumaN * horzSpan;
        lumaS = lumaE * (1.0 - horzSpan) + lumaS * horzSpan;
        float gradientN = abs(lumaN - lumaM);
        float gradientS = abs(lumaS - lumaM);
        lumaN = (lumaN + lumaM) * 0.5;
        lumaS = (lumaS + lumaM) * 0.5;
        float pairN = step(gradientS, gradientN);

        lumaN = lumaS * (1.0 - pairN) + lumaN * pairN;
        gradientN = gradientS * (1.0 - pairN) + gradientN * pairN;
        lengthSign *= ((pairN - 0.5) * 2.0);
        float2 posN;
        posN.x = texCoord.x + lengthSign * 0.5 * (1.0 - horzSpan);
        posN.y = texCoord.y + lengthSign * 0.5 * horzSpan;
        gradientN *= FXAA_SEARCH_THRESHOLD;
        float2 posP = posN;
        float2 offNP = float2(offset.x, 0.0) * horzSpan + float2(0.0, offset.y) * (1.0 - horzSpan);
        float lumaEndN = lumaN;
        float lumaEndP = lumaN;
        bool doneN = false;
        bool doneP = false;
        posN += offNP * float2(-1.0, -1.0);
        posP += offNP * float2( 1.0,  1.0);
        for(int i = 0; i < FXAA_SEARCH_STEPS; i++) {
            if(!doneN) {
                lumaEndN = fxaaLuminance(tex2DLodWrap(sceneColorTex, posN.xy, 0.0).xyz);
            }
            if(!doneP) {
                lumaEndP = fxaaLuminance(tex2DLodWrap(sceneColorTex, posP.xy, 0.0).xyz);
            }
            doneN = doneN || (abs(lumaEndN - lumaN) >= gradientN);
            doneP = doneP || (abs(lumaEndP - lumaN) >= gradientN);
            if(doneN && doneP) break;
            if(!doneN) posN -= offNP;
            if(!doneP) posP += offNP;
        }
        float dstN = (texCoord.x - posN.x) * horzSpan + (texCoord.y - posN.y) * (1.0 - horzSpan);
        float dstP = (posP.x - texCoord.x) * horzSpan + (posP.y - texCoord.y) * (1.0 - horzSpan);
        float directionN = step(dstN, dstP);
        lumaEndN = lumaEndN * directionN + lumaEndP * (1.0 - directionN);

        if(((lumaM - lumaN) < 0.0) == ((lumaEndN - lumaN) < 0.0)) {
            lengthSign = 0.0;
        }

        float spanLength = (dstP + dstN);
        dstN = dstN * directionN + dstP * (1.0 - directionN);
        float subPixelOffset = (0.5 + (dstN * (-1.0 / spanLength))) * lengthSign;
        float4 colorF = tex2DLodWrap(sceneColorTex, float2(texCoord.x + (subPixelOffset * (1.0 - horzSpan)), texCoord.y + (subPixelOffset * horzSpan)), 0.0);
        float4 finalColor = fxaaLerp(colorL, colorF, blendL);
        return finalColor;
    }
}

PixelShaderOutput main(PixelShaderInput input)
{
    float4 sceneColor = tex2D(u_SceneColorTex, input.v_TexCoord0);
    //fxaa抗锯齿
    if(u_Switchs.x == 1)
    {
        sceneColor = fxaa(u_SceneColorTex, float4(input.v_TexCoord0.x, input.v_TexCoord0.y, 0, 0), u_Params[2].xy);
    }
    if(u_Switchs.y == 1)
    {
        float4 bloom = tex2D(u_BloomTex, input.v_TexCoord0);
        sceneColor = saturate(sceneColor + bloom);
    }

    #ifdef COLOR_GRADING
        sceneColor.rgb = tex3D(u_CLUTTex, sceneColor.rgb).rgb;
    #endif


    PixelShaderOutput output;
    output.color = sceneColor;
    return output;
}
