#define PIInv 0.31830988
float3 sRGBToRGB(float3 color) {
	return pow(color, float3(2.2, 2.2, 2.2));
}
struct PixelShaderInput
{
    float4 v_WorldPos : TEXCOORD0;
    float4 v_ViewPos : TEXCOORD1;
    float4 v_ProjPos : TEXCOORD2;
    float3 v_VertexNormal : TEXCOORD3;
};
struct PixelShaderOutput
{
    float4 color : COLOR0;
    float4 depth : COLOR1;
};

float4 textureCylinder(sampler2D tex, float3 dir) {
    float4 texCoord;
    dir.x += 0.0001;
    texCoord.x = acos(normalize(dir.xz).x) * PIInv * sign(dir.z);
    texCoord.x = texCoord.x * 0.5 + 0.5;
    texCoord.y = acos(dir.y) * PIInv;
    texCoord.z = 0.0;
    texCoord.w = 0.0;
    float4 color = tex2Dlod(tex, texCoord);
    return color;
}

float3 g_ViewPosition;

#ifdef ENVIRONMENT_MAPPING
    #ifdef CYLINDER_MAPPING
        sampler2D u_EnvironmentMap ;
    #else
        samplerCUBE u_EnvironmentMap ;
    #endif
#endif
float4 u_Params[4];
//结构如下：
//float3 fog_color;
//float fog_density;
//float3 sky_color;
//float shininess;
//float wave_dir[2];
//float wave_speed;
//float wave_scale;
//float wave_density;
//float refractive_density;
//float refractive_index;
//float caustics_density;
//

sampler2D u_SceneColorTex ;
sampler2D u_SceneDepthTex ;

#ifdef REFLECTION
    sampler2D u_ReflectionTex ;
    sampler2D u_ReflectionDepthTex;
    float4x4 u_ReflectionMatrix ;
#endif


PixelShaderOutput main(PixelShaderInput input)
{
    float3 worldViewDir = normalize(input.v_WorldPos.xyz - g_ViewPosition);
    float2 sceneCoord = (input.v_ProjPos.xy / input.v_ProjPos.w + 1.0) * 0.5;
    sceneCoord.y = 1.0 - sceneCoord.y;
    float srcSceneDepth = tex2D(u_SceneDepthTex, sceneCoord).x;
    clip(srcSceneDepth - input.v_ProjPos.z / input.v_ProjPos.w);
    PixelShaderOutput output;
    float3 sceneColor = tex2D(u_SceneColorTex, sceneCoord).rgb * u_Params[0].xyz;

    #ifdef REFLECTION
        float4 reflectionCoord = mul(input.v_WorldPos, u_ReflectionMatrix);
        reflectionCoord.xy /= reflectionCoord.w;
        reflectionCoord.xy = (reflectionCoord.xy + 1.0) * 0.5;
        reflectionCoord.y = 1.0 - reflectionCoord.y;
        float4 reflectionColor = tex2D(u_ReflectionTex, reflectionCoord.xy);
        #ifdef ENVIRONMENT_MAPPING
            float3 refNor = reflect(worldViewDir, input.v_VertexNormal);
            #ifdef CYLINDER_MAPPING
                float3 skyColor = textureCylinder(u_EnvironmentMap, refNor).xyz;
            #else
                float3 skyColor = texCUBE(u_EnvironmentMap, refNor).xyz;
            #endif
            skyColor.xyz = sRGBToRGB(skyColor.xyz);
        #else
            float3 skyColor = u_Params[1].xyz;
        #endif
        reflectionColor.rgb = lerp(skyColor, reflectionColor.rgb, reflectionColor.a);

        float flag = abs(dot(worldViewDir, input.v_VertexNormal));
        output.color = float4(lerp(reflectionColor.rgb, sceneColor,(1.0 - u_Params[1].w / 255.0) * flag), 1.0);
    #else
        output.color = float4(sceneColor, 1.0);
    #endif

    float depth = input.v_ProjPos.z / input.v_ProjPos.w;
    output.depth = float4(depth, depth, depth, 1.0);
    return output;
} 