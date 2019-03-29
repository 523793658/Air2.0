float4 HdrCustomResolveVS(uint id : SV_VertexID) : SV_POSITION
{
	int x = id & 1;
	int y = id >> 1;
	return float4(x * 4 - 1, y * 4 - 1, 0, 1);
}

float4 encode(float4 color)
{
	return float4(color.rgb * rcp(color.r * 0.299 + color.g * 0.587 + color.b * 0.114 + 1.0), color.a);
}

float4 decode(float4 color)
{
	return float4(color.rgb * rcp(color.r * (-0.299) + color.g * (-0.587) + color.b * (-0.114) + 1.0), color.a);
}


#if HDR_CUSTOM_RESOLVE_2X
Texture2DMS<float4, 2> tex;
float4 HdrCustomResolve2xPS(float4 pos : SV_POSITION) : SV_Target0
{
	uint2 p = uint2(pos.xy);
	float4 c0 = encode(tex.Load(p, 0));
	float4 c1 = encode(tex.Load(p, 1));
	return decode(c0 * 0.5 + c1 * 0.5);
}
#endif

#if HDR_CUSTOM_RESOLVE_4X
Texture2DMS<float4, 4> tex;
float4 HdrCustomResolve4xPS(float4 pos : SV_POSITION) : SV_Target0
{
	uint2 p = uint2(pos.xy);
	float4 c0 = encode(tex.Load(p, 0));
	float4 c1 = encode(tex.Load(p, 1));
	float4 c2 = encode(tex.Load(p, 2));
	float4 c3 = encode(tex.Load(p, 3));

	return decode(c0 * 0.25 + c1 * 0.25 + c2 * 0.25 + c3 * 0.25);
}
#endif

#if HDR_CUSTOM_RESOLVE_8X
Texture2DMS<float4, 8> tex;
float4 HdrCustomResolve8xPS(float4 pos : SV_POSITION) : SV_Target0
{
	uint2 p = uint2(pos.xy);
	float4 c0 = encode(tex.Load(p, 0));
	float4 c1 = encode(tex.Load(p, 1));
	float4 c2 = encode(tex.Load(p, 2));
	float4 c3 = encode(tex.Load(p, 3));
	float4 c4 = encode(tex.Load(p, 4));
	float4 c5 = encode(tex.Load(p, 5));
	float4 c6 = encode(tex.Load(p, 6));
	float4 c7 = encode(tex.Load(p, 7));

	return decode(c0 * 0.125 + c1 * 0.125 + c2 * 0.125 + c3 * 0.125 + c4 * 0.125+ c5 * 0.125 + c6*0.125 + c7 * 0.125);
}
#endif

