#define PI 3.1415926
#ifdef PCF_8X
    #define PRE_PCF_SAMPLES 2
    #define PCF_SAMPLES 2
#endif
#ifdef PCF_16X
    #define PRE_PCF_SAMPLES 2
    #define PCF_SAMPLES 6
#endif
#ifdef PCF_32X
    #define PRE_PCF_SAMPLES 4
    #define PCF_SAMPLES 12
#endif
#ifdef PCF_48X
    #define PRE_PCF_SAMPLES 4
    #define PCF_SAMPLES 20
#endif
#ifdef PCF_64X
    #define PRE_PCF_SAMPLES 4
    #define PCF_SAMPLES 28
#endif
#ifndef PRE_PCF_SAMPLES
    #define PRE_PCF_SAMPLES 0
    #define PCF_SAMPLES 0
#endif

float4 getDecal(in sampler2D decalMap, in float2 decalCoord, in float vDepth, in float zFar)
{
	float2 coord =  decalCoord * float2(0.5, -0.5 ) + 0.5;
    float4 color = tex2D(decalMap, coord);
    float fade = max( clamp( ( 1.0 - length(decalCoord) ) * 5.0, 0.0, 1.0 ), 1.0 - clamp(-vDepth - zFar, 0.0, 1.0) );

    return float4( float3(color.rgb * fade), 1.0 - fade + color.a * fade );
}

float getMipLevel(float2 uv, float2 textureSize, int mipmapCount) 
{
	float2 dx = ddx(uv * textureSize.x);
	float2 dy = ddy(uv * textureSize.y);
	float d = max(dot(dx, dx), dot(dy, dy));

	float rangeClamp = pow(2.0, (float(mipmapCount) - 1.0) * 2.0);
	d = clamp(d, 1.0, rangeClamp);

	float mipLevel = 0.5 * log2(d);

	return mipLevel;
}
float4 sRGBTexture2D(sampler2D tex, float2 texCoord) {
	float4 color = tex2D(tex, texCoord);
	color.xyz = pow(color.xyz, float3(2.2, 2.2, 2.2));
	return color;
}

float4 textureCylinder(sampler2D tex, float3 dir) {

	float2 texCoord;
	texCoord.x = acos(normalize(dir.xz).x) / PI * sign(dir.z);
	texCoord.x = texCoord.x * 0.5 + 0.5;
	texCoord.y = acos(dir.y) / PI;
	//texCoord.y = 1.0 - (dir.y * 0.5 + 0.5);

	float4 color = tex2D(tex, texCoord);

	return color;
}

float4 textureCylinderLod(sampler2D tex, float4 dir) {

	float4 texCoord;
	texCoord.x = acos(normalize(dir.xz).x) / PI * sign(dir.z);
	texCoord.x = texCoord.x * 0.5 + 0.5;
	texCoord.y = acos(dir.y) / PI;
	texCoord.z = 0.0;
	texCoord.w = dir.w;
	float4 color = tex2Dlod(tex, texCoord);
	return color;
}


float getLuminance(float3 rgbColor) {
	return rgbColor.r * 0.299 + rgbColor.g * 0.587 + rgbColor.b * 0.114;
}

float3 sRGBToRGB(float3 color) {
	return pow(color, float3(2.2, 2.2, 2.2));
}

float3 RGBTosRGB(float3 color) {
	return pow(color, float3(0.454545, 0.454545, 0.454545));
}

struct PixelShaderInput
{
	float4 pos : POSITION;

	// DEPTH 对应后两个成员 v_Depth
	float4 v_TexCoord0 : TEXCOORD0;

#ifdef LIGHTMAP
	float2 v_TexCoord1 : TEXCOORD1;
#endif

	// FLOOD 对应diffuse的最后一个分量 v_FloodFactor
	float4 v_Diffuse : COLOR0;

	float4 v_ViewPos : TEXCOORD2;
	float3 v_EnvLightColor : TEXCOORD3;

#ifdef ENVIRONMENT_MAPPING
	float3 v_EnvironmentCoord : TEXCOORD4;
#endif
    float3 v_Ambient : TEXCOORD5;
#ifdef DECAL
	#ifndef DECAL_IGNORE
		float2 v_DecalProjCoord : TEXCOORD6;
	#endif
#endif

#ifdef VERTEX_COLOR
	float4 v_VertexColor : TEXCOORD7;
#endif
};


