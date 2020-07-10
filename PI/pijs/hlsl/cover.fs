struct PixelShaderInput
{
    float3 v_WorldPos : COLOR0;
    float3 v_VertexNormal : TEXCOORD0;
};
struct PixelShaderOutput
{
	float4 color : COLOR0;
};

float3 g_ViewPosition ;

PixelShaderOutput main(PixelShaderInput input)
{
	PixelShaderOutput output;
    float3 worldViewDir = normalize(input.v_WorldPos - g_ViewPosition);
    float3 worldNormal = normalize(input.v_VertexNormal);
	float factor = abs(dot(worldViewDir, worldNormal));
	if(factor < 0.5){
		output.color = float4(0.063, 0.58, 1.0, 1.0);
	}
	else{
		output.color = float4(0.15, 0.91, 1.0, 0.5);
	}
	return output;
}