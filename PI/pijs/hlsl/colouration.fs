#define ONE_THIRD  1.0/3.0
#define ONE_SIXTH  1.0/6.0
#define TWO_THIRD  2.0/3.0
#define HSLMAX 1.0
#define RGBMAX 1.0

#define ZERO 0.001
struct PixelShaderInput
{
    float4 pos : POSITION;
    float2 v_TexCoord0 : TEXCOORD0;
#ifdef DEPTH
    float2 v_Depth : TEXCOORD1;
#endif
#ifdef VERTEX_COLOR
    float4 v_Color : COLOR0;
#endif
};

#ifdef COLOR
float4 u_Color ;
#else
sampler2D u_DiffuseMap ;
#endif

#ifdef COLOURATION
    float3 u_HLSValue ;
#endif


struct PixelShaderOutput
{
    float4 color : COLOR0;
#ifdef DEPTH
    float4 depth : COLOR1;
#endif
};

float mod(float x, float y)
{
    float r = x - y * floor(x / y );
    return r;
}

int mod(int x, int y)
{
    int r = x - y * floor(x / y);
    return r;
}

float hue2rgb(float x, float y, float z)
{
    float p = x, q = y, t = z;
    if (t < 0.0) {
        t += 1.0;
    }
    if (t > 1.0) {
        t -= 1.0;
    }
    if (t < 1.0 / 6.0) {
        return p + (q - p) * 6.0 * t;
    }
    if (t < 1.0 / 2.0) {
        return q;
    }
    if (t < 2.0 / 3.0) {
        return p + (q - p) * (2.0 / 3.0 - t) * 6.0;
    }
    return p;
}


float3 RGBToHSL(float3 rgb)
{

    float h, s, l, maxc, minc, d;
    float r = rgb.r, g = rgb.g, b = rgb.b;
    maxc = max(max(r, g), b);
    minc = min(min(r, g), b);
    l = (maxc + minc) / 2.0;

    if (maxc - minc < 0.00001) {
        h = s = 0.0; // achromatic
    } else {
        d = maxc - minc;
        s = l > 0.5 ? d / (2.0 - maxc - minc) : d / (maxc + minc);
        if(maxc - r < 0.0001)
        {
            h = (g - b) / d + (g < b ? 6.0 : 0.0);
        }
        else if(maxc - g < 0.0001)
        {
            h = (b - r) / d + 2.0;
        }
        else
        {
            h = (r - g) / d + 4.0;
        }
        h /= 6.0;
    }

    return float3(h, s, l);
}

float3 HSLToRGB(float3 hsl)
{
    float h = hsl.x, s = hsl.y, l = hsl.z;
    float r, g, b, q, p;

    if (abs(s) < 0.0001) {
        r = g = b = l; // achromatic
    } else {
        q = (l < 0.5) ? (l * (1.0 + s)) : (l + s - l * s);
        p = 2.0 * l - q;
        r = hue2rgb(p, q, h + 1.0 / 3.0);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1.0 / 3.0);
    }

    return float3(r, g, b);
}

float3 SetBright(float3 rgb, float bValue)
{
    float3 color = rgb;
    if(bValue > 0.0)
    {
        color = color + ( 1.0 - color) * bValue;
    }
    else if(bValue <0.0)
    {
        color = color + color * bValue;
    }
    color = clamp(color, 0.0, 1.0);
    return color;
}
float3 SetHueAndSaturation(float3 rgb, float hValue, float sValue)
{
    float3 hsl = RGBToHSL(rgb);
    hsl.x += hValue;
    hsl.x = mod(hsl.x, 1.0);
    float3 color = HSLToRGB(hsl);
    if(abs(sValue) > 0.0001)
    {
        if(sValue > 0.0)
        {
            sValue = sValue + hsl.y >= 1.0 ? hsl.y : 1.0 - sValue;
            sValue = 1.0 / sValue - 1.0;
        }
        color += ((color-hsl.z)*sValue);
        clamp(color, 0.0, 1.0);
    }
    return color;
}



PixelShaderOutput main(PixelShaderInput input)
{
#ifdef COLOR
    float4 color = u_Color;
#else
    float4 color = tex2D(u_DiffuseMap, input.v_TexCoord0);
#endif

#ifdef VERTEX_COLOR
    color = color * input.v_Color;
#endif

    PixelShaderOutput output;
    #ifdef COLOURATION
        float hValue = u_HLSValue.x;
        float sValue = u_HLSValue.y;
        float bValue = u_HLSValue.z;
        if(sValue > 0.0 && abs(bValue) > 0.0001)
        {
            color.rgb = SetBright(color.rgb, bValue);
        }
        color.rgb = SetHueAndSaturation(color.rgb, hValue, sValue);
        if(abs(bValue) > 0.0001 && sValue <= 0.0)
        {
            color.rgb = SetBright(color.rgb, bValue);
        }
    #endif


    output.color = color;
#ifdef DEPTH
    float depth = input.v_Depth.x / input.v_Depth.y;
    output.depth = float4(depth, depth, depth, depth);
#endif
    return output;
}