struct PixelShaderOutput
{
	float4 color : COLOR0;
#ifdef DEPTH
	float4 depth : COLOR1;
#endif
};



float g_Time ;
float u_Glossiness ;

#ifdef ALPHA_MAP
sampler2D u_AlphaMap ;
float g_AlphaCullOff ;

#ifdef ALPHA_UV_ANIM
float2 u_AlphaUVVector ;
#endif
#endif

#ifdef TERRAIN
float2 u_TerrainTexScale ;
#else
#ifndef DIFFUSE_MAP_MATRIX
#ifdef DIFFUSE_UV_ANIM
float2 u_DiffuseUVVector ;
#endif
#endif
#endif

#ifdef DIFFUSE_MAP
sampler2D u_DiffuseMap;
#endif

#ifdef MTRL_SHADOW
float u_MtrlShadow;
#endif

#ifdef GLOW_MAP
sampler2D u_GlowMap ;
float u_GlowScale ;
#ifndef TERRAIN
#ifndef GLOW_MAP_MATRIX
#ifdef GLOW_UV_ANIM
float2 u_GlowUVVector ;
#endif
#endif
#endif
#endif

#ifdef ENVIRONMENT_MAPPING
#ifdef CYLINDER_MAPPING
sampler2D u_EnvironmentMap ;
#else
samplerCUBE u_EnvironmentMap ;
#endif
#endif

#ifdef FADE
float u_FadeProcess ;
#endif

#ifdef LIGHTMAP
sampler2D u_AOMap ;
#endif

#ifdef DECAL
#ifndef DECAL_IGNORE
		sampler2D u_DecalMap ;
		float4 u_DecalZFar ;
#endif
#endif


#ifdef FLOOD2
	float3 u_Flood2Color ;
	float4 u_Flood2 ;
    #ifdef FLOOD_MAP
        sampler2D u_FloodMap2 ;
    #endif
#else
    #ifdef FLOOD
        float u_FloodScale ;
        float3 u_FloodColor ;
        #ifdef FLOOD_TEX
            float2 u_FloodUVVector ;
            sampler2D u_FloodMap ;
        #endif
    #endif
#endif


#ifdef VANISH
float u_VanishSpeed;
float3  u_VanishColor ;
float u_VanishGap ;
float u_VanishPower ;
sampler2D u_VanishMap ;
#endif

float3 u_DiffuseRevise ;
#ifdef REFLECTION_MAP
sampler2D u_ReflectionMap;
#else
float u_Reflection;
#endif


