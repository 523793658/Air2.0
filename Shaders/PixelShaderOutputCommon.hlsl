#ifndef PIXELSHADEROUTPUT_INTERPOLANTS
#define PIXELSHADEROUTPUT_INTERPOLANTS 0
#endif

#ifndef PIXELSHADEROUTPUT_BASEPASS
#define PIXELSHADEROUTPUT_BASEPASS 0
#endif

#ifndef PIXELSHADEROUTPUT_MRT0
#define PIXELSHADEROUTPUT_MRT0 0
#endif

#ifndef PIXELSHADEROUTPUT_MRT1
#define PIXELSHADEROUTPUT_MRT1 0
#endif

#ifndef PIXELSHADEROUTPUT_MRT2
#define PIXELSHADEROUTPUT_MRT2 0
#endif

#ifndef PIXELSHADEROUTPUT_MRT3
#define PIXELSHADEROUTPUT_MRT3 0
#endif

#ifndef PIXELSHADEROUTPUT_MRT4
#define PIXELSHADEROUTPUT_MRT4 0
#endif

#ifndef PIXELSHADEROUTPUT_MRT5
#define PIXELSHADEROUTPUT_MRT5 0
#endif

#ifndef PIXELSHADEROUTPUT_MRT6
#define PIXELSHADEROUTPUT_MRT6 0
#endif

#ifndef PIXELSHADEROUTPUT_MRT7
#define PIXELSHADEROUTPUT_MRT7 0
#endif

#ifndef PIXELSHADEROUTPUT_COVERAGE
#define PIXELSHADEROUTPUT_COVERAGE 0
#endif

#ifndef PIXELSHADEROUTPUT_A2C
#define PIXELSHADEROUTPUT_A2C 0
#endif

#define OUTPUT_PIXEL_DEPTH_OFFSET (WANT_PIXEL_DEPTH_OFFSET &&(MATERIALBLENDING_SOLID || MATERIALBLENDING_MASKED) && !ES2_PROFILE)

#define SUPPORTS_CONSERVATIVE_DEPTH_WRITES ((COMPILER_HLSL && FEATURE_LEVEL >= FEATURE_LEVEL_SM5) || (PS4_PROFILE))

#define USE_CONSERVATIVE_DEPTH_WRITES (OUTPUT_PIXEL_DEPTH_OFFSET && SUPPORTS_CONSERVATIVE_DEPTH_WRITES)

#if USE_CONSERVATIVE_DEPTH_WRITES

#if COMPILER_HLSL
	#define INPUT_POSITION_QUALIFIERS linear noperspective centroid
	#define DEPTH_WRITE_SEMANTIC SV_DepthLessEqual
#elif PS_PROFILE
	#define INPUT_POSITION_QUALIFIERS
	#define DEPTH_WRITE_SEMANTIC S_DEPTH_LE_OUTPUT
#else
	#error USE_CONSERVATIVE_DEPTH_WRITES enabled for unsupported platform
#endif
#else
	#define INPUT_POSITION_QUALIFIERS
	#define DEPTH_WRITE_SEMANTIC SV_DEPTH
#endif

void MainPS
	(
#if PIXELSHADEROUTPUT_INTERPOLANTS || PIXELSHADEROUTPUT_BASEPASS
		VertexFactoryInterpolantsVSToPS interpolants,
#endif
#if PIXELSHADEROUTPUT_BASEPASS
		BasePassInterpolantsVSToPS basePassInterpolants,
#endif
		in INPUT_POSITION_QUALIFIERS float4 svPosition : SV_Position
		OPTIONAL_IsFrontFace
#if PIXELSHADEROUTPUT_MRT0
		, out float4 outTarget0 : SV_Target0
#endif
#if PIXELSHADEROUTPUT_MRT1
		, out float4 outTarget1 : SV_Target1
#endif
#if PIXELSHADEROUTPUT_MRT2
		, out float4 outTarget2 : SV_Target2
#endif

#if PIXELSHADEROUTPUT_MRT3
		, out float4 outTarget3 : SV_Target3
#endif

#if PIXELSHADEROUTPUT_MRT4
		, out float4 outTarget4 : SV_Target4
#endif
#if PIXELSHADEROUTPUT_MRT5
		, out float4 outTarget5 : SV_Target5
#endif

#if PIXELSHADEROUTPUT_MRT6
		, out float4 outTarget6 : SV_Target6
#endif
#if PIXELSHADEROUTPUT_MRT7
		, out float4 outTarget7 : SV_Target7
#endif
#if PIXELSHADEROUTPUT_COVERAGE || PIXELSHADEROUTPUT_A2C
#if PIXELSHADEROUTPUT_A2C
		, in uint inCoverage : SV_Coverage
#endif
		, out uint OutCoverage : SV_Coverage
#endif
)
{
		PixelShaderIn pixelShaderIn = (PixelShaderIn)0;
		PixelShaderOut pixelShaderOut = (PixelShaderOut)0;
		
#if PIXELSHADEROUTPUT_COVERAGE || PIXELSHADEROUTPUT_A2C
#if PIXELSHADEROUTPUT_A2C
	pixelShaderIn.Coverage = inCoverage;
#else 
	pixelShaderIn.Coverage = 0xF;
#endif
	pixelShaderOut.Coverage = pixelShaderIn.Coverage;
#endif
	pixelShaderIn.SvPosition = svPosition;
	pixelShaderIn.bIsFrontFace = bIsFrontFace;
	
#if PIXELSHADEROUTPUT_BASEPASS
	PixelShaderInOut_MainPS(interpolants, basePassInterpolants, pixelShaderIn, pixelShaderOut);
#elif PIXELSHADEROUTPUT_INTERPOLANTS
	PixelShaderInOut_MainPS(interpolants, pixelShaderIn, pixelShaderOut);
#else
	PixelShaderInOut_MainPS(pixelShaderIn, pixelShaderOut);
#endif

#if PIXELSHADEROUTPUT_MRT0
	outTarget0 = pixelShaderOut.MRT[0];
#endif

#if PIXELSHADEROUTPUT_MRT1
	outTarget1 = pixelShaderOut.MRT[1];
#endif

#if PIXELSHADEROUTPUT_MRT2
	outTarget2 = pixelShaderOut.MRT[2];
#endif

#if PIXELSHADEROUTPUT_MRT3
	outTarget3 = pixelShaderOut.MRT[3];
#endif

#if PIXELSHADEROUTPUT_MRT4
	outTarget4 = pixelShaderOut.MRT[4];
#endif

#if PIXELSHADEROUTPUT_MRT5
	outTarget5 = pixelShaderOut.MRT[5];
#endif

#if PIXELSHADEROUTPUT_MRT6
	outTarget6 = pixelShaderOut.MRT[6];
#endif

#if PIXELSHADEROUTPUT_MRT7
	outTarget7 = pixelShaderOut.MRT[7];
#endif

}