PixelShaderOutput main(PixelShaderInput input)
{
	PixelShaderOutput output;
#ifdef VANISH
	float vanishAlpha = tex2D(u_VanishMap, input.v_TexCoord0.xy).a;
	clip(vanishAlpha - u_VanishSpeed + u_VanishGap);
#endif

	float alpha = 1.0;
#ifdef ALPHA_MAP

	float2 alphaMapTexcoord = input.v_TexCoord0.xy;
#ifdef ALPHA_UV_ANIM
	alphaMapTexcoord += u_AlphaUVVector * g_Time;
#endif
	alpha = tex2D(u_AlphaMap, alphaMapTexcoord).a;
    clip(alpha - g_AlphaCullOff);
#endif
#ifdef TERRAIN
	float2 diffuseTexcoord = input.v_TexCoord0 * u_TerrainTexScale;
#else
#ifdef DIFFUSE_MAP_MATRIX
	float2 diffuseTexcoord = input.v_DiffuseTexcoord;
#else
	float2 diffuseTexcoord = input.v_TexCoord0;
#ifdef DIFFUSE_UV_ANIM
		diffuseTexcoord += u_DiffuseUVVector * g_Time;
#endif
#endif
#endif

	float4 diffuseColor = float4(1.0, 1.0, 1.0, 1.0);
#ifdef DIFFUSE_MAP
		diffuseColor = sRGBTexture2D(u_DiffuseMap, diffuseTexcoord);
#endif
#ifdef VERTEX_COLOR
	diffuseColor *= input.v_VertexColor;
#endif

	diffuseColor.xyz *= u_DiffuseRevise;
	alpha *= diffuseColor.a;
	float3 glowColor = float3(0.0, 0.0, 0.0);


#ifdef GLOW_MAP
#ifdef TERRAIN
	float2 glowMapTexcoord = input.v_TexCoord0 * u_TerrainTexScale;
#else
#ifdef GLOW_MAP_MATRIX
		float2 glowMapTexcoord = input.v_GlowTexcoord;
#else
		float2 glowMapTexcoord = input.v_TexCoord0;
#ifdef GLOW_UV_ANIM
	glowMapTexcoord += u_GlowUVVector * g_Time;
#endif
#endif
#endif
	glowColor = sRGBTexture2D(u_GlowMap, glowMapTexcoord).xyz * u_GlowScale * diffuseColor.a;
#endif

    float shadow = 1.0;


#ifdef MTRL_SHADOW
    float shadowIntensity = u_MtrlShadow * input.v_ViewPos.w;
#else
    #define shadowIntensity input.v_ViewPos.w
#endif

#ifdef LIGHTMAP
    shadow = tex2D(u_AOMap, input.v_TexCoord1).a;
    shadow = saturate(1.0 - shadowIntensity + shadowIntensity * shadow) * 0.8 + 0.2;
#endif

	float glossiness = u_Glossiness;
	float3 ambient = input.v_Ambient.xyz;
	float3 diffuse = input.v_Diffuse.xyz * shadow;

#ifdef DECAL
#ifndef DECAL_IGNORE
	float4 decal = getDecal(u_DecalMap, input.v_DecalProjCoord, input.v_ViewPos.z, u_DecalZFar);
	diffuse += decal.xyz;
	diffuseColor.xyz *= decal.a;
#endif
#endif

	#ifdef REFLECTION_MAP
	    float viewFR = tex2D(u_ReflectionMap, diffuseTexcoord).a;
    #else
        float viewFR = u_Reflection;
	#endif

#ifdef ENVIRONMENT_MAPPING
	float4 refDir;
	refDir.xyz = input.v_EnvironmentCoord;
	float mipmapLevel = max(1.0 - glossiness, 0.0) * 6.0 + getMipLevel(input.v_TexCoord0, float2(512.0, 512.0), 10);
	refDir.w = mipmapLevel;
#ifdef CYLINDER_MAPPING
	float3 environment = textureCylinderLod(u_EnvironmentMap, refDir).xyz;
#else
	float3 environment = texCUBElod(u_EnvironmentMap, refDir).xyz;
#endif
		environment = sRGBToRGB(environment);
		environment *= 0.75 * (0.5 + 0.5) * input.v_EnvLightColor;
    #ifdef BACKLIGHT
        float3 finalColor = diffuseColor.xyz * ambient * shadow;
    #else
        float3 finalColor = diffuseColor.xyz * ambient + diffuseColor.xyz * diffuse * (1.0 - viewFR) + glowColor;
    #endif
#else
    #ifdef BACKLIGHT
        float3 finalColor = diffuseColor.xyz * ambient * shadow;
    #else
	    float3 finalColor = diffuseColor.xyz * ambient + diffuseColor.xyz * diffuse + glowColor;
    #endif
#endif

	output.color = float4(finalColor.xyz, alpha);
	output.color.xyz = RGBTosRGB(output.color.xyz);

#ifdef DEPTH
	float depth = input.v_TexCoord0.z / input.v_TexCoord0.w;
	output.depth = float4(depth, depth, depth, depth);
#endif

#ifdef FLOOD2
	float3 flood2Color = u_Flood2Color;
	#ifdef FLOOD_MAP
		flood2Color *= tex2D(u_FloodMap2, input.v_TexCoord0.xy + u_Flood2.zw * g_Time).xyz;
	#endif
	flood2Color = max(flood2Color, float3(0.1, 0.1, 0.1)) * u_Flood2.x * input.v_Diffuse.w;
	output.color.xyz += flood2Color;
#else
#ifdef FLOOD
	float3 floodColor = u_FloodColor;
#ifdef FLOOD_TEX
	floodColor *= tex2D(u_FloodMap, v_TexCoord0.xy + u_FloodUVVector * g_Time).xyz;
#endif
	floodColor = max(floodColor, float3(0.1, 0.1, 0.1)) * u_FloodScale * input.v_Diffuse.w;
	output.color.xyz += floodColor;
#endif
#endif
#ifdef DISCARDALPHA
    output.color.a = 1.0;
#endif
#ifdef FADE
    output.color.a *= u_FadeProcess;
#endif
#ifdef VANISH
	if (vanishAlpha < u_VanishSpeed)
    	{
    		output.color.rgb *= float3(u_VanishColor * u_VanishPower);
    	}
#endif
	return output;
